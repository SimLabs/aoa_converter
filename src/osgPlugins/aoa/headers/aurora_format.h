#pragma once
#include "reflection/reflection.h"

#define DECLARE_AURORA_FIELD(name) \
    const char* Field__## name = "#" #name;

namespace aurora
{

DECLARE_AURORA_FIELD(NODE)
DECLARE_AURORA_FIELD(NODE_NAME)
DECLARE_AURORA_FIELD(NODE_FLAGS)
DECLARE_AURORA_FIELD(NODE_CHILDS)
DECLARE_AURORA_FIELD(NODE_CHILD_COUNT)
DECLARE_AURORA_FIELD(NODE_CHILD_NAME)

DECLARE_AURORA_FIELD(DRAW_ORDER)

DECLARE_AURORA_FIELD(MESH)
DECLARE_AURORA_FIELD(MESH_NUMFACEARRAY)
DECLARE_AURORA_FIELD(MESH_VAO_REF)
DECLARE_AURORA_FIELD(MESH_BSPHERE)
DECLARE_AURORA_FIELD(MESH_BBOX)
DECLARE_AURORA_FIELD(MESH_FACE_ARRAY2)
DECLARE_AURORA_FIELD(MESH_FACE_OFFSET_COUNT_MTLNAME3)

DECLARE_AURORA_FIELD(DATA_BUFFER)
DECLARE_AURORA_FIELD(DATA_BUFFER_FILE)
DECLARE_AURORA_FIELD(VERTEX_FILE_OFFSET_SIZE)
DECLARE_AURORA_FIELD(INDEX_FILE_OFFSET_SIZE)
DECLARE_AURORA_FIELD(VAO_VERTEX_FORMAT_OFFSET)
DECLARE_AURORA_FIELD(VAO_NUM_ELEM)
DECLARE_AURORA_FIELD(VAO_BUFFER)
DECLARE_AURORA_FIELD(VERTEX_ATTRIBUTE)
DECLARE_AURORA_FIELD(VERTEX_FORMAT)

struct aurora_vector_field_tag
{
    aurora_vector_field_tag(const char* size_field = nullptr)
        : size_field(size_field)
    {}

    const char* size_field;
};

enum type_t {
    GL_UNSIGNED_INT_2_10_10_10_REV,
    GL_INT_2_10_10_10_REV,
    GL_UNSIGNED_INT,
    GL_INT,
    GL_UNSIGNED_BYTE,
    GL_BYTE,
    GL_FLOAT,
    GL_HALF_FLOAT
};

ENUM_DECL(type_t)
    ENUM_DECL_ENTRY(GL_UNSIGNED_INT_2_10_10_10_REV)
    ENUM_DECL_ENTRY(GL_INT_2_10_10_10_REV)
    ENUM_DECL_ENTRY(GL_UNSIGNED_INT)
    ENUM_DECL_ENTRY(GL_INT)
    ENUM_DECL_ENTRY(GL_UNSIGNED_BYTE)
    ENUM_DECL_ENTRY(GL_BYTE)
    ENUM_DECL_ENTRY(GL_FLOAT)
    ENUM_DECL_ENTRY(GL_HALF_FLOAT)
ENUM_DECL_END()

enum mode_t {
    ATTR_MODE_INT,
    ATTR_MODE_FLOAT,
    ATTR_MODE_FIXED,
    ATTR_MODE_PACKED
};

ENUM_DECL(mode_t)
    ENUM_DECL_ENTRY(ATTR_MODE_INT)
    ENUM_DECL_ENTRY(ATTR_MODE_FLOAT)
    ENUM_DECL_ENTRY(ATTR_MODE_FIXED)
    ENUM_DECL_ENTRY(ATTR_MODE_PACKED)
ENUM_DECL_END()

struct data_buffer
{
    struct vao_buffer
    {
        struct format_offset
        {
            size_t format;
            size_t offset;

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
                type_t   type;
                mode_t   mode;
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

    struct offset_size
    {
        size_t offset;
        size_t size;

        REFL_INNER(offset_size)
            REFL_ENTRY(offset)
            REFL_ENTRY(size)
        REFL_END()
    };

    string             data_buffer_file;
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





}
