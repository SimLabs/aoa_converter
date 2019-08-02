#include "aurora_format.h"
#include "aurora_aoa_reader.h"
#include "aoa_to_osg_data.h"

#include <osg/Group>
#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <filesystem>
#include <optional>

using std::filesystem::path;

namespace aurora
{

struct node_cmp
{
    using is_transparent = void;

    bool operator()(refl::node const& l, refl::node const& r) const
    {
        return string(l.name) < string(r.name);
    }

    bool operator()(string const& l, refl::node const& r) const
    {
        return l < string(r.name);
    }

    bool operator()(refl::node const& l, string const& r) const
    {
        return string(l.name) < r;
    }
};

struct osg_node_cmp
{
    using is_transparent = void;

    bool operator()(osg::ref_ptr<osg::Node> const& l, osg::ref_ptr<osg::Node> const& r) const
    {
        return l->getName() < r->getName();
    }

    bool operator()(string const& l, osg::ref_ptr<osg::Node> const& r) const
    {
        return l < r->getName();
    }

    bool operator()(osg::ref_ptr<osg::Node> const& l, string const& r) const
    {
        return l->getName() < r;
    }
};

struct node_context
{
    node_context(string const& aoa_path, refl::aurora_format aoa, refl::node root_node)
        : root_node_(root_node)
        , data_buffer_description_(aoa.buffer_data)
    {
        std::ifstream buffer_file(path(aoa_path).parent_path() / string(aoa.buffer_data.data_buffer_file), std::ios_base::binary);
        std::ostringstream ss;
        ss << buffer_file.rdbuf();
        auto content = ss.str();

        data_buffer_ = {content.begin(), content.end()};

        build_vertex_arrays_cache(aoa);
    }

    osg::Array* get_stream_vertex_attr_array(unsigned stream, unsigned attr_num) const
    {
        auto it = stream_attr_arrays_cache_.find(std::pair(stream, attr_num));
        return it != stream_attr_arrays_cache_.end() ? it->second.get() : nullptr;
    }

    osg::ref_ptr<osg::DrawElements> get_mesh_draw_elements(unsigned vao, unsigned stream_id, unsigned offset, unsigned count, unsigned base_vertex) const
    {
        // Vertex array (described by VAO_BUFFER section in aoa) can contain multiple streams.
        // As we are extracting each individual mesh chunks
        // we have to adjust the base vertex so that the indices are counting from the start of the stream
        // not from the start of the whole vertex array.
        unsigned previous_streams_offset = 0;

        for(unsigned i = 0; i < stream_id; ++i)
        {
            auto const& stream = root_node_.controllers.object_param_controller->buffer.geometry_streams.at(i);
            auto [buffer_id, buffer_format] = get_buffer_format_by_vertex_offset(stream.vertex_offset_size.offset);
            if(buffer_id == vao)
                previous_streams_offset += stream.vertex_offset_size.size;
        }

        auto b = data_buffer_.begin();
        auto e = data_buffer_.begin();

        auto stream = shared_streams_.geometry_streams.at(stream_id);
        auto buf_chunk = data_buffer_description_.vaos.at(vao);
        auto [not_used, stride] = get_attribute_array_offset_stride(buf_chunk, 0);

        auto [buffer_id, buffer_format] = get_buffer_format_by_vertex_offset(stream.vertex_offset_size.offset);
        assert(vao == buffer_id);

        std::advance(b, data_buffer_description_.index_file_offset_size.offset);
        std::advance(e, data_buffer_description_.index_file_offset_size.offset + stream.index_offset_size.size);
        auto result = elements_array_to_osg(b, e, buf_chunk.vertex_format_offset.format, offset, count, base_vertex - previous_streams_offset / stride);
        assert(result->getNumIndices() > 0);
        return result;
    }

    pair<unsigned, unsigned> get_attribute_array_offset_stride(refl::data_buffer::vao_buffer const& buffer_format, unsigned num) const
    {
        pair<unsigned, unsigned> result{0, 0};
        unsigned i = 0;
        for(auto a : buffer_format.format.attributes)
        {
            if(i == num)
                result.first = result.second;
            i++;
            switch(a.type)
            {
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::UNSIGNED_INT_2_10_10_10_REV:
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::INT_2_10_10_10_REV:
                    result.second += sizeof(uint32_t);
                    break;
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::FLOAT:
                    result.second += a.size * sizeof(float);
                    break;
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::HALF_FLOAT:
                    result.second += a.size * sizeof(float) / 2;
                    break;
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::UNSIGNED_INT:
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::INT:
                    result.second += a.size * sizeof(uint32_t);
                    break;
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::UNSIGNED_BYTE:
                case refl::data_buffer::vao_buffer::vertex_format::vertex_attribute::BYTE:
                    result.second += a.size * sizeof(char);
                    break;
                default:
                    assert(false);
            }
        }
        assert(result.second != 0);
        return result;
    }

    std::pair<unsigned, refl::data_buffer::vao_buffer> get_buffer_format_by_vertex_offset(size_t offset) const
    {
        auto result = data_buffer_description_.vaos.end();
        for(auto it = data_buffer_description_.vaos.begin(); it != data_buffer_description_.vaos.end(); ++it)
        {
            if(result == data_buffer_description_.vaos.end() 
                || it->vertex_format_offset.offset > result->vertex_format_offset.offset &&
                   it->vertex_format_offset.offset - data_buffer_description_.index_file_offset_size.size <= offset
            ) 
                result = it;
        }
        assert(result != data_buffer_description_.vaos.end());
        return {std::distance(data_buffer_description_.vaos.begin(), result), *result};
    }

