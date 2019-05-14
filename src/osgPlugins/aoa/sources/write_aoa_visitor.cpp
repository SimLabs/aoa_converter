#include "write_aoa_visitor.h"

#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/LightSource>
#include <osg/Geode>

#include "material_loader.h"
#include "aurora_aoa_writer.h"
#include "cpp_utils/enum_to_string.h"
#include "object_lights_config.h"

namespace aurora
{

namespace
{

geom::rectangle_3 get_rect_from_drawable(osg::Geometry& node)
{
    auto& vertices = dynamic_cast<osg::Vec3Array&>(*node.getVertexArray());
    auto num_vertices = vertices.getNumElements();

    double min_x = std::numeric_limits<double>::infinity(), max_x = -std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity(), max_y = -std::numeric_limits<double>::infinity();
    double min_z = std::numeric_limits<double>::infinity(), max_z = -std::numeric_limits<double>::infinity();

    for(unsigned i = 0; i < num_vertices; ++i)
    {
        double x, y, z;
        x = vertices[i][0];
        y = vertices[i][1];
        z = vertices[i][2];

        min_x = (std::min(min_x, x));
        min_y = (std::min(min_y, y));
        min_z = (std::min(min_z, z));

        max_x = (std::max(max_x, x));
        max_y = (std::max(max_y, y));
        max_z = (std::max(max_z, z));

        double tol = 0.01;

        if(max_x - min_x > tol &&
           max_y - min_y > tol &&
           max_z - min_z > tol)
            break;
    }
    return geom::rectangle_3(geom::point_3(min_x, min_y, min_z), geom::point_3(max_x, max_y, max_z));
}

bool find_ignore_case(const std::string & strHaystack, const std::string & strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != strHaystack.end());
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

string material_name_for_chunk(string name, material_info const& data)
{
    if(!data.explicit_material.empty())
    {
        return data.explicit_material;
    }
    else
    {
        return name + "_mtl";
    }
}
}

write_aoa_visitor::write_aoa_visitor(material_loader& l, aoa_writer& w)
    : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
    , material_loader_(l)
    , aoa_writer_(w)
    , current_geode_(nullptr)
    //, material_manager_(material_manager)
{
}

void write_aoa_visitor::apply(osg::Geode &geode)
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

    if(detect_light.should_exclude())
        return;

    if (geode2chunks_.find(&geode) != geode2chunks_.end())
        return;
    current_geode_ = &geode;
    traverse(geode);
}

void write_aoa_visitor::apply(osg::Geometry &geometry)
{
    static unsigned count = 0;
    chunk_info_opt_material chunk;
    
    extract_texture_info(geometry, chunk);

    chunk.name = geometry.getName().empty() ? "object_" + std::to_string(++count) : geometry.getName();
    chunk.vertex_range = collect_verticies(geometry);
    if (chunk.vertex_range.hi() == chunk.vertex_range.lo())
    {
        OSG_WARN << "Empty geometry" << std::endl;
    }
    fill_aabb(chunk);

    //chunk.material.material = material_manager_->extract_material(geometry);
    //if (geometry.getTexCoordArray(0) != nullptr)
    //{
    //    chunk.material.texture_index = 0;
    //    chunk.material.texture = extract_texture(geometry, 0);
    //}

    chunk.faces_range  = collect_faces(chunk.vertex_range, geometry);

    geode2chunks_[current_geode_].push_back(chunks_.size());
    chunks_.push_back(chunk);
}

void write_aoa_visitor::apply(osg::LightSource & light_source)
{
    //auto osg_light = light_source.getLight();
    //if(osg_light->getDirection().length2() == 0.)
    //{
    //    // omni light
    //    aod::omni_light light;
    //    light.position[0] = osg_light->getPosition()[0];
    //    light.position[1] = osg_light->getPosition()[1];
    //    light.position[2] = osg_light->getPosition()[2];

    //    light.power = 10000.;
    //    light.color = geom::colorb(osg_light->getDiffuse().r(), osg_light->getDiffuse().g(), osg_light->getDiffuse().b());
    //    light.r_min = 255;

    //    omni_lights.push_back(light);
    //}
    //else
    //{
    //    // spot light
    //    aod::spot_light spot;
    //    spot.position[0] = osg_light->getPosition()[0];
    //    spot.position[1] = osg_light->getPosition()[1];
    //    spot.position[2] = osg_light->getPosition()[2];
    //    spot.power = 10000;
    //    spot.color = geom::colorb(osg_light->getDiffuse().r(), osg_light->getDiffuse().g(), osg_light->getDiffuse().b());
    //    spot.r_min = 1;
    //    spot.dir_x = osg_light->getDirection()[0];
    //    spot.dir_y = osg_light->getDirection()[1];
    //    spot.dir_z = osg_light->getDirection()[2];
    //    spot.mask = 0x7;
    //    spot.half_fov = geom::grad2rad(45.);
    //    spot.angular_power = 1.;

    //    spot_lights.push_back(spot);
    //}
}

vector<size_t> const & write_aoa_visitor::get_chunks(osg::Geode const &geode) const
{
    return geode2chunks_.at(&geode);
}

