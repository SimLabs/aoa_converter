#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>

#include "lights_generation_visitor.h"
#include "aurora_aoa_writer.h"
#include "object_lights_config.h"

using namespace aurora;

namespace
{

bool find_ignore_case(const std::string & strHaystack, const std::string & strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != strHaystack.end());
}

geom::point_3f get_translation(osg::Matrix const& m)
{
    osg::Vec3 osg_translate = m.getTrans();
    return { osg_translate.x(), osg_translate.y(), osg_translate.z() };
}


geom::quaternionf get_rotation(osg::Matrix const& m)
{
    osg::Quat osg_rotate = m.getRotate();
    return { float(osg_rotate.w()), geom::point_3f(osg_rotate.x(), osg_rotate.y(), osg_rotate.z()) };
}

struct detect_light_node_visitor : osg::NodeVisitor
{
    detect_light_node_visitor()
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS)
    {}

    void apply(osg::Node& node) override
    {
        auto const& config = get_config();
        for(auto p : config.lights.light_kind_node_names_map)
        {
            for(auto name : p.second)
            {
                if(find_ignore_case(node.getName(), name))
                {
                    light_type_ = *cpp_utils::string_to_enum<light_type>(p.first);
                }
            }
        }
        if(light_type_ || node.getName().find("light") != std::string::npos)
        {
            should_remove_ = true;
        }
        if(!light_type_)
        {
            traverse(node);
        }
    }

    optional<light_type> get_result()
    {
        return light_type_;
    }

    bool should_exclude()
    {
        return should_remove_;
    }

private:
    bool should_remove_ = false;
    optional<light_type> light_type_;
};

}

void lights_generation_visitor::apply(osg::Geode & geode)
{
    detect_light_node_visitor detect_light;
    geode.accept(detect_light);
    if(detect_light.get_result())
    {
        if(geode.getNumDrawables() == 0)
            return;
        else
        {
            lights_[*detect_light.get_result()].insert(&geode);
        }
    }
}

void aurora::lights_generation_visitor::generate_lights()
{
    aoa_writer::node_ptr root = aoa_writer_.get_root_node();
    auto& config = get_config();
    auto& lights_config = get_object_lights_config();

    vector<aod::omni_light> omni_lights;
    vector<aod::spot_light> spot_lights;
    unsigned current_offset_omni = 0;
    unsigned current_offset_spot = 0;

    auto lights_node = root->create_child("lights");
    auto lights_geom = lights_node->create_child("lights_geom");
    unsigned ref_node_id = 0;

    for(auto& p : lights_)
    {
        auto light_type = p.first;
        auto lights_of_specific_type = lights_node->create_child(cpp_utils::enum_to_string(light_type) + "_lights");
        for(auto& geode : p.second)
        {
            unsigned num_drawables = geode->getNumChildren();

            for(unsigned i = 0; i < num_drawables; ++i)
            {
                osg::Matrix transform;
                osg::Node* node = geode;

                while(node->getNumParents())
                {
                    auto mat_transform = dynamic_cast<osg::MatrixTransform*>(node->getParent(0));
                    if(mat_transform)
                    {
                        transform.postMult(mat_transform->getMatrix());
                    }
                    node = node->getParent(0);
                }

                osg::Matrix ref_node_transform = get_config().flip_YZ ? get_config().reverse_flip_YZ_matrix : osg::Matrix::identity();
                ref_node_transform.postMult(transform);

                auto it = config.lights.light_kind_node_ref.find(cpp_utils::enum_to_string(light_type));
                if(it == config.lights.light_kind_node_ref.end())
                {
                    OSG_WARN << "ref node for " << light_type << " not found";
                    continue;
                }
                auto const& node_ref = it->second;

                // add ref to node
                lights_geom->create_child("lights_geom_" + std::to_string(ref_node_id++))
                    ->set_translation(get_translation(ref_node_transform))
                    ->set_rotation(get_rotation(ref_node_transform))
                    ->set_control_ref_node_spec(node_ref);

                auto lights_it = lights_config.find(node_ref);
                if(lights_it == lights_config.end())
                    continue;

                auto& ref_node_omni_light = lights_it->second.omni_lights;
                auto& ref_node_spot_lights = lights_it->second.spot_lights;
                auto omni_size = omni_lights.size();
                auto spot_size = spot_lights.size();
                std::copy(begin(ref_node_omni_light), end(ref_node_omni_light), back_inserter(omni_lights));
                std::copy(begin(ref_node_spot_lights), end(ref_node_spot_lights), back_inserter(spot_lights));
                auto adjust_omni = [translation = get_translation(ref_node_transform)](auto& l)
                {
                    l.position[0] += translation.x;
                    l.position[1] += translation.y;
                    l.position[2] += translation.z;
                };
                auto adjust_spot = [rotation = get_rotation(ref_node_transform), &adjust_omni](auto& l)
                {
                   adjust_omni(l);
                   geom::point_3f dir = {l.dir_x, l.dir_y, l.dir_z};
                   dir = rotation.rotate_vector(dir);
                   l.dir_x = dir.x;
                   l.dir_y = dir.y; 
                   l.dir_z = dir.z;
                };
                std::for_each(begin(omni_lights) + omni_size, end(omni_lights), adjust_omni);
                std::for_each(begin(spot_lights) + spot_size, end(spot_lights), adjust_spot);
            }
        }

        if(omni_lights.size() != current_offset_omni)
            lights_of_specific_type->set_omni_lights(current_offset_omni, omni_lights.size() - current_offset_omni);
        if(spot_lights.size() != current_offset_spot)
            lights_of_specific_type->set_spot_lights(current_offset_spot, spot_lights.size() - current_offset_spot);

        current_offset_omni = omni_lights.size();
        current_offset_spot = spot_lights.size();
    }

    aoa_writer_.set_omni_lights_buffer_data(omni_lights);
    aoa_writer_.set_spot_lights_buffer_data(spot_lights);
}
