#include "aurora_aoa_writer.h"
#include "write_aoa_visitor.h"
#include "aurora_format.h"
#include "aurora_write_processor.h"
#include "osgDB/FileUtils"

namespace aurora
{

using geometry_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::geometry_buffer_stream;
using light_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::light_buffer_stream;
using collision_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::collision_buffer_stream;

namespace
{
struct lights_buffer
{
    vector<aod::omni_light> omni_lights;
    vector<aod::spot_light> spot_lights;

    size_t size() const
    {
        return omni_size() + spot_size();
    }

    template<class T>
    void write_omni(T& out) const
    {
        out.write(reinterpret_cast<const char*>(omni_lights.data()), omni_size());
    }

    template<class T>
    void write_spot(T& out) const
    {
        out.write(reinterpret_cast<const char*>(spot_lights.data()), spot_size());
    }

    size_t omni_size() const
    {
        return sizeof(std::remove_reference_t<decltype(this->omni_lights)>::value_type) * omni_lights.size();
    }

    size_t spot_size() const
    {
        return sizeof(std::remove_reference_t<decltype(this->spot_lights)>::value_type) * spot_lights.size();
    }
};

struct in_place
{
    template<typename T>
    operator std::optional<T>() const
    {
        return std::optional<T>(std::in_place);
    }

};

}

struct buffer_chunk
{
    enum kind_t
    {
        MESH, COLLISION_MESH
    };
    
    kind_t kind           = MESH;
    float             lod = 0.;
    unsigned num_vertices = 0;
    vector<aoa_writer::vertex_attribute> vertex_format;
    vector<char> data;
    vector<face> faces;
    string material;
    string shadow_material;
    geom::rectangle_3f bbox;
    weak_ptr<aoa_writer::node> node;
};

struct aoa_writer::node::impl
{
    impl(aoa_writer& writer)
        : factory(writer)
    {
    }