chunk_info_opt_material const & write_aoa_visitor::get_chunk(size_t index) const
{
    return chunks_[index];
}

vector<chunk_info_opt_material> const & write_aoa_visitor::get_chunks() const
{
    return chunks_;
}

vector<vertex_info> const & write_aoa_visitor::get_verticies() const
{
    return verticies_;
}

vector<face> const & write_aoa_visitor::get_faces() const
{
    return faces_;
}

geom::range_2ui write_aoa_visitor::collect_verticies(osg::Geometry const &geometry)
{
    osg::Vec3Array const *vertex_array = dynamic_cast<osg::Vec3Array const*>(geometry.getVertexArray());
    osg::Vec2Array const *uv_array     = dynamic_cast<osg::Vec2Array const*>(geometry.getTexCoordArray(0));
    osg::Vec3Array const *norm_array   = dynamic_cast<osg::Vec3Array const*>(geometry.getNormalArray());

    if (!vertex_array)
        return geom::range_2ui(0, 0);

    Assert(vertex_array);
    Assert(!uv_array   || uv_array->size()   == vertex_array->size());
    Assert(!norm_array || norm_array->size() == vertex_array->size());

    size_t vertex_start = verticies_.size();
    for(size_t i = 0; i < vertex_array->size(); ++i)
    {
        vertex_info vertex;
        vertex.pos = geom::point_3f((*vertex_array)[i][0], (*vertex_array)[i][1], (*vertex_array)[i][2]);
        if(uv_array)
            vertex.uv = geom::point_2f((*uv_array)[i][0], (*uv_array)[i][1]);
        if(norm_array)
            vertex.norm = geom::point_3f((*norm_array)[i][0], (*norm_array)[i][1], (*norm_array)[i][2]);

        verticies_.push_back(vertex);
    }

    return {unsigned(vertex_start), unsigned(vertex_start + vertex_array->size())};
}

geom::range_2ui write_aoa_visitor::collect_faces(geom::range_2ui vertex_range, osg::Geometry const &geometry)
{
    geom::range_2ui faces_range = {(uint32_t)faces_.size(), (uint32_t)faces_.size()};
    for (unsigned i = 0; i < geometry.getNumPrimitiveSets(); ++i)
    {
        geom::range_2ui sub_range = collect_faces(vertex_range, *geometry.getPrimitiveSet(i));
        faces_range.hi_ = sub_range.hi();
    }
    return faces_range;
}

geom::range_2ui write_aoa_visitor::collect_faces(geom::range_2ui vertex_range, osg::PrimitiveSet const &primitive_set)
{
    geom::range_2ui faces_range = {(uint32_t)faces_.size(), (uint32_t)faces_.size()};
    if (primitive_set.getMode() != osg::PrimitiveSet::TRIANGLES)
        return faces_range;

    unsigned elements_count = primitive_set.getNumIndices();
    Assert(elements_count % 3 == 0);

    for (unsigned i = 0; i < elements_count; i += 3)
    {
        face face;
        for (unsigned j = 0; j < 3; ++j)
        {
            face.v[j] = primitive_set.index(i + j);
        }
        faces_.push_back(face);
    }

    faces_range.hi_ += elements_count / 3;
    return faces_range;
}

