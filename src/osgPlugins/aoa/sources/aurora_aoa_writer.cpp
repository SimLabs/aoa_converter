#include "aurora_aoa_writer.h"
#include "geometry_visitor.h"
#include "aurora_format.h"
#include "aurora_write_processor.h"

namespace aurora
{

using vertex_attribute = refl::data_buffer::vao_buffer::vertex_format::vertex_attribute;
using geometry_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::geometry_buffer_stream;
using aurora::refl::vertex_attrs::type_t;
using aurora::refl::vertex_attrs::mode_t;


auto index_offset = [](unsigned len) { return len * sizeof(face); };
auto vertex_offset = [](unsigned len) { return len * sizeof(vertex_info); };

refl::node create_root_node(geometry_visitor const& v)
{
    refl::node root_node;

    root_node.name = "Old House";
    root_node.draw_order = 0;
    root_node.controllers.treat_children = "";
    root_node.scope = "GLOBAL";

    refl::node::controllers_t::control_object_param_data control_object_params;
    auto&  geometry_streams = control_object_params.buffer;

    for(auto const& chunk : v.get_chunks())
    {
        root_node.children.children.push_back(chunk.name);

        geometry_buffer_stream geom_stream;
        geom_stream.index_offset_size.offset = index_offset(chunk.faces_range.lo());
        geom_stream.index_offset_size.size = index_offset(chunk.faces_range.hi() - chunk.faces_range.lo());
        geom_stream.vertex_offset_size.offset = vertex_offset(chunk.vertex_range.lo());
        geom_stream.vertex_offset_size.size = vertex_offset(chunk.vertex_range.hi() - chunk.vertex_range.lo());
        geom_stream.lod = 250.;

        geometry_streams.geometry_streams.push_back(geom_stream);
    }

    root_node.controllers.object_param_controller = control_object_params;

    return root_node;
}

void aoa_writer::save_data(geometry_visitor const& v)
{
    refl::aurora_format aoa_descr;

    auto const& vertices = v.get_verticies();
    auto const& faces = v.get_faces();

    unsigned collision_buffer_size = 0;
    unsigned light_buffer_size = 0;
    unsigned index_buffer_size = faces.size() * sizeof(std::remove_reference_t<decltype(faces)>::value_type);
    unsigned vertex_buffer_size = vertices.size() * sizeof(std::remove_reference_t<decltype(vertices)>::value_type);

    // stub for materials

    aoa_descr.materials.list.push_back({"test_mtl", "__DEAFG"});

    // create buffer file description

    refl::data_buffer buffer_descr;
    buffer_descr.data_buffer_file = fs::path(filename_).filename().string();

    buffer_descr.index_file_offset_size = { c_GP_HeaderSize, index_buffer_size };
    buffer_descr.vertex_file_offset_size = { c_GP_HeaderSize + index_buffer_size, vertex_buffer_size };

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

    all_nodes.push_back(create_root_node(v));

    unsigned count = 0;
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
        mesh_geom.params.offset = index_offset(chunk.faces_range.lo());
        mesh_geom.params.count = chunk.faces_range.hi() - chunk.faces_range.lo();
        mesh_geom.params.base_vertex = 0;
        mesh_geom.params.mat = "test_mtl";
        mesh_geom.params.shadow_mat = "Shadow_Common";
        mesh_geom.params.num_vertices = chunk.vertex_range.hi() - chunk.vertex_range.lo();

        mesh.vao_ref.vao_id = 0;
        mesh.vao_ref.geom_stream_id = count++;

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

    write(faces.data(), index_buffer_size);
    write(vertices.data(), vertex_buffer_size);

    // write sizes of section to the header

    file_.seekp(8);

    write(&collision_buffer_size, sizeof(unsigned));
    write(&light_buffer_size, sizeof(unsigned));
    write(&index_buffer_size, sizeof(unsigned));
    write(&vertex_buffer_size, sizeof(unsigned));
}

}