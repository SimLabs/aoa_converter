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
    // conjugation is needed due to convention difference with osg
    return !geom::quaternionf{ float(osg_rotate.w()), geom::point_3f(osg_rotate.x(), osg_rotate.y(), osg_rotate.z()) };
}

osg::Matrix compute_ref_node_transform(osg::Node* node)
{
    osg::Matrix transform;

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
    return ref_node_transform;
}

struct detect_light_node_visitor : osg::NodeVisitor
{
    detect_light_node_visitor()
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS)
    {}

    bool is_root_node(const osg::Node* n)
    {
        return n->getName().find(".fbx") == n->getName().length() - 4;
    }

    bool node_matches(vector<vector<string>> const& rules)
    {
        vector<osg::Node*> nodes_path = getNodePath();
        auto last_path_node = nodes_path.end()-1;

        // remove transform before geode with the same name
        if(dynamic_cast<osg::Geode*>(*last_path_node) && last_path_node != nodes_path.begin())
        {
            auto prev_last = (last_path_node - 1);
            if(dynamic_cast<osg::MatrixTransform*>(*prev_last) && (*prev_last)->getName() == (*last_path_node)->getName())
                nodes_path.erase(prev_last);
        }

        // remove transforms if there is a neighbour node (either parent or child) with the same name
        //auto it = nodes_path.begin();
        //while(it != nodes_path.end())
        //{
        //    auto next_it = it != nodes_path.end() ? it+1 : nodes_path.end();
        //    auto prev_it = it != nodes_path.begin() ? it-1 : nodes_path.end();
        //    bool need_erase = false;
        //    auto t = dynamic_cast<osg::MatrixTransform*>(*it);
        //    if(!t)
        //        need_erase = false;
        //    else if(prev_it != nodes_path.end() && (*prev_it)->getName() == t->getName())
        //    {
        //        Assert(!dynamic_cast<osg::MatrixTransform*>(*prev_it));
        //        need_erase = true;
        //    }
        //    else if(next_it != nodes_path.end() && (*next_it)->getName() == t->getName())
        //    {
        //        Assert(!dynamic_cast<osg::MatrixTransform*>(*next_it));
        //        need_erase = true;
        //    }

        //    if(need_erase)
        //        it = nodes_path.erase(it);
        //    else
        //        ++it;
        //}

        auto nodes_path_end = nodes_path.cend();
        auto root_it = std::find_if(nodes_path.cbegin(), nodes_path_end, [this](auto const n){  return is_root_node(n); });
        if(root_it == nodes_path_end)
        {
            OSG_FATAL << "AOA plugin: root node was not found" << std::endl;
            return false;
        }

        auto cur_rule = rules.cbegin();
        auto cur_node_in_path = ++root_it;

        if(std::distance(cur_node_in_path, nodes_path_end) != rules.size())
            return false;

        for(;cur_node_in_path != nodes_path_end; 
            ++cur_node_in_path, ++cur_rule)
        {
            bool matches = false;
            for(auto rule: *cur_rule)
            {
                if(rule == "*" || (*cur_node_in_path)->getName() == rule)
                {
                    matches = true;
                    break;
                }
            }
            if(!matches)
                return false;
        }

        return true;
    }

    void apply(osg::Node& node) override
    {
        auto const& config = get_config();
        for(auto p : config.lights)
        {
            for(auto p2: p.second)
            {
                if(node_matches(p2.second.find_rules))
                {
                    light_type_ = pair<string, string>{ p.first, p2.first };
                }
            }
        }
        if(!light_type_)
        {
            traverse(node);
        }
    }

    optional<pair<string, string>> const& get_result()
    {
        return light_type_;
    }

private:
    optional<pair<string, string>> light_type_;
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
            auto const& path = detect_light.get_result().value();
            for(unsigned i = 0; i < geode.getNumDrawables(); ++i)
            {
                lights_[path.first][path.second].insert(geode.getDrawable(i));
            }
        }
    }
}