    vector<buffer_chunk> buffer_chunks;
    aoa_writer& factory;
    refl::node node_descr;
    lights_buffer lights_buf;
};

aoa_writer::node::node(aoa_writer& writer)
    : pimpl_(std::make_unique<impl>(writer))
{

}

aoa_writer::node::node(node &&) = default;

aoa_writer::node::~node() = default;

aoa_writer::node_ptr aoa_writer::node::create_child(string name)
{
    node_ptr child = pimpl_->factory.create_node()
        ->set_name(name);
        
    this->add_flags(aoa_writer::TREAT_CHILDREN);

    pimpl_->node_descr.children.children.push_back(name);
    return child;
}

aoa_writer::node_ptr aoa_writer::node::add_flags(uint32_t flags)
{
    if(flags & node_flags::TREAT_CHILDREN)
    {
        pimpl_->node_descr.controllers.treat_children = in_place();
    }
    if(flags & node_flags::DRAW_LIGHTPOINTS)
    {
        pimpl_->node_descr.controllers.draw_lightpoint = in_place();
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

aoa_writer::node_ptr aoa_writer::node::set_cvbox_spec(geom::rectangle_3f const & box)
{
    if(!pimpl_->node_descr.controllers.object_param_controller)
    {
        pimpl_->node_descr.controllers.object_param_controller = in_place();
    }

    refl::node::controllers_t::control_object_param_data::cv_box cvbox(box);
    pimpl_->node_descr.controllers.object_param_controller->cvbox = cvbox;
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_cvsphere_spec(geom::sphere_3f const & sphere)
{
    if(!pimpl_->node_descr.controllers.object_param_controller)
    {
        pimpl_->node_descr.controllers.object_param_controller = in_place();
    }

    refl::node::controllers_t::control_object_param_data::cv_sphere cvsphere(sphere);
    pimpl_->node_descr.controllers.object_param_controller->cvsphere = cvsphere;
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_control_pos_key_spec(float key, geom::point_3f pos)
{
    if(!pimpl_->node_descr.controllers.control_pos)
    {
        pimpl_->node_descr.controllers.control_pos = in_place();
    }

    pimpl_->node_descr.controllers.control_pos->keys.emplace_back(key, pos.x, pos.y, pos.z);
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_control_rot_key_spec(float key, geom::quaternionf rot)
{
    if(!pimpl_->node_descr.controllers.control_rot)
    {
        pimpl_->node_descr.controllers.control_rot = in_place();
    }

    pimpl_->node_descr.controllers.control_rot->keys.emplace_back(key, rot.get_v().x, rot.get_v().y, rot.get_v().z, rot.get_w());
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_control_ref_node_spec(string name, string sub_channel)
{
    if(!pimpl_->node_descr.controllers.node_ref)
    {
        pimpl_->node_descr.controllers.node_ref = in_place();
    }

    *pimpl_->node_descr.controllers.node_ref = {name, sub_channel};

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_control_lod_spec(float radius, vector<float> metric)
{
    pimpl_->node_descr.controllers.control_lod = in_place();

    std::sort(metric.begin(), metric.end(), std::greater<>());
    
    pimpl_->node_descr.controllers.control_lod->radius = radius;
    pimpl_->node_descr.controllers.control_lod->lod_pixel = move(metric);

    return node_ptr();
}

aoa_writer::node_ptr aoa_writer::node::add_ref_node_arg_spec(string chan, string type, float value)
{
    assert(pimpl_->node_descr.controllers.node_ref);
    if(!pimpl_->node_descr.controllers.node_ref->args)
    {
        pimpl_->node_descr.controllers.node_ref->args = in_place();
    }
    
    pimpl_->node_descr.controllers.node_ref->args->list.emplace_back(chan, *cpp_utils::string_to_enum<refl::node::arg_type>(type), value);
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_translation(geom::point_3f pos)
{
    return add_control_pos_key_spec(0, pos);
}

aoa_writer::node_ptr aoa_writer::node::set_rotation(geom::quaternionf rot)
{
    return add_control_rot_key_spec(0, rot);
}

aoa_writer::node_ptr aoa_writer::node::set_channel_file(string name)
{
    pimpl_->node_descr.channel_filename = name;
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_control_light_power_spec(string channel, vector<std::tuple<float, float>> keys, pair<arg_out_of_range_action, arg_out_of_range_action> out_of_range)
{
    if(!pimpl_->node_descr.controllers.control_light_power)
    {
        pimpl_->node_descr.controllers.control_light_power = in_place();
    }

    pimpl_->node_descr.controllers.control_light_power->keys = keys;
    pimpl_->node_descr.controllers.control_light_power->channel = channel;
    pimpl_->node_descr.controllers.control_light_power->out_of_range_actions = {out_of_range.first, out_of_range.second};

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_float_arg_spec(string channel, float def_value)
{
    return add_arg_spec(channel, "FLOAT", def_value);
}

aoa_writer::node_ptr aoa_writer::node::add_arg_spec(string channel, string type, float value)
{
    if(!pimpl_->node_descr.args)
    {
        pimpl_->node_descr.args = in_place();
    }
    
    pimpl_->node_descr.args->list.emplace_back(channel, *cpp_utils::string_to_enum<refl::node::arg_type>(type), value);
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_collision_stream_spec(pair<unsigned, unsigned> vertex_offset_size, pair<unsigned, unsigned> index_offset_size)
{
    if(!pimpl_->node_descr.controllers.object_param_controller)
    {
        pimpl_->node_descr.controllers.object_param_controller = in_place();
    }

    collision_buffer_stream col_stream;
    col_stream.index_offset_size.offset = index_offset_size.first;
    col_stream.index_offset_size.size = index_offset_size.second;
    col_stream.vertex_offset_size.offset = vertex_offset_size.first;
    col_stream.vertex_offset_size.size = vertex_offset_size.second;

    pimpl_->node_descr.controllers.object_param_controller->buffer.collision_stream = col_stream;

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_cvmesh_spec(unsigned vao, std::pair<unsigned, unsigned> vertex_offset_size, std::pair<unsigned, unsigned> index_offset_size)
{
    pimpl_->node_descr.controllers.collision_volume = in_place();

    auto& collsion_volume = pimpl_->node_descr.controllers.collision_volume.value();
    std::remove_reference_t<decltype(collsion_volume.meshes)>::value_type mesh;

    mesh.vertex_vao_offset_size.format = vao;
    mesh.vertex_vao_offset_size.offset = vertex_offset_size.first;
    mesh.vertex_vao_offset_size.size = vertex_offset_size.second;

    mesh.index_offset_size.offset = index_offset_size.first;
    mesh.index_offset_size.size = index_offset_size.second;

    collsion_volume.meshes.emplace_back(mesh);

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_mesh_spec(geom::rectangle_3f bbox, unsigned offset, unsigned count, unsigned base_vertex, unsigned num_vertices, string material, string shadow_material, std::pair<unsigned, unsigned> vao_ref)
{
    if(!pimpl_->node_descr.mesh)
    {
        pimpl_->node_descr.mesh = in_place();
    }

    auto& mesh = pimpl_->node_descr.mesh.value();

    pimpl_->node_descr.controllers.draw_mesh = in_place();

    refl::node::mesh_t::mesh_face mesh_geom;

    mesh_geom.with_shadow_mat = in_place();

    mesh_geom.with_shadow_mat->id = 0;
    mesh_geom.with_shadow_mat->offset = offset;
    mesh_geom.with_shadow_mat->count = count;
    mesh_geom.with_shadow_mat->base_vertex = base_vertex;
    mesh_geom.with_shadow_mat->mat = material;
    mesh_geom.with_shadow_mat->shadow_mat = shadow_material;
    mesh_geom.with_shadow_mat->num_vertices = num_vertices;

    mesh.vao_ref.geom_stream_id = vao_ref.first;
    mesh.vao_ref.vao_id = vao_ref.second;

    mesh.bbox.rect = bbox;

    mesh.face_array.push_back(mesh_geom);

    add_flags(aoa_writer::DRAW_MESH);

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_collision_mesh_impl(const char * data, size_t size, size_t num_vertices, vector<face> const & faces, vector<vertex_attribute> format)
{
    buffer_chunk chunk;
    chunk.kind = buffer_chunk::kind_t::COLLISION_MESH;
    chunk.data.resize(size);
    std::copy(data, data + size, chunk.data.begin());
    chunk.vertex_format = format;
    chunk.faces = faces;
    chunk.num_vertices = num_vertices;
    chunk.node = shared_from_this();

    pimpl_->buffer_chunks.emplace_back(std::move(chunk));
    return shared_from_this();
}


aoa_writer::node_ptr aoa_writer::node::add_mesh_impl(geom::rectangle_3f bbox, const char * data, size_t size, size_t num_vertices, vector<vertex_attribute> format, vector<face> const & faces, float lod, string material, string shadow_material)
{
    buffer_chunk chunk;
    chunk.kind = buffer_chunk::kind_t::MESH;
    chunk.data.resize(size);
    std::copy(data, data + size, chunk.data.begin());
    chunk.vertex_format = format;
    chunk.faces = faces;
    chunk.lod = lod;
    chunk.material = material;
    chunk.shadow_material = shadow_material;
    chunk.num_vertices = num_vertices;
    chunk.node = shared_from_this();
    chunk.bbox = bbox;
    pimpl_->buffer_chunks.emplace_back(std::move(chunk));
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::add_geometry_stream(float lod, pair<unsigned, unsigned> vertex_offset_size, pair<unsigned, unsigned> index_offset_size)
{
    if(!pimpl_->node_descr.controllers.object_param_controller)
    {
        pimpl_->node_descr.controllers.object_param_controller = in_place();
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
        pimpl_->node_descr.controllers.object_param_controller = in_place();
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
        pimpl_->node_descr.controllers.object_param_controller = in_place();
    }

    light_buffer_stream spot_lights;
    spot_lights.light_offset_size.offset = offset;
    spot_lights.light_offset_size.size = size;
    spot_lights.type = refl::light_t::SPOT;
    pimpl_->node_descr.controllers.object_param_controller->buffer.light_streams.push_back(spot_lights);

    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_omni_lights(unsigned offset, unsigned size)
{
    if(!pimpl_->node_descr.lightpoint2)
        pimpl_->node_descr.lightpoint2 = in_place();

    add_flags(aoa_writer::DRAW_LIGHTPOINTS);

    pimpl_->node_descr.lightpoint2->omni_offset_size = refl::offset_size{ offset, size };
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_spot_lights(unsigned offset, unsigned size)
{
    if(!pimpl_->node_descr.lightpoint2)
        pimpl_->node_descr.lightpoint2 = in_place();

    add_flags(aoa_writer::DRAW_LIGHTPOINTS);

    pimpl_->node_descr.lightpoint2->spot_offset_size = refl::offset_size{ offset, size };
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_lights_class(unsigned cls)
{
    assert(cls != 0 && pimpl_->node_descr.lightpoint2);

    pimpl_->node_descr.lightpoint2->clazz = cls;
    return shared_from_this();
}

aoa_writer::node_ptr aoa_writer::node::set_name(string name)
{
    pimpl_->node_descr.name = name;
    return shared_from_this();
}

string aoa_writer::node::get_name() const
{
    return pimpl_->node_descr.name;
}

void * aoa_writer::node::get_underlying()
{
    return &pimpl_->node_descr;
}


}

///////////////////////////////////////////  END OF NODE ///////////////////////////////////////////

namespace aurora
{

namespace
{

constexpr uint32_t c_GP_Version = 300;
constexpr uint32_t c_GP_BaseFileMarker = ('D' << 8) | 'O';
constexpr uint32_t AOD_HEADER_SIZE = sizeof(c_GP_Version) + sizeof(c_GP_BaseFileMarker) + 4 * sizeof(uint32_t);

auto index_offset = [](unsigned len) -> unsigned { return len * sizeof(face); };
auto vertex_offset = [](unsigned len) -> unsigned { return len * sizeof(vertex_info); };

struct buffer_chunk_cmp
{
    bool operator()(buffer_chunk const& l, buffer_chunk const& r)
    {
        if(l.vertex_format.size() == r.vertex_format.size())
        {
            auto N = l.vertex_format.size();
            for(size_t i = 0; i < N; i++)
            {
                if(l.vertex_format[i] == r.vertex_format[i])
                    continue;
                else
                    return l.vertex_format[i] < r.vertex_format[i];
            }

            // if attrs are equal, compare by LODs
            if(l.lod == r.lod)
                return false;
            else
                return l.lod < r.lod;

        }
        else
            return l.vertex_format.size() < r.vertex_format.size();
    }
};

struct collision_buffer
{
    size_t size() const
    {
        return 0u;
    }
};

}

struct aoa_writer::impl
{
    impl(string const& name, aoa_writer& self)
        : filename_(name)
        , self_(self)
        , buffer_file_(fs::path(name).filename().replace_extension("aod").string())
        , file_(std::ios_base::binary | std::ios_base::out | std::ios_base::in)
    {
        file_.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    }

    void write(const void* data, size_t size)
    {
        file_.write(reinterpret_cast<const char*>(data), size);
    }

    void write_header()
    {
        unsigned zeros[4] = {};
        write(&c_GP_BaseFileMarker, sizeof(unsigned));
        write(&c_GP_Version, sizeof(unsigned));
        write(zeros, sizeof(zeros));    
    }

    void write_aoa()
    {
        std::for_each(rbegin(nodes_), rend(nodes_),
            [this](auto& n)
            {
                aoa_descr_.nodes.push_back(*(refl::node*)n->get_underlying());
            }
        );

        write_processor proc;
        reflect(proc, aoa_descr_);

        std::ofstream aoa_file(filename_);
        aoa_file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
        aoa_file << proc.result();
    }

    unsigned index_buffer_size() const
    {
        size_t result = 0;
        for(auto& n : nodes_)
        {
            for(auto& c: n->pimpl_->buffer_chunks)
            {
                if(c.kind == buffer_chunk::kind_t::MESH)
                    result += c.faces.size() * sizeof(std::remove_reference_t<decltype(c.faces)>::value_type);
            }
        }
        return result;
    }

    unsigned current_buffer_offset()
    {
        return file_.tellp();
    }

    pair<unsigned, unsigned> process_mesh_chunks(vector<buffer_chunk>& nodes_buffer_chunks)
    {
        if(nodes_buffer_chunks.empty())
            return {0, 0};

        unsigned whole_index_buffer_size = 0;
        std::for_each(begin(nodes_buffer_chunks), end(nodes_buffer_chunks), 
            [&](auto const& c) { whole_index_buffer_size += index_offset(c.faces.size()); });

        // sort by attrs, then by lod
        std::sort(begin(nodes_buffer_chunks), end(nodes_buffer_chunks), buffer_chunk_cmp());

        refl::data_buffer& buffer_descr = aoa_descr_.buffer_data;
        unsigned current_vao = buffer_descr.vaos.size();
        unsigned current_stream = 0;
        unsigned mesh_vertex_offset = 0;
        unsigned geom_stream_vertex_offset = 0;
        unsigned geom_stream_index_offset = 0;
        unsigned index_buffer_size = 0;
        unsigned vertex_buffer_size = 0;

        {
            refl::data_buffer::vao_buffer vao_descr;
            vao_descr.vertex_format_offset.offset = whole_index_buffer_size + vertex_buffer_size;
            vao_descr.format.attributes = nodes_buffer_chunks.begin()->vertex_format;

            buffer_descr.vaos.push_back(vao_descr);
        }

        for(int i = 0; i < nodes_buffer_chunks.size(); i++)
        {
            auto& chunk = nodes_buffer_chunks[i];
            bool add_geom_stream = false;

            if(chunk.vertex_format != buffer_descr.vaos.back().format.attributes)
            {
                refl::data_buffer::vao_buffer vao_descr;
                vao_descr.vertex_format_offset.offset = whole_index_buffer_size + vertex_buffer_size;
                vao_descr.format.attributes = chunk.vertex_format;

                current_vao = buffer_descr.vaos.size();
                buffer_descr.vaos.push_back(vao_descr);
                add_geom_stream = true;
            }
            else if(i - 1 >= 0)
            {
                if(nodes_buffer_chunks[i - 1].lod != chunk.lod)
                {
                    add_geom_stream = true;
                }
            }

            if(add_geom_stream)
            {
                current_stream++;
                self_.get_root_node()->add_geometry_stream(chunk.lod,
                    { geom_stream_vertex_offset, vertex_buffer_size - geom_stream_vertex_offset },
                    { geom_stream_index_offset, index_buffer_size - geom_stream_index_offset });

                geom_stream_vertex_offset = vertex_buffer_size;
                geom_stream_index_offset = index_buffer_size;
                mesh_vertex_offset = 0;
            }

            auto node = chunk.node.lock();
            node->add_mesh_spec(chunk.bbox,
                                index_buffer_size / sizeof(face) * 3, // offset
                                chunk.faces.size(),         // count
                                mesh_vertex_offset,         // base_vertex
                                chunk.num_vertices, chunk.material, chunk.shadow_material, { current_stream, current_vao });

            mesh_vertex_offset += chunk.num_vertices;
            index_buffer_size += index_offset(chunk.faces.size());
            vertex_buffer_size += chunk.data.size();
        }

        // add last stream
        self_.get_root_node()->add_geometry_stream(prev(nodes_buffer_chunks.end())->lod,
            { geom_stream_vertex_offset, vertex_buffer_size - geom_stream_vertex_offset },
            { geom_stream_index_offset, index_buffer_size - geom_stream_index_offset });

        assert(index_buffer_size == whole_index_buffer_size);
        return {index_buffer_size, vertex_buffer_size};
    }

    pair<unsigned, unsigned> process_collision_chunks(vector<buffer_chunk>& nodes_buffer_chunks)
    {
        if(nodes_buffer_chunks.empty())
            return {0, 0};
        
        // format, as I get it, must be the same because there is only one stream
        // at least I've not seen more than one (SL)
        for(int i = 1; i < nodes_buffer_chunks.size(); ++i)
        {
            assert(nodes_buffer_chunks[i-1].vertex_format == nodes_buffer_chunks[i].vertex_format);
        }

        refl::data_buffer& buffer_descr = aoa_descr_.buffer_data;
        unsigned index_buffer_size = 0;
        unsigned vertex_buffer_size = 0;
        unsigned current_vao = buffer_descr.vaos.size();

        // add vao spec for collision data
        {
            refl::data_buffer::vao_buffer vao_descr;
            vao_descr.vertex_format_offset.offset = 0;
            vao_descr.format.attributes = nodes_buffer_chunks.begin()->vertex_format;

            buffer_descr.vaos.push_back(vao_descr);
        }

        for(int i = 0; i < nodes_buffer_chunks.size(); i++)
        {
            auto& chunk = nodes_buffer_chunks[i];
            unsigned chunk_index_size = index_offset(chunk.faces.size());
            unsigned chunk_vertex_size = chunk.data.size();

            auto node = chunk.node.lock();

            node->add_cvmesh_spec(current_vao, {vertex_buffer_size, chunk_vertex_size},
                                               {index_buffer_size, chunk_index_size});

            index_buffer_size += chunk_index_size;
            vertex_buffer_size += chunk_vertex_size;
        }

        return { index_buffer_size, vertex_buffer_size };
    }

    void write_index_data(vector<buffer_chunk>const & chunks)
    {
        for(auto const& chunk : chunks)
        {
            write(chunk.faces.data(), index_offset(chunk.faces.size()));
        }
    }

    void write_vertex_data(vector<buffer_chunk>const & chunks)
    {
        for(auto const& chunk : chunks)
        {
            write(chunk.data.data(), chunk.data.size());
        }
    }

    void write_lights_data()
    {
        size_t offset = current_buffer_offset();
        std::for_each(begin(nodes_), end(nodes_),
        [this, &offset](auto const& n)
        {
            if(n->pimpl_->lights_buf.omni_size())
            {
                n->add_omnilights_stream(current_buffer_offset(), n->pimpl_->lights_buf.omni_size());
                n->pimpl_->lights_buf.write_omni(*this);
                assert(offset + n->pimpl_->lights_buf.omni_size() == current_buffer_offset());
                offset += n->pimpl_->lights_buf.omni_size();
            }
            if(n->pimpl_->lights_buf.spot_size())
            {
                n->add_spotlights_stream(current_buffer_offset(), n->pimpl_->lights_buf.spot_size());
                n->pimpl_->lights_buf.write_spot(*this);
                assert(offset + n->pimpl_->lights_buf.spot_size() == current_buffer_offset());
                offset += n->pimpl_->lights_buf.spot_size();
            }
        });
    }

    size_t lights_buffer_size()
    {
        size_t result = 0;
        for(auto& n: nodes_)
        {
            result += n->pimpl_->lights_buf.omni_size();
            result += n->pimpl_->lights_buf.spot_size();
        }
        return result;
    }

    aoa_writer& self_;
    string   filename_;
    string   buffer_file_;
    std::stringstream file_;
    node_ptr root_;
    vector<node_ptr> nodes_;
    refl::aurora_format aoa_descr_;

    collision_buffer      collision_buffer_;
};

aoa_writer::aoa_writer(string path)
    : pimpl_(std::make_unique<impl>(path, *this))
{
}

aoa_writer::~aoa_writer() = default;

void aoa_writer::save_data()
{
    vector<buffer_chunk> mesh_chunks;
    for(auto& n: pimpl_->nodes_)
    {
        std::copy_if(begin(n->pimpl_->buffer_chunks), end(n->pimpl_->buffer_chunks), back_inserter(mesh_chunks), 
            [](auto const& c){ return c.kind == buffer_chunk::kind_t::MESH; });
    }

    vector<buffer_chunk> collision_chunks;

    for(auto& n : pimpl_->nodes_)
    {
        std::copy_if(begin(n->pimpl_->buffer_chunks), end(n->pimpl_->buffer_chunks), back_inserter(collision_chunks),
            [](auto const& c) { return c.kind == buffer_chunk::kind_t::COLLISION_MESH; });
    }

    auto [index_buffer_size, vertex_buffer_size] = pimpl_->process_mesh_chunks(mesh_chunks);
    auto [col_index_buffer_size, col_vertex_buffer_size] = pimpl_->process_collision_chunks(collision_chunks);
    unsigned collision_buffer_size = col_index_buffer_size + col_vertex_buffer_size;
    unsigned light_buffer_size = pimpl_->lights_buffer_size();

    // THE FOLLOWING CODE IS A BIT MESSY
    // with writes to aod mixed with side effects on aoa because we have to write offsets into aod in aoa
    pimpl_->write_header();
    // write collision data
    assert(pimpl_->current_buffer_offset() == AOD_HEADER_SIZE);
    get_root_node()
        ->set_collision_stream_spec({ pimpl_->current_buffer_offset() + col_index_buffer_size, col_vertex_buffer_size }, 
                                    { pimpl_->current_buffer_offset(), col_index_buffer_size });

    pimpl_->write_index_data(collision_chunks);
    pimpl_->write_vertex_data(collision_chunks);
    assert(pimpl_->current_buffer_offset() == AOD_HEADER_SIZE + collision_buffer_size);
    // write lights data
    pimpl_->write_lights_data();
    assert(pimpl_->current_buffer_offset() == AOD_HEADER_SIZE + light_buffer_size + collision_buffer_size);

    // create buffer file description

    refl::data_buffer& buffer_descr = pimpl_->aoa_descr_.buffer_data;
    buffer_descr.data_buffer_file = pimpl_->buffer_file_;

    buffer_descr.index_file_offset_size = { pimpl_->current_buffer_offset(), index_buffer_size };
    buffer_descr.vertex_file_offset_size = { pimpl_->current_buffer_offset() + index_buffer_size, vertex_buffer_size };

    // write index and vertex data
    pimpl_->write_index_data(mesh_chunks);
    pimpl_->write_vertex_data(mesh_chunks);
    assert(pimpl_->current_buffer_offset() == AOD_HEADER_SIZE + light_buffer_size + collision_buffer_size + index_buffer_size + vertex_buffer_size);
    // write sizes of section to the header

    pimpl_->file_.seekp(8);

    pimpl_->write(&collision_buffer_size, sizeof(unsigned));
    pimpl_->write(&light_buffer_size, sizeof(unsigned));
    pimpl_->write(&index_buffer_size, sizeof(unsigned));
    pimpl_->write(&vertex_buffer_size, sizeof(unsigned));

    // finally, write aoa file
    osgDB::makeDirectoryForFile(pimpl_->filename_);
    pimpl_->write_aoa();
    std::ofstream aod_file(fs::path(pimpl_->filename_).replace_extension("aod").string(), std::ios_base::binary | std::ios_base::out);
    aod_file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    aod_file << pimpl_->file_.rdbuf();
}

void aoa_writer::node::set_omni_lights_buffer_data(vector<aod::omni_light> const & data)
{
    pimpl_->lights_buf.omni_lights = data;
}

void aoa_writer::node::set_spot_lights_buffer_data(vector<aod::spot_light> const & data)
{
    pimpl_->lights_buf.spot_lights = data;
}

aoa_writer::node_ptr aoa_writer::get_root_node()
{
    if(pimpl_->root_)
        return pimpl_->root_;

    pimpl_->root_ = create_node();

    return pimpl_->root_->set_name(fs::path(pimpl_->filename_).stem().string())
        ->add_flags(aoa_writer::GLOBAL_NODE);
}

aoa_writer::node_ptr aurora::aoa_writer::create_top_level_node()
{
    return create_node()->add_flags(aoa_writer::GLOBAL_NODE);
}

aoa_writer & aoa_writer::add_material(string name, material_info const & mat)
{
    // don't generate material
    if(!mat.explicit_material.empty())
        return *this;

    refl::material_list::material mat_descr;
    mat_descr.name = name;

    switch(mat.textures.size())
    {
    case 0:
    case 1:
        mat_descr.link = "__D";
        break;
    case 2:
        mat_descr.link = "__DN";
        break;
    case 3:
    default:
        mat_descr.link = "__DEAFG";
        break;
    }

    for(unsigned i = 0; i < std::min(size_t(3), mat.textures.size()); i++)
    {
        mat_descr.textures.push_back(refl::material_list::material::material_group{ i, mat.textures[i] });
    }

    pimpl_->aoa_descr_.materials.list.push_back(mat_descr);

    return *this;
}

aoa_writer::node_ptr aoa_writer::create_node()
{
    pimpl_->nodes_.emplace_back(make_shared<node>(*this));
    return pimpl_->nodes_.back();
}

}