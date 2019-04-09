#pragma once
#include "reflection/reflection.h"

#define DECLARE_AURORA_FIELD(name) \
    constexpr char* Field__## name = "#" #name;

namespace aurora
{

namespace refl
{

DECLARE_AURORA_FIELD(DATA_BUFFER)
DECLARE_AURORA_FIELD(DATA_BUFFER_FILE)
DECLARE_AURORA_FIELD(VERTEX_FILE_OFFSET_SIZE)
DECLARE_AURORA_FIELD(INDEX_FILE_OFFSET_SIZE)
DECLARE_AURORA_FIELD(VAO_VERTEX_FORMAT_OFFSET)
DECLARE_AURORA_FIELD(VAO_NUM_ELEM)
DECLARE_AURORA_FIELD(VAO_BUFFER)
DECLARE_AURORA_FIELD(VERTEX_ATTRIBUTE)
DECLARE_AURORA_FIELD(VERTEX_FORMAT)

DECLARE_AURORA_FIELD(NODE)
DECLARE_AURORA_FIELD(DRAW_ORDER)
DECLARE_AURORA_FIELD(NODE_NAME)
DECLARE_AURORA_FIELD(NODE_SCOPE)
DECLARE_AURORA_FIELD(NODE_FLAGS)
DECLARE_AURORA_FIELD(NODE_CHILDS)
DECLARE_AURORA_FIELD(NODE_CHILDS_COUNT)
DECLARE_AURORA_FIELD(NODE_CHILD_NAME)

DECLARE_AURORA_FIELD(MESH)
DECLARE_AURORA_FIELD(MESH_NUMFACEARRAY)
DECLARE_AURORA_FIELD(MESH_VAO_REF)
DECLARE_AURORA_FIELD(MESH_BSPHERE)
DECLARE_AURORA_FIELD(MESH_BBOX)
DECLARE_AURORA_FIELD(MESH_FACE_ARRAY2)
DECLARE_AURORA_FIELD(MESH_FACE_OFFSET_COUNT_MTLNAME3)

DECLARE_AURORA_FIELD(MATERIAL)
DECLARE_AURORA_FIELD(MATERIAL_LIST)
DECLARE_AURORA_FIELD(MATERIAL_NAME)
DECLARE_AURORA_FIELD(MATERIAL_LINK)
DECLARE_AURORA_FIELD(MATERIAL_GROUP)
DECLARE_AURORA_FIELD(TEXTURE)
DECLARE_AURORA_FIELD(TEXTURE_LINK)

DECLARE_AURORA_FIELD(LOD_PIXEL)
DECLARE_AURORA_FIELD(GEOMETRY_STREAM_NUM_ELEM)
DECLARE_AURORA_FIELD(GEOMETRY_BUFFER_STREAM)

DECLARE_AURORA_FIELD(CONTROLLERS)
DECLARE_AURORA_FIELD(CONTROL_NUMBER)
DECLARE_AURORA_FIELD(CONTROL_OBJECT_PARAM_DATA)
DECLARE_AURORA_FIELD(CONTROL_TREAT_CHILDS)
DECLARE_AURORA_FIELD(CONTROL_DRAW_MESH)
DECLARE_AURORA_FIELD(CONTROL_DRAW_LIGHTPOINT)

DECLARE_AURORA_FIELD(LIGHTS_FILE_OFFSET_SIZE)
DECLARE_AURORA_FIELD(LIGHTS_TYPE)
DECLARE_AURORA_FIELD(LIGHT_BUFFER_STREAM)
DECLARE_AURORA_FIELD(LIGHTS_STREAM_NUM_ELEM)

DECLARE_AURORA_FIELD(LIGHTPOINT2)
DECLARE_AURORA_FIELD(OMNI)
DECLARE_AURORA_FIELD(SPOT)

struct aurora_vector_field_tag
{
    aurora_vector_field_tag(const char* size_field = nullptr)
        : size_field(size_field)
    {}

    const char* size_field;
};

struct quoted_string
{
    string value;

    quoted_string(string s = "")
        : value(s)
    {
    }

    quoted_string& operator=(string s)
    {
        value = s;
        return *this;
    }

    operator string& ()
    {
        return value;
    }

    operator string const& () const
    {
        return value;
    }

