#include "aurora_aoa_writer.h"
#include "geometry_visitor.h"
#include "aurora_format.h"
#include "aurora_write_processor.h"

namespace aurora
{

using vertex_attribute = refl::data_buffer::vao_buffer::vertex_format::vertex_attribute;
using aurora::refl::vertex_attrs::type_t;
using aurora::refl::vertex_attrs::mode_t;


void aoa_writer::save_data(geometry_visitor const& v)
{

    auto const& vertices = v.get_verticies();
    auto const& faces = v.get_faces();

    unsigned collision_buffer_size = 0;
    unsigned light_buffer_size = 0;
    unsigned index_buffer_size = faces.size() * sizeof(std::remove_reference_t<decltype(faces)>::value_type);
    unsigned vertex_buffer_size = vertices.size() * sizeof(std::remove_reference_t<decltype(vertices)>::value_type);

    // create buffer file description

    refl::data_buffer buffer_descr;
    buffer_descr.data_buffer_file = filename_;

    buffer_descr.index_file_offset_size = { c_GP_HeaderSize, index_buffer_size };
    buffer_descr.vertex_file_offset_size = { c_GP_HeaderSize + index_buffer_size, vertex_buffer_size };

    refl::data_buffer::vao_buffer vao_descr;
    vao_descr.vertex_format_offset = { uint16_t(~0u), 0 /*faces.size() * sizeof(decltype(faces)::element_type) */ };
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

    for(auto const& chunk: chunks)
    {
        refl::node node_descr;

        node_descr.name = chunk.name;
        node_descr.draw_order = 0;

        refl::node::mesh_t::mesh_face mesh_geom;
        mesh_geom.params.id = 0;
        mesh_geom.params.offset = chunk.faces_range.lo();
        mesh_geom.params.count = chunk.faces_range.hi() - chunk.faces_range.lo();
        mesh_geom.params.base_vertex = chunk.vertex_range.lo();
        mesh_geom.params.mat = "__DEAFG";
        mesh_geom.params.shadow_mat = "Shadow_Common";
        mesh_geom.params.num_vertices = chunk.vertex_range.hi() - chunk.vertex_range.lo();

        node_descr.mesh.vao_ref.vao_id = 0;
        node_descr.mesh.vao_ref.geom_stream_id = 0;

        node_descr.mesh.bbox.rect = chunk.aabb;

        node_descr.mesh.face_array.push_back(mesh_geom);

        all_nodes.push_back(node_descr);
    }

    // write aoa file

    
    write_processor proc;
    refl::aurora_format aoa_descr;

    aoa_descr.buffer_data = buffer_descr;
    aoa_descr.nodes = all_nodes;

    reflect(proc, aoa_descr);

    std::ofstream aoa_file(fs::path(filename_).replace_extension("aoa").string());
    aoa_file << proc.result();

    // write data

    write(vertices.data(), vertex_buffer_size);
    write(faces.data(), index_buffer_size);

    // write sizes of section to the header

    file_.seekp(8);

    write(&collision_buffer_size, sizeof(unsigned));
    write(&light_buffer_size, sizeof(unsigned));
    write(&index_buffer_size, sizeof(unsigned));
    write(&vertex_buffer_size, sizeof(unsigned));
}

}