void write_aoa_visitor::extract_texture_info(osg::Drawable& node, chunk_info_opt_material& chunk)
{
    osg::Drawable* drawable = &node;
    if(drawable && drawable->getStateSet())
    {
        osg::StateSet* state_set = drawable->getStateSet();

        osg::Material* mat = (osg::Material*) state_set->getAttribute(osg::StateAttribute::MATERIAL);

        if(!mat)
        {
            OSG_WARN << "AOA plugin: no material in node \"" << node.getName() << "\" skipping \n";
            return;
        }

        auto mat_data = material_loader_.get_material_data(mat->getName());

        if(!mat_data)
        {
            OSG_WARN << "AOA plugin: material \"" << mat->getName() << "\" not found, skipping\n";
            return;
        }

        if(mat_data->explicit_material.empty())
        {
            for(unsigned int i = 0; i < state_set->getTextureAttributeList().size(); ++i)
            {
                osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(state_set->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
                if(texture && texture->getImage())
                {
                    chunk.material.textures.push_back(texture->getImage()->getFileName());
                }
            }            
        }
        else
        {
            chunk.material.explicit_material = mat_data->explicit_material;
        }
    }

    //traverse(node);
}

void write_aoa_visitor::fill_aabb(chunk_info_opt_material &chunk) const
{
    float min_x, min_y, min_z;
    min_x = min_y = min_z = std::numeric_limits<float>::max();

    float max_x, max_y, max_z;
    max_x = max_y = max_z = std::numeric_limits<float>::min();

    auto update_min_max = [](float &min, float &max, float value)
    {
        min = std::min(min, value);
        max = std::max(max, value);
    };

    for (size_t i = chunk.vertex_range.lo(); i < chunk.vertex_range.hi(); ++i)
    {
        update_min_max(min_x, max_x, verticies_[i].pos.x);
        update_min_max(min_y, max_y, verticies_[i].pos.y);
        update_min_max(min_z, max_z, verticies_[i].pos.z);
    }

    chunk.aabb = geom::rectangle_3f(geom::point_3f(min_x, min_y, min_z), geom::point_3f(max_x, max_y, max_z));
}

void write_aoa_visitor::write_debug_obj_file(string file_name) const
{
    std::ofstream obj_file(file_name);

    for(auto const& v : get_verticies())
    {
        obj_file << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z << std::endl;
    }

    for(auto const& v : get_verticies())
    {
        obj_file << "vn " << v.norm.x << " " << v.norm.y << " " << v.norm.z << std::endl;
    }

    for(auto const& face : get_faces())
    {
        unsigned v[3] = { face.v[0] + 1, face.v[1] + 1, face.v[2] + 1 };
        obj_file << "f ";
        for(unsigned i = 0; i < 3; ++i)
        {
            string foo = std::to_string(v[i]);
            foo = foo + "//" + foo + " ";
            obj_file << foo;
        }
        obj_file << std::endl;
    }
}

void write_aoa_visitor::write_aoa()
{
    OSG_INFO << "AOA plugin: EXTRACTED " << get_chunks().size()    << " CHUNKS"   << std::endl;
    OSG_INFO << "AOA plugin: EXTRACTED " << get_faces().size()     << " FACES"    << std::endl;
    OSG_INFO << "AOA plugin: EXTRACTED " << get_verticies().size() << " VIRTICES" << std::endl;

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

    for(auto& p: lights_)
    {
        auto light_type = p.first;
        auto lights_of_specific_type = lights_node->create_child(cpp_utils::enum_to_string(light_type) + "_lights");
        for(auto& geode: p.second)
        {
            unsigned num_drawables = geode->getNumChildren();

            for(unsigned i = 0; i < num_drawables; ++i)
            {   
                auto box = get_rect_from_drawable(dynamic_cast<osg::Geometry&>(*geode->getDrawable(i)));
                auto box_center = box.center();
            
                auto it = config.lights.light_kind_node_ref.find(cpp_utils::enum_to_string(light_type));
                if(it == config.lights.light_kind_node_ref.end())
                {
                    OSG_WARN << "ref node for " << light_type << " not found";
                    continue;
                }
                auto const& node_ref = it->second;

                // add ref to node
                lights_geom->create_child("lights_geom_" + std::to_string(ref_node_id++))
                    ->add_control_pos_key_spec(0, box_center)
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
                auto adjust_pos = [&box_center](auto& l)
                {
                    l.position[0] += box_center.x;
                    l.position[1] += box_center.y;
                    l.position[2] += box_center.z;
                };
                std::for_each(begin(omni_lights) + omni_size, end(omni_lights), adjust_pos);
                std::for_each(begin(spot_lights) + spot_size, end(spot_lights), adjust_pos);
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

    vector<vertex_attribute> vertex_format;
    vertex_format.push_back(vertex_attribute
    {   /*.id      = */ 0,
                                          /*.size    = */ 3,
                                          /*.type    = */ type_t::FLOAT,
                                          /*.mode    = */ mode_t::ATTR_MODE_FLOAT,
                                          /*.divisor = */ 0
    });

    vertex_format.push_back(vertex_attribute
    {   /*.id      = */ 1,
                                          /*.size    = */ 3,
                                          /*.type    = */ type_t::FLOAT,
                                          /*.mode    = */ mode_t::ATTR_MODE_FLOAT,
                                          /*.divisor = */ 0
    });

    vertex_format.push_back(vertex_attribute
    {   /*.id      = */ 4,
                                          /*.size    = */ 2,
                                          /*.type    = */ type_t::FLOAT,
                                          /*.mode    = */ mode_t::ATTR_MODE_FLOAT,
                                          /*.divisor = */ 0
    });

    vector<vertex_attribute> col_attrs_format{vertex_format.front()};

    geom::rectangle_3f bbox;

    for(auto const& chunk: get_chunks())
    {
        bbox |= chunk.aabb;
    }

    for(auto const& chunk : get_chunks())
    {
        vector<vertex_info> vertices(get_verticies().begin() + chunk.vertex_range.lo(), get_verticies().begin() + chunk.vertex_range.hi());
        vector<face>        faces(get_faces().begin() + chunk.faces_range.lo(), get_faces().begin() + chunk.faces_range.hi());
        vector<geom::point_3f> col_attrs;
        vector<face>        col_faces = faces;

        std::transform(begin(vertices), end(vertices), back_inserter(col_attrs), 
            [](auto const& v){ return v.pos; });

        aoa_writer_.add_material(material_name_for_chunk(chunk.name, chunk.material), chunk.material);

        root->set_cvbox_spec(bbox);
        root->create_child(chunk.name)
            ->add_mesh(chunk.aabb, vertices, vertex_format, faces, 250.f, material_name_for_chunk(chunk.name, chunk.material));
        root->create_child(chunk.name + "_col")
            ->add_collision_mesh(col_attrs, col_faces, col_attrs_format);
    }

    aoa_writer_.save_data();
}

}