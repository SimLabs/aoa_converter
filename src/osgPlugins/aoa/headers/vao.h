#pragma once

#include <osg/Image>

struct vertex_info
{
    geom::point_3f pos;
    geom::point_3f norm;
    geom::point_2f uv;
    //geom::point_3f tangent;

    vertex_info() {};

    //    REFL_INNER(vertex_info)
    //        REFL_ENTRY(pos)
    //        REFL_ENTRY(uv)
    //        REFL_ENTRY(norm)
    //        REFL_ENTRY(tangent)
    //    REFL_END()
};

using image_ref_cptr = osg::ref_ptr<const osg::Image>;
using image_ptr = osg::ref_ptr<osg::Image>;

struct material_data
{
    string         material;
    image_ref_cptr texture;
};

struct material_modifier
{
    string                      material;
    map<size_t, image_ref_cptr> textures;
};

inline size_t hash_value(material_modifier const &modifier)
{
    using boost::hash_value;

    size_t res = hash_value(modifier.material);
    for(auto const &item : modifier.textures)
    {
        boost::hash_combine(res, hash_value(item.first));
        boost::hash_combine(res, hash_value(item.second->getFileName()));
    }

    return res;
}

inline bool operator==(material_modifier const &lhs, material_modifier const &rhs)
{
    return lhs.material == rhs.material
        && lhs.textures == rhs.textures;
}

struct material_info
{
    // ordered list of texture unit (index in vector) -> filename
    vector<string> textures;
    string explicit_material;

    material_info()
    {}
};

struct chunk_info_opt_material
{
    material_info      material;

    string             name;
    geom::range_2ui    vertex_range;
    geom::range_2ui    faces_range;
    geom::rectangle_3f aabb;
};

struct face
{
    unsigned v[3];
};
