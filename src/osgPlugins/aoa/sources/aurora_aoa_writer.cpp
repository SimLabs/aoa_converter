#include "aurora_aoa_writer.h"
#include "write_aoa_visitor.h"
#include "aurora_format.h"
#include "aurora_write_processor.h"

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
    impl(string const& name)
        : filename_(name)
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

    string   filename_;
    string   buffer_file_;
    ofstream file_;
    node_ptr root_;
    vector<node_ptr> nodes_;
    refl::aurora_format aoa_descr_;

    collision_buffer      collision_buffer_;
    lights_buffer         lights_buffer_;
    vector<face>          index_buffer_data_;
    vector<vertex_info>   vertex_buffer_data_;
};

aoa_writer::aoa_writer(string path)
    : pimpl_(std::make_unique<impl>(path))
{
}

aoa_writer::~aoa_writer() = default;

void aoa_writer::save_data()
{
    unsigned collision_buffer_size = pimpl_->collision_buffer_.size();
    unsigned light_buffer_size = pimpl_->lights_buffer_.size();
    unsigned index_buffer_size = pimpl_->index_buffer_data_.size() * sizeof(std::remove_reference_t<decltype(pimpl_->index_buffer_data_)>::value_type);
    unsigned vertex_buffer_size = pimpl_->vertex_buffer_data_.size() * sizeof(std::remove_reference_t<decltype(pimpl_->vertex_buffer_data_)>::value_type);

    // create buffer file description

    refl::data_buffer buffer_descr;
    buffer_descr.data_buffer_file = pimpl_->buffer_file_;

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
    pimpl_->aoa_descr_.buffer_data = buffer_descr;

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
    pimpl_->write(pimpl_->index_buffer_data_.data(), index_buffer_size);
    pimpl_->write(pimpl_->vertex_buffer_data_.data(), vertex_buffer_size);

    // write sizes of section to the header

    pimpl_->file_.seekp(8);

    pimpl_->write(&collision_buffer_size, sizeof(unsigned));
    pimpl_->write(&light_buffer_size, sizeof(unsigned));
    pimpl_->write(&index_buffer_size, sizeof(unsigned));
    pimpl_->write(&vertex_buffer_size, sizeof(unsigned));
}

void aoa_writer::set_index_buffer_data(vector<face> const & data)
{
    pimpl_->index_buffer_data_ = data;
}

void aoa_writer::set_vertex_buffer_data(vector<vertex_info> const & data)
{
    pimpl_->vertex_buffer_data_ = data;
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
        ->add_geometry_stream(250., 
            { 0, vertex_offset(pimpl_->vertex_buffer_data_.size()) }, 
            { 0, index_offset(pimpl_->index_buffer_data_.size()) })
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