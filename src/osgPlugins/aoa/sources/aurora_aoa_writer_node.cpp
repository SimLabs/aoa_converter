#include "aurora_aoa_writer.h"
#include "aurora_format.h"

namespace aurora
{

using geometry_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::geometry_buffer_stream;
using light_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::light_buffer_stream;

struct aoa_writer::node::impl
{
    impl(aoa_writer& writer)
        : factory(writer) 
    {
    }

    aoa_writer& factory;
    refl::node node_descr;
};

aoa_writer::node::node(aoa_writer& writer)
    : pimpl_(std::make_unique<impl>(writer))
{

}

aoa_writer::node::node(node &&) = default;

aoa_writer::node::~node() = default;

aoa_writer::node_ptr aoa_writer::node::create_child(string name)
{
    node_ptr child = pimpl_->factory.create_node();
    child->set_name(name);
    pimpl_->node_descr.children.children.push_back(name);
    return child;
}

aoa_writer::node_ptr aoa_writer::node::add_flags(uint32_t flags)
{
    if(flags & node_flags::TREAT_CHILDREN)
    {
        pimpl_->node_descr.controllers.treat_children = boost::in_place();
    }
    if(flags & node_flags::DRAW_LIGHTPOINTS)
    {
        pimpl_->node_descr.controllers.draw_lightpoint = boost::in_place();
    }
    if(flags & node_flags::GLOBAL_NODE)
    {
        pimpl_->node_descr.scope = "GLOBAL";
    }
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_draw_order(unsigned order)
{
    pimpl_->node_descr.draw_order = order;
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_geometry_stream(float lod, pair<unsigned, unsigned> vertex_offset_size, pair<unsigned, unsigned> index_offset_size)
{
    if(!pimpl_->node_descr.controllers.object_param_controller)
    {
        pimpl_->node_descr.controllers.object_param_controller = boost::in_place();
    }

    geometry_buffer_stream geom_stream;
    geom_stream.index_offset_size.offset = index_offset_size.first;
    geom_stream.index_offset_size.size = index_offset_size.second;
    geom_stream.vertex_offset_size.offset = vertex_offset_size.first;
    geom_stream.vertex_offset_size.size = vertex_offset_size.second;
    geom_stream.lod = lod;

    pimpl_->node_descr.controllers.object_param_controller->buffer.geometry_streams.push_back(geom_stream);

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_omnilights_stream(unsigned offset, unsigned size)
{
    if(!pimpl_->node_descr.controllers.object_param_controller)
    {
        pimpl_->node_descr.controllers.object_param_controller = boost::in_place();
    }

    light_buffer_stream omni_lights;
    omni_lights.light_offset_size.offset = offset;
    omni_lights.light_offset_size.size = size;
    omni_lights.type = refl::light_t::OMNI;
    pimpl_->node_descr.controllers.object_param_controller->buffer.light_streams.push_back(omni_lights);

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_spotlights_stream(unsigned offset, unsigned size)
{
    if(!pimpl_->node_descr.controllers.object_param_controller)
    {
        pimpl_->node_descr.controllers.object_param_controller = boost::in_place();
    }

    light_buffer_stream spot_lights;
    spot_lights.light_offset_size.offset = offset;
    spot_lights.light_offset_size.size = size;
    spot_lights.type = refl::light_t::SPOT;
    pimpl_->node_descr.controllers.object_param_controller->buffer.light_streams.push_back(spot_lights);

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_mesh(geom::rectangle_3f bbox, unsigned offset, unsigned count, unsigned base_vertex, unsigned num_vertices, std::pair<unsigned, unsigned> vao_ref)
{
    if(!pimpl_->node_descr.mesh)
    {
        pimpl_->node_descr.mesh = boost::in_place();
    }

    auto& mesh = pimpl_->node_descr.mesh.value();
    
    pimpl_->node_descr.controllers.draw_mesh = boost::in_place();

    refl::node::mesh_t::mesh_face mesh_geom;

    mesh_geom.params.id = 0;
    mesh_geom.params.offset = offset;
    mesh_geom.params.count = count;
    mesh_geom.params.base_vertex = base_vertex;
    mesh_geom.params.mat = material_name_for_node(pimpl_->node_descr.name);
    mesh_geom.params.shadow_mat = "Shadow_Common";
    mesh_geom.params.num_vertices = num_vertices;

    mesh.vao_ref.geom_stream_id = vao_ref.first;
    mesh.vao_ref.vao_id = vao_ref.second;

    mesh.bbox.rect = bbox;

    mesh.face_array.push_back(mesh_geom);

    add_flags(aoa_writer::DRAW_MESH);

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_omni_lights(unsigned offset, unsigned size)
{
    if(!pimpl_->node_descr.lightpoint2)
        pimpl_->node_descr.lightpoint2 = boost::in_place();

    add_flags(aoa_writer::DRAW_LIGHTPOINTS);

    pimpl_->node_descr.lightpoint2->omni_offset_size = refl::offset_size{ offset, size };
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_spot_lights(unsigned offset, unsigned size)
{
    if(!pimpl_->node_descr.lightpoint2)
        pimpl_->node_descr.lightpoint2 = boost::in_place();

    pimpl_->node_descr.lightpoint2->spot_offset_size = refl::offset_size{ offset, size };
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_name(string name)
{
    pimpl_->node_descr.name = name;
    return shared_from_this();
}

void * aoa_writer::node::get_underlying()
{
    return &pimpl_->node_descr;
}


}