    void build_vertex_arrays_cache(refl::aurora_format aoa)
    {
        auto const& root_node = root_node_;
        assert(root_node.controllers.object_param_controller);

        shared_streams_ = root_node.controllers.object_param_controller->buffer;

        unsigned stream_num = 0;
        for(auto stream: root_node.controllers.object_param_controller->buffer.geometry_streams)
        {
            auto [buffer_id, buffer_format] = get_buffer_format_by_vertex_offset(stream.vertex_offset_size.offset);
            for(unsigned i = 0; i < buffer_format.format.attributes.size(); ++i)
            {
                auto const a = buffer_format.format.attributes[i];
                auto [offset, stride] = get_attribute_array_offset_stride(buffer_format, i);
                auto b = data_buffer_.begin();
                auto e = b;
                // vertex buffer offset + stream offset + attribute offset
                std::advance(b, data_buffer_description_.vertex_file_offset_size.offset + stream.vertex_offset_size.offset);
                std::advance(e, data_buffer_description_.vertex_file_offset_size.offset + stream.vertex_offset_size.offset + stream.vertex_offset_size.size);
                stream_attr_arrays_cache_.emplace(pair{stream_num, a.id}, attribute_array_to_osg(b, e, a.type, a.size, offset, stride));
            } 
            stream_num++;
        }
    }

private:
    refl::node root_node_;
    refl::data_buffer data_buffer_description_;
    refl::node::controllers_t::control_object_param_data::data_buffer shared_streams_;
    std::map<pair<unsigned, unsigned>, osg::ref_ptr<osg::Array>> stream_attr_arrays_cache_;
    vector<char> data_buffer_;
};

osg::ref_ptr<osg::Group> convert_group_node(refl::node const& n, node_context const& context)
{
    return new osg::Group();
}

osg::ref_ptr<osg::Node> osg_geometry_from_aoa_mesh(refl::node::mesh_t mesh, node_context const& context)
{
    osg::ref_ptr<osg::Group> result = new osg::Group();

    for(auto mesh_params: mesh.face_array)
    {
        osg::ref_ptr<osg::Geometry> g = new osg::Geometry();
        // 0 - position
        if(auto a = context.get_stream_vertex_attr_array(mesh.vao_ref.geom_stream_id, 0))
        {
            g->setVertexArray(a);
        }
        // 1 - normal
        if(auto a = context.get_stream_vertex_attr_array(mesh.vao_ref.geom_stream_id, 1))
        {
            g->setNormalArray(a, osg::Array::BIND_PER_VERTEX);
        }
        // 4 - uv
        if(auto a = context.get_stream_vertex_attr_array(mesh.vao_ref.geom_stream_id, 4))
        {
            g->setTexCoordArray(0, a);
        }

        if(mesh_params.with_shadow_mat)
        {
            g->addPrimitiveSet(context.get_mesh_draw_elements(
                mesh.vao_ref.vao_id, 
                mesh.vao_ref.geom_stream_id, 
                mesh_params.with_shadow_mat->offset,
                3 * mesh_params.with_shadow_mat->count,
                mesh_params.with_shadow_mat->base_vertex
            ));
        }

        result->addChild(g);
    }

    return result;
}


osg::ref_ptr<osg::Node> convert_geometry_node(refl::node const& n, node_context const& context)
{
    osg::ref_ptr<osg::Node> result;
    if(n.mesh)
        result = osg_geometry_from_aoa_mesh(*n.mesh, context);
    else
        result = new osg::Node();

    return result;
}

osg::ref_ptr<osg::Node> aoa_node_to_osg_node(refl::node const& n, node_context const& context)
{
    osg::ref_ptr<osg::Node> result;
    if(n.controllers.draw_mesh)
    {
        result = convert_geometry_node(n, context);
    } 
    else
    {
        result = convert_group_node(n, context);
    }

    //std::string node_name = n.name;
    //for(auto& c: node_name)
    //    c = tolower(c);

    result->setName(n.name);
    return result;
}

bool compareChar(char c1, char c2)
{
    if(c1 == c2)
        return true;
    else if(std::toupper(c1) == std::toupper(c2))
        return true;
    return false;
}

/*
 * Case Insensitive String Comparision
 */
bool caseInSensStringCompare(std::string const& str1, std::string const&str2)
{
    return ((str1.size() == str2.size()) &&
             std::equal(str1.begin(), str1.end(), str2.begin(), &compareChar));
}

osg::ref_ptr<osg::Node> aoa_to_osg(string const& path)
{
    auto aoa = read_aoa(path);
    string root_name = std::filesystem::path(path).stem().string();

    std::set<refl::node, node_cmp> nodes{aoa.nodes.begin(), aoa.nodes.end()};
    std::set<osg::ref_ptr<osg::Node>, osg_node_cmp> osg_nodes;

    std::optional<refl::node> root_node;

    for(auto n : nodes)
    {
        if(caseInSensStringCompare(n.name, root_name))
        {
            root_node = n;
            break;
        }
    }

    assert(root_node);

    node_context context(path, aoa, *root_node);

    std::transform(nodes.begin(), nodes.end(), std::inserter(osg_nodes, osg_nodes.end()), 
        [&context](auto const& n)
        {
            return aoa_node_to_osg_node(n, context);
        }
    );

    for(auto const&n : nodes)
    {
        for(auto const& child: n.children.children)
        {
            osg::Group* g = dynamic_cast<osg::Group*>((*osg_nodes.find(n.name)).get());
            if(g)
                g->addChild(*osg_nodes.find(child));
            else
            {
                OSG_WARN << "AOA PLUGIN: node with children is not Group";
            }
        }
    }

    for(auto n: osg_nodes)
    {
        if(caseInSensStringCompare(n->getName(), root_name))
            return n;
    }

    assert(false);
    return nullptr;
}

}