    REFL_INNER(quoted_string)
        REFL_ENTRY(value)
    REFL_END()
};

namespace vertex_attrs
{

enum type_t {
    UNSIGNED_INT_2_10_10_10_REV,
    INT_2_10_10_10_REV,
    UNSIGNED_INT,
    INT,
    UNSIGNED_BYTE,
    BYTE,
    FLOAT,
    HALF_FLOAT
};

enum mode_t {
    ATTR_MODE_INT,
    ATTR_MODE_FLOAT,
    ATTR_MODE_FIXED,
    ATTR_MODE_PACKED
};

ENUM_DECL(vertex_attrs::type_t)
    ENUM_DECL_ENTRY(UNSIGNED_INT_2_10_10_10_REV)
    ENUM_DECL_ENTRY(INT_2_10_10_10_REV)
    ENUM_DECL_ENTRY(UNSIGNED_INT)
    ENUM_DECL_ENTRY(INT)
    ENUM_DECL_ENTRY(UNSIGNED_BYTE)
    ENUM_DECL_ENTRY(BYTE)
    ENUM_DECL_ENTRY(FLOAT)
    ENUM_DECL_ENTRY(HALF_FLOAT)
ENUM_DECL_END()


ENUM_DECL(vertex_attrs::mode_t)
    ENUM_DECL_ENTRY(ATTR_MODE_INT)
    ENUM_DECL_ENTRY(ATTR_MODE_FLOAT)
    ENUM_DECL_ENTRY(ATTR_MODE_FIXED)
    ENUM_DECL_ENTRY(ATTR_MODE_PACKED)
ENUM_DECL_END()

}

enum node_scope_t {
    GLOBAL
};

enum light_t
{
    OMNI, SPOT
};

ENUM_DECL(node_scope_t)
    ENUM_DECL_ENTRY(GLOBAL)
ENUM_DECL_END()

ENUM_DECL(light_t)
    ENUM_DECL_ENTRY(OMNI)
    ENUM_DECL_ENTRY(SPOT)
ENUM_DECL_END()

struct offset_size
{
    size_t offset;
    size_t size;

    REFL_INNER(offset_size)
        REFL_ENTRY(offset)
        REFL_ENTRY(size)
    REFL_END()
};

struct data_buffer
{
    struct vao_buffer
    {
        struct format_offset
        {
            uint32_t format = ~0u;
            uint32_t offset;

            REFL_INNER(format_offset)
                REFL_ENTRY(format)
                REFL_ENTRY(offset)
            REFL_END()
        };

        
        struct vertex_format
        {   
            struct vertex_attribute
            {
                unsigned id;
                unsigned size;
                vertex_attrs::type_t   type;
                vertex_attrs::mode_t   mode;
                unsigned divisor;

                REFL_INNER(vertex_attribute)
                    REFL_ENTRY(id)
                    REFL_ENTRY(size)
                    REFL_AS_TYPE(type, string)
                    REFL_AS_TYPE(mode, string)
                    REFL_ENTRY(divisor)
                REFL_END()
            };

            vector<vertex_attribute> attributes;

            REFL_INNER(vertex_format)
                REFL_ENTRY_NAMED(attributes, Field__VERTEX_ATTRIBUTE)
            REFL_END()
        };


        format_offset vertex_format_offset;
        vertex_format format;

        REFL_INNER(vao_buffer)
            REFL_ENTRY_NAMED(vertex_format_offset, Field__VAO_VERTEX_FORMAT_OFFSET)
            REFL_ENTRY_NAMED(format, Field__VERTEX_FORMAT)
        REFL_END()
    };

    quoted_string      data_buffer_file;
    offset_size        vertex_file_offset_size;
    offset_size        index_file_offset_size;
    vector<vao_buffer> vaos;

    REFL_INNER(data_buffer)
        REFL_ENTRY_NAMED(data_buffer_file, Field__DATA_BUFFER_FILE)
        REFL_ENTRY_NAMED(vertex_file_offset_size, Field__VERTEX_FILE_OFFSET_SIZE)
        REFL_ENTRY_NAMED(index_file_offset_size, Field__INDEX_FILE_OFFSET_SIZE)
        REFL_ENTRY_NAMED_WITH_TAG(vaos, Field__VAO_BUFFER, aurora_vector_field_tag(Field__VAO_NUM_ELEM))
    REFL_END()
};

struct node
{
    struct node_children
    {
        vector<quoted_string> children;

        REFL_INNER(node_children)
            REFL_ENTRY_NAMED_WITH_TAG(children, Field__NODE_CHILD_NAME, aurora_vector_field_tag(Field__NODE_CHILDS_COUNT))
        REFL_END()
    };

