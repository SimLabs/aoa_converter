#include "aurora_aoa_writer.h"
#include "write_aoa_visitor.h"
#include "aurora_format.h"
#include "aurora_write_processor.h"

namespace aurora
{

using geometry_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::geometry_buffer_stream;
using light_buffer_stream = refl::node::controllers_t::control_object_param_data::data_buffer::light_buffer_stream;

struct buffer_chunk
{
    enum kind_t
    {
        MESH, COLLISION_MESH
    };
    
    kind_t kind           = MESH;
    float             lod = 0.;
    unsigned num_vertices = 0;
    vector<vertex_attribute> vertex_format;
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

aoa_writer::node_ptr aoa_writer::node::add_mesh_spec(geom::rectangle_3f bbox, unsigned offset, unsigned count, unsigned base_vertex, unsigned num_vertices, string material, string shadow_material, std::pair<unsigned, unsigned> vao_ref)
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
    mesh_geom.params.mat = material;
    mesh_geom.params.shadow_mat = shadow_material;
    mesh_geom.params.num_vertices = num_vertices;

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

    add_flags(aoa_writer::DRAW_LIGHTPOINTS);

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

///////////////////////////////////////////  END OF NODE ///////////////////////////////////////////

namespace aurora
{

using vertex_attribute = refl::data_buffer::vao_buffer::vertex_format::vertex_attribute;

using aurora::refl::vertex_attrs::type_t;
using aurora::refl::vertex_attrs::mode_t;

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

struct lights_buffer
{
    vector<aod::omni_light> omni_lights;
    vector<aod::spot_light> spot_lights;

    size_t size() const
    {
        return omni_size() + spot_size();
    }

    template<class T>
    void write(T& out) const
    {
        out.write(reinterpret_cast<const char*>(omni_lights.data()), omni_size());
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
        , file_(fs::path(name).replace_extension("aod").string(), std::ios_base::binary | std::ios_base::out)
    {
        unsigned zeros[4] = {};
        write(&c_GP_BaseFileMarker, sizeof(unsigned));
        write(&c_GP_Version, sizeof(unsigned));
        write(zeros, sizeof(zeros));
    }

    void write(const void* data, size_t size)
    {
        file_.write(reinterpret_cast<const char*>(data), size);
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

    pair<unsigned, unsigned> process_mesh_chunks(vector<buffer_chunk>& nodes_buffer_chunks)
    {
        unsigned whole_index_buffer_size = 0;
        std::for_each(begin(nodes_buffer_chunks), end(nodes_buffer_chunks), 
            [&](auto const& c) { whole_index_buffer_size += index_offset(c.faces.size()); });

        // sort by attrs, then by lod
        std::sort(begin(nodes_buffer_chunks), end(nodes_buffer_chunks), buffer_chunk_cmp());

        refl::data_buffer& buffer_descr = aoa_descr_.buffer_data;
        unsigned current_vao = 0;
        unsigned current_stream = 0;
        unsigned mesh_vertex_offset = 0;
        unsigned geom_stream_vertex_offset = 0;
        unsigned geom_stream_index_offset = 0;
        unsigned index_buffer_size = 0;
        unsigned vertex_buffer_size = 0;

        if(nodes_buffer_chunks.size() > 0)
        {
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
        }

        assert(index_buffer_size == whole_index_buffer_size);
        return {index_buffer_size, vertex_buffer_size};
    }

    void process_collision_chunks(vector<buffer_chunk>& nodes_buffer_chunks)
    {
        std::sort(begin(nodes_buffer_chunks), end(nodes_buffer_chunks), buffer_chunk_cmp());
    }

    aoa_writer& self_;
    string   filename_;
    string   buffer_file_;
    ofstream file_;
    node_ptr root_;
    vector<node_ptr> nodes_;
    refl::aurora_format aoa_descr_;

    collision_buffer      collision_buffer_;
    lights_buffer         lights_buffer_;
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
        std::copy_if(begin(n->pimpl_->buffer_chunks), end(n->pimpl_->buffer_chunks), back_inserter(mesh_chunks),
            [](auto const& c) { return c.kind == buffer_chunk::kind_t::COLLISION_MESH; });
    }

    pimpl_->process_collision_chunks(collision_chunks);
    auto [index_buffer_size, vertex_buffer_size] = pimpl_->process_mesh_chunks(mesh_chunks);
    unsigned collision_buffer_size = pimpl_->collision_buffer_.size();
    unsigned light_buffer_size = pimpl_->lights_buffer_.size();

    // create buffer file description

    refl::data_buffer& buffer_descr = pimpl_->aoa_descr_.buffer_data;
    buffer_descr.data_buffer_file = pimpl_->buffer_file_;

    buffer_descr.index_file_offset_size = { AOD_HEADER_SIZE + collision_buffer_size + light_buffer_size, index_buffer_size };
    buffer_descr.vertex_file_offset_size = { AOD_HEADER_SIZE + collision_buffer_size + light_buffer_size + index_buffer_size, vertex_buffer_size };

    // write aoa file
    write_processor proc;

    assert(pimpl_->root_);
    pimpl_->aoa_descr_.nodes.push_back(*(refl::node*)pimpl_->root_->get_underlying());

    for(auto& n : pimpl_->nodes_)
    {
        pimpl_->aoa_descr_.nodes.push_back(*(refl::node*)n->get_underlying());
    }

    reflect(proc, pimpl_->aoa_descr_);

    std::ofstream aoa_file(pimpl_->filename_);
    aoa_file << proc.result();

    // write data

    pimpl_->lights_buffer_.write(*pimpl_);
    for(auto const& chunk: mesh_chunks)
    {
        pimpl_->write(chunk.faces.data(), index_offset(chunk.faces.size()));
    }
    for(auto const& chunk : mesh_chunks)
    {
        pimpl_->write(chunk.data.data(), chunk.data.size());
    }

    // write sizes of section to the header

    pimpl_->file_.seekp(8);

    pimpl_->write(&collision_buffer_size, sizeof(unsigned));
    pimpl_->write(&light_buffer_size, sizeof(unsigned));
    pimpl_->write(&index_buffer_size, sizeof(unsigned));
    pimpl_->write(&vertex_buffer_size, sizeof(unsigned));
}

void aoa_writer::set_omni_lights_buffer_data(vector<aod::omni_light> const & data)
{
    pimpl_->lights_buffer_.omni_lights = data;
}

void aoa_writer::set_spot_lights_buffer_data(vector<aod::spot_light> const & data)
{
    pimpl_->lights_buffer_.spot_lights = data;
}

aoa_writer::node_ptr aoa_writer::get_root_node()
{
    if(pimpl_->root_)
        return pimpl_->root_;

    pimpl_->root_ = make_shared<node>(*this);

    return pimpl_->root_->set_name(fs::path(pimpl_->filename_).stem().string())
        ->add_flags(aoa_writer::GLOBAL_NODE)
        ->add_flags(aoa_writer::TREAT_CHILDREN)
        ->add_omnilights_stream(AOD_HEADER_SIZE + pimpl_->collision_buffer_.size(),  pimpl_->lights_buffer_.omni_size())
        ->add_spotlights_stream(AOD_HEADER_SIZE + pimpl_->collision_buffer_.size() + pimpl_->lights_buffer_.omni_size(), pimpl_->lights_buffer_.spot_size());
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

    for(unsigned i = 0; i < mat.textures.size(); i++)
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