void aurora::lights_generation_visitor::generate_lights()
{
    aoa_writer::node_ptr root = aoa_writer_.get_root_node();
    auto& config = get_config();
    auto& lights_config = get_object_lights_config();

    auto lights_node = root->create_child("lights");
    unsigned ref_node_id = 0;

    auto add_node_args = [&config](auto node)
    {
        if(config.channel_file.empty())
        {
            OSG_WARN << "AOA plugin: channel file was not specified" << std::endl;
        }
        else
            node->set_channel_file(config.channel_file);

        for(auto const& p : config.lights)
        {
            for(auto const&p2 : p.second)
            {
                node->add_float_arg_spec(p2.first, 1.);
            }
        }

        node->add_flags(aoa_writer::node_flags::GLOBAL_NODE);
    };

    for(auto const& p2 : lights_)
    {
        for(auto const& p: p2.second)
        {
            vector<aod::omni_light> omni_lights;
            vector<aod::spot_light> spot_lights;

            auto sub_channel = p2.first;
            auto light_type = p.first;
            auto node_config = config.lights.at(sub_channel).at(light_type);
            auto const& ref_node = node_config.ref_node;
            assert(!ref_node.empty());
            auto lights_it = lights_config.find(ref_node);

            string lights_node_name = sub_channel + "_" + light_type + "_lights";

            auto lights_of_specific_type = lights_node->create_child(lights_node_name);
            auto lights_placement_node = lights_of_specific_type;

            // custom case for inserting node that has its own lights
            // this is used for PAPI lights
            // we just insert ref node section and define args there
            if(p.second.size() == 1 && lights_it == lights_config.end())
            {
                osg::Matrix ref_node_transform = compute_ref_node_transform(*p.second.begin());

                // add ref to node
                lights_placement_node
                    ->set_translation(get_translation(ref_node_transform))
                    ->set_rotation(get_rotation(ref_node_transform))
                    ->set_control_ref_node_spec(ref_node, sub_channel)
                    ->add_ref_node_arg_spec(light_type, "FLOAT", 1.);

                for(auto const& a: node_config.add_arguments)
                {
                    lights_placement_node->add_ref_node_arg_spec(a.channel, a.type, a.value);
                }
            }
            else
            {
                auto placement_node_name = lights_node_name + "_content";
                lights_placement_node = aoa_writer_.create_top_level_node()->set_name(placement_node_name);
                lights_of_specific_type->set_control_ref_node_spec(placement_node_name, sub_channel);
                auto lights_geom = lights_placement_node->create_child(lights_node_name + "_content_geom");
                string const& ref_node_name = lights_it != lights_config.end() ? lights_it->second.ref_node : ref_node;

                for(auto drawable : p.second)
                {
                    osg::Matrix ref_node_transform = compute_ref_node_transform(drawable);

                    // add ref to node
                    lights_geom->create_child("lights_geom_" + std::to_string(ref_node_id++))
                        ->set_translation(get_translation(ref_node_transform))
                        ->set_rotation(get_rotation(ref_node_transform))
                        ->set_control_ref_node_spec(ref_node_name);

                    if(lights_it != lights_config.end())
                    {
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
                        auto adjust_spot = [rotation = ref_node_transform.getRotate(), &adjust_omni](auto& l)
                        {
                            adjust_omni(l);
                            osg::Vec3 dir = { l.dir_x, l.dir_y, l.dir_z };
                            dir = rotation * dir;
                            l.dir_x = dir.x();
                            l.dir_y = dir.y();
                            l.dir_z = dir.z();
                        };
                        std::for_each(begin(omni_lights) + omni_size, end(omni_lights), adjust_omni);
                        std::for_each(begin(spot_lights) + spot_size, end(spot_lights), adjust_spot);
                    }
                }
            }

            add_node_args(lights_placement_node);
            for(auto& arg: node_config.add_arguments)
            {
                lights_placement_node->add_float_arg_spec(arg.channel, arg.value);
            }

            if(omni_lights.size() || spot_lights.size())
            {
                auto draw_node = lights_placement_node->create_child(lights_node_name + "_draw");
                draw_node->set_control_light_power_spec(light_type);

                if(omni_lights.size() != 0)
                {
                    lights_placement_node->set_omni_lights_buffer_data(omni_lights);
                    draw_node->set_omni_lights(0, omni_lights.size());
                    draw_node->set_lights_class(node_config.clazz);
                }
                if(spot_lights.size() != 0)
                {
                    lights_placement_node->set_spot_lights_buffer_data(spot_lights);
                    draw_node->set_spot_lights(0, spot_lights.size());
                    draw_node->set_lights_class(node_config.clazz);
                }
            }
        }
    }

    add_node_args(root);

}