    struct mesh_t
    {
        struct mesh_face
        {
            struct mesh_face_offset_count_base_mat
            {
                unsigned id;
                unsigned offset;
                unsigned count;
                unsigned base_vertex;
                quoted_string mat;
                quoted_string shadow_mat;
                unsigned num_vertices;

                REFL_INNER(mesh_face_offset_count_base_mat)
                    REFL_ENTRY(id)
                    REFL_ENTRY(offset)
                    REFL_ENTRY(count)
                    REFL_ENTRY(base_vertex)
                    REFL_ENTRY(mat)
                    REFL_ENTRY(shadow_mat)
                    REFL_ENTRY(num_vertices)
                REFL_END()
            };
            
            mesh_face_offset_count_base_mat params;

            REFL_INNER(mesh_face)
                REFL_ENTRY_NAMED(params, Field__MESH_FACE_OFFSET_COUNT_MTLNAME3)
            REFL_END()
        };

        struct mesh_vao_ref
        {
            unsigned geom_stream_id;
            unsigned vao_id;

            REFL_INNER(mesh_vao_ref)
                REFL_ENTRY(geom_stream_id)
                REFL_ENTRY(vao_id)
            REFL_END()
        };

        struct mesh_bsphere
        {
            geom::point_3f center;
            float radius;

            REFL_INNER(mesh_bsphere)
                REFL_ENTRY(center.x)
                REFL_ENTRY(center.y)
                REFL_ENTRY(center.z)
                REFL_ENTRY(radius)
            REFL_END()
        };

        struct mesh_bbox
        {
            geom::rectangle_3f rect;

            REFL_INNER(mesh_bbox)
                REFL_ENTRY(rect.x.lo_)
                REFL_ENTRY(rect.y.lo_)
                REFL_ENTRY(rect.z.lo_)
                REFL_ENTRY(rect.x.hi_)
                REFL_ENTRY(rect.y.hi_)
                REFL_ENTRY(rect.z.hi_)
            REFL_END()
        };

        vector<mesh_face> face_array;
        mesh_vao_ref vao_ref;
        mesh_bsphere bsphere;
        mesh_bbox    bbox;

        REFL_INNER(mesh_t)
            REFL_ENTRY_NAMED(vao_ref, Field__MESH_VAO_REF)
            //REFL_ENTRY_NAMED(bsphere, Field__MESH_BSPHERE)
            REFL_ENTRY_NAMED(bbox,    Field__MESH_BBOX)
            REFL_ENTRY_NAMED_WITH_TAG(face_array, Field__MESH_FACE_ARRAY2, aurora_vector_field_tag(Field__MESH_NUMFACEARRAY))
        REFL_END()
    };

    struct controllers_t
    {
        struct control_object_param_data
        {
            struct data_buffer
            {
                struct geometry_buffer_stream
                {
                    float lod;
                    offset_size vertex_offset_size;
                    offset_size index_offset_size;

                    REFL_INNER(geometry_buffer_stream)
                        REFL_ENTRY_NAMED(lod, Field__LOD_PIXEL)
                        REFL_ENTRY_NAMED(vertex_offset_size, Field__VERTEX_FILE_OFFSET_SIZE)
                        REFL_ENTRY_NAMED(index_offset_size,  Field__INDEX_FILE_OFFSET_SIZE)
                    REFL_END()
                };

                struct light_buffer_stream
                {
                    offset_size light_offset_size;
                    light_t     type;

                    REFL_INNER(light_buffer_stream)
                        REFL_ENTRY_NAMED(light_offset_size, Field__LIGHTS_FILE_OFFSET_SIZE)
                        REFL_AS_TYPE_NAMED(type, string, Field__LIGHTS_TYPE)
                    REFL_END()
                };

                vector<geometry_buffer_stream> geometry_streams;
                vector<light_buffer_stream>    light_streams;

                REFL_INNER(data_buffer)
                    REFL_ENTRY_NAMED_WITH_TAG(geometry_streams, Field__GEOMETRY_BUFFER_STREAM, aurora_vector_field_tag(Field__GEOMETRY_STREAM_NUM_ELEM))
                    REFL_ENTRY_NAMED_WITH_TAG(light_streams, Field__LIGHT_BUFFER_STREAM, aurora_vector_field_tag(Field__LIGHTS_STREAM_NUM_ELEM))
                REFL_END()
            };
        
            // just wrapper 
            data_buffer buffer;

            REFL_INNER(control_object_param_data)
                REFL_ENTRY_NAMED(buffer, Field__DATA_BUFFER)
            REFL_END()
        };

        template<class T>
        unsigned count_field(optional<T> const& f) const
        {
            return bool(f) ? 1 : 0;
        }

        unsigned num_controllers() const
        {
            return count_field(object_param_controller) +
                   count_field(treat_children) +
                   count_field(draw_mesh) +
                   count_field(draw_lightpoint);
        }

        optional<control_object_param_data> object_param_controller;
        optional<string>                    treat_children;
        optional<string>                    draw_mesh;
        optional<string>                    draw_lightpoint;

        REFL_INNER(controllers_t)
            auto size = lobj.num_controllers();
            proc(size, size, Field__CONTROL_NUMBER);
            REFL_ENTRY_NAMED(object_param_controller, Field__CONTROL_OBJECT_PARAM_DATA)
            REFL_ENTRY_NAMED(treat_children, Field__CONTROL_TREAT_CHILDS)
            REFL_ENTRY_NAMED(draw_mesh, Field__CONTROL_DRAW_MESH)
            REFL_ENTRY_NAMED(draw_lightpoint, Field__CONTROL_DRAW_LIGHTPOINT)
        REFL_END()
    };

    struct lightpoint2
    {
        optional<offset_size> omni_offset_size;
        optional<offset_size> spot_offset_size;

        REFL_INNER(lightpoint2)
            REFL_ENTRY_NAMED(omni_offset_size, Field__OMNI)
            REFL_ENTRY_NAMED(spot_offset_size, Field__SPOT)
        REFL_END()
    };

    quoted_string name;
    optional<string> scope;
    unsigned flags;
    unsigned draw_order = 0;
    node_children children;
    optional<mesh_t> mesh;
    controllers_t controllers;
    optional<lightpoint2> lightpoint2;

    REFL_INNER(node)
        REFL_ENTRY_NAMED(name, Field__NODE_NAME)
        REFL_ENTRY_NAMED(scope,  Field__NODE_SCOPE)
        REFL_ENTRY_NAMED(draw_order, Field__DRAW_ORDER)
        REFL_ENTRY_NAMED(children, Field__NODE_CHILDS)
        REFL_ENTRY_NAMED(controllers, Field__CONTROLLERS)
        REFL_ENTRY_NAMED(mesh, Field__MESH)
        REFL_ENTRY_NAMED(lightpoint2, Field__LIGHTPOINT2)
    REFL_END()
};

enum texture_link {
    __part0,
    __part1,
    __part2
};

ENUM_DECL(texture_link)
    ENUM_DECL_ENTRY(__part0)
    ENUM_DECL_ENTRY(__part1)
    ENUM_DECL_ENTRY(__part2)
ENUM_DECL_END()

struct material_list
{
    struct material
    {
        struct material_group
        {
            material_group(unsigned texture_unit, string texture_file)
                : link(texture_link(texture_unit))
                , texture(texture_file)
            {
            }

            texture_link  link;
            quoted_string texture;

            REFL_INNER(material_group)
                REFL_AS_TYPE_NAMED(link, quoted_string, Field__TEXTURE_LINK)
                REFL_ENTRY_NAMED(texture, Field__TEXTURE)
            REFL_END()
        };

        quoted_string          name;
        quoted_string          link;
        vector<material_group> textures;

        REFL_INNER(material)
            REFL_ENTRY_NAMED(name, Field__MATERIAL_NAME)
            REFL_ENTRY_NAMED(link, Field__MATERIAL_LINK)
            REFL_ENTRY_NAMED(textures, Field__MATERIAL_GROUP)
        REFL_END()
    };

    vector<material> list;

    REFL_INNER(material_list)
        REFL_ENTRY_NAMED(list, Field__MATERIAL)
    REFL_END()
};

struct aurora_format
{
    data_buffer buffer_data;
    vector<node> nodes;
    material_list materials;

    REFL_INNER(aurora_format)
        REFL_ENTRY_NAMED(materials, Field__MATERIAL_LIST)
        REFL_ENTRY_NAMED(buffer_data, Field__DATA_BUFFER)
        REFL_ENTRY_NAMED(nodes, Field__NODE)
    REFL_END()
};


}
}
