#include "aurora_aoa_writer.h"
#include "geometry_visitor.h"
#include "aurora_format.h"
#include "aurora_write_processor.h"

namespace aurora
{

using vertex_attribute = refl::data_buffer::vao_buffer::vertex_format::vertex_attribute;
using geometry_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::geometry_buffer_stream;
using light_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::light_buffer_stream;

using aurora::refl::vertex_attrs::type_t;
using aurora::refl::vertex_attrs::mode_t;


auto index_offset = [](unsigned len) { return len * sizeof(face); };
auto vertex_offset = [](unsigned len) { return len * sizeof(vertex_info); };
void write_material(chunk_info_opt_material const& info, refl::material_list& mats);

string material_name_for_node(string node_name)
{
    return node_name + "_mtl";
}

refl::node create_root_node(geometry_visitor const& v, aod::lights_buffer const& lights_buffer, aod::collision_buffer const& col_buffer, string root_name)
{
    refl::node root_node;

    root_node.name = root_name;
    root_node.draw_order = 0;
    root_node.controllers.treat_children = "";
    root_node.controllers.draw_lightpoint = "";
    root_node.scope = "GLOBAL";

    refl::node::controllers_t::control_object_param_data control_object_params;
    auto&  geometry_streams = control_object_params.buffer;

    for(auto const& chunk : v.get_chunks())
    {
        root_node.children.children.push_back(chunk.name);
    }

    geometry_buffer_stream geom_stream;
    geom_stream.index_offset_size.offset = 0;
    geom_stream.index_offset_size.size = index_offset(v.get_faces().size());
    geom_stream.vertex_offset_size.offset = 0;
    geom_stream.vertex_offset_size.size = vertex_offset(v.get_verticies().size());
    geom_stream.lod = 250.;
    geometry_streams.geometry_streams.push_back(geom_stream);

    light_buffer_stream omni_lights;
    omni_lights.light_offset_size.offset = aoa_writer::AOD_HEADER_SIZE + col_buffer.size();
    omni_lights.light_offset_size.size = lights_buffer.omni_size();
    omni_lights.type = refl::light_t::OMNI;
    geometry_streams.light_streams.push_back(omni_lights);

    light_buffer_stream spot_lights;
    spot_lights.light_offset_size.offset = aoa_writer::AOD_HEADER_SIZE + col_buffer.size() + lights_buffer.omni_size();
    spot_lights.light_offset_size.size = lights_buffer.spot_size();
    spot_lights.type = refl::light_t::SPOT;
    geometry_streams.light_streams.push_back(spot_lights);

    root_node.controllers.object_param_controller = control_object_params;

    root_node.lightpoint2 = boost::in_place();
    if(lights_buffer.omni_lights.size())
        root_node.lightpoint2->omni_offset_size = refl::offset_size{0, lights_buffer.omni_lights.size()};
    if(lights_buffer.spot_lights.size())
        root_node.lightpoint2->spot_offset_size = refl::offset_size{0, lights_buffer.spot_lights.size()};

    return root_node;
}

void aoa_writer::save_data(geometry_visitor const& v)
{
    refl::aurora_format aoa_descr;

    auto const& vertices = v.get_verticies();
    auto const& faces = v.get_faces();

    aod::collision_buffer collision_buffer;
    aod::lights_buffer lights_buffer;

    aod::omni_light light;
    light.position[0] = 0;
    light.position[1] = -10;
    light.position[2] = 13.;

    light.power = 1000.;
    light.color = geom::colorb(255, 0, 0);
    light.r_min = 255;

    lights_buffer.omni_lights.push_back(light);

    aod::spot_light spot;
    spot.position[0] = 0;
    spot.position[1] = -30;
    spot.position[2] = 13.;
    spot.power = 1000000;
    spot.color = geom::color_blue();
    spot.r_min = 1;
    spot.dir_x = 0.;
    spot.dir_y = 1.;
    spot.dir_z = 0.;
    spot.mask = 0x7;
    spot.half_fov = geom::grad2rad(45.);
    spot.angular_power = 1.;

    lights_buffer.spot_lights.push_back(spot);

    //lights_buffer.spot_lights.resize(200);

    unsigned collision_buffer_size = collision_buffer.size();
    unsigned light_buffer_size = lights_buffer.size();
    unsigned index_buffer_size = faces.size() * sizeof(std::remove_reference_t<decltype(faces)>::value_type);
    unsigned vertex_buffer_size = vertices.size() * sizeof(std::remove_reference_t<decltype(vertices)>::value_type);

    // write materials

    for(auto const& chunk : v.get_chunks())
    {
        write_material(chunk, aoa_descr.materials);
    }

    // create buffer file description

    refl::data_buffer buffer_descr;
    buffer_descr.data_buffer_file = fs::path(filename_).filename().string();

    buffer_descr.index_file_offset_size = { AOD_HEADER_SIZE + collision_buffer_size + light_buffer_size, index_buffer_size };
    buffer_descr.vertex_file_offset_size = { AOD_HEADER_SIZE + collision_buffer_size + light_buffer_size + index_buffer_size, vertex_buffer_size };

    refl::data_buffer::vao_buffer vao_descr;
    vao_descr.vertex_format_offset.offset = index_buffer_size;
    vao_descr.format.attributes.push_back(vertex_attribute
    {   /*.id      = */ 0, 
        /*.size    = */ 3, 
        /*.type    = */ type_t::FLOAT, 
        /*.mode    = */ mode_t::ATTR_MODE_FLOAT, 
        /*.divisor = */ 0
    });

    vao_descr.format.attributes.push_back(vertex_attribute
    {   /*.id      = */ 1,
        /*.size    = */ 3,
        /*.type    = */ type_t::FLOAT,
        /*.mode    = */ mode_t::ATTR_MODE_FLOAT,
        /*.divisor = */ 0
    });

    vao_descr.format.attributes.push_back(vertex_attribute
    {   /*.id      = */ 4,
        /*.size    = */ 2,
        /*.type    = */ type_t::FLOAT,
        /*.mode    = */ mode_t::ATTR_MODE_FLOAT,
        /*.divisor = */ 0
    });

    buffer_descr.vaos.push_back(vao_descr);

    // create node description

    auto const& chunks = v.get_chunks();
    vector<refl::node> all_nodes;

    all_nodes.push_back(create_root_node(v, lights_buffer, collision_buffer, fs::path(filename_).stem().string()));

    for(auto const& chunk: chunks)
    {
        refl::node node_descr;
        node_descr.mesh = refl::node::mesh_t{};
        auto& mesh = node_descr.mesh.value();

        node_descr.name = chunk.name;
        node_descr.draw_order = 0;

        refl::node::mesh_t::mesh_face mesh_geom;
        refl::node::controllers_t::control_object_param_data control_object_params;
        refl::node::controllers_t::control_object_param_data::data_buffer geometry_streams;

        //geometry_buffer_stream geom_stream;
        //geom_stream.index_offset_size.offset = index_offset(chunk.faces_range.lo());
        //geom_stream.index_offset_size.size = index_offset(chunk.faces_range.hi() - chunk.faces_range.lo());
        //geom_stream.vertex_offset_size.offset = vertex_offset(chunk.vertex_range.lo());
        //geom_stream.vertex_offset_size.size = vertex_offset(chunk.vertex_range.hi() - chunk.vertex_range.lo());
        //geom_stream.lod = 250.;
        //geometry_streams.geometry_streams.push_back(geom_stream);

        //control_object_params.buffer = geometry_streams;

        //node_descr.controllers.object_param_controller = control_object_params;
        node_descr.controllers.draw_mesh = "";

        mesh_geom.params.id = 0;
        mesh_geom.params.offset = chunk.faces_range.lo() * 3;
        mesh_geom.params.count = chunk.faces_range.hi() - chunk.faces_range.lo();
        mesh_geom.params.base_vertex = chunk.vertex_range.lo();
        mesh_geom.params.mat = material_name_for_node(node_descr.name);
        mesh_geom.params.shadow_mat = "Shadow_Common";
        mesh_geom.params.num_vertices = chunk.vertex_range.hi() - chunk.vertex_range.lo();

        mesh.vao_ref.vao_id = 0;
        mesh.vao_ref.geom_stream_id = 0;

        mesh.bbox.rect = chunk.aabb;

        mesh.face_array.push_back(mesh_geom);

        all_nodes.push_back(node_descr);
    }

    // write aoa file

    
    write_processor proc;

    aoa_descr.buffer_data = buffer_descr;
    aoa_descr.nodes = all_nodes;

    reflect(proc, aoa_descr);

    std::ofstream aoa_file(fs::path(filename_).replace_extension("aoa").string());
    aoa_file << proc.result();

    // write data

    lights_buffer.write(*this);
    write(faces.data(), index_buffer_size);
    write(vertices.data(), vertex_buffer_size);

    // write sizes of section to the header

    file_.seekp(8);

    write(&collision_buffer_size, sizeof(unsigned));
    write(&light_buffer_size, sizeof(unsigned));
    write(&index_buffer_size, sizeof(unsigned));
    write(&vertex_buffer_size, sizeof(unsigned));
}



void write_material(chunk_info_opt_material const & info, refl::material_list& mats)
{
    refl::material_list::material mat;
    mat.name = material_name_for_node(info.name);

    switch(info.material.textures.size())
    {
        case 0:
        case 1:
            mat.link = "__D";
            break;
        case 2:
            mat.link = "__DN";
            break;
        case 3:
        default:
            mat.link = "__DEAFG";
            break;
    }

    for(unsigned i = 0; i < info.material.textures.size(); i++)
    {
        mat.textures.push_back(refl::material_list::material::material_group{i, info.material.textures[i]});
    }

    mats.list.push_back(mat);
}



}