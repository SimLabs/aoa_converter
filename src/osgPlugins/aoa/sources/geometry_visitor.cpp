#include "geometry_visitor.h"
#include "geometry_visitor.h"
//#include "utils.h"
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/Material>

namespace aurora
{

geometry_visitor::geometry_visitor(material_loader& l)
    : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
    , material_loader_(l)
    , current_geode_(nullptr)
    , finalized_    (false)
    //, material_manager_(material_manager)
{
}

void geometry_visitor::apply(osg::Geode &geode)
{
    if (geode2chunks_.find(&geode) != geode2chunks_.end())
        return;
    current_geode_ = &geode;
    traverse(geode);
}

void geometry_visitor::apply(osg::Geometry &geometry)
{
    static unsigned count = 0;
    chunk_info_opt_material chunk;
    
    extract_texture_info(geometry, chunk);

    chunk.name = geometry.getName().empty() ? "object_" + std::to_string(++count) : geometry.getName();
    chunk.vertex_range = collect_verticies(geometry);
    if (chunk.vertex_range.hi() == chunk.vertex_range.lo())
    {
        OSG_WARN << "Empty geometry" << std::endl;
    }
    fill_aabb(chunk);

    //chunk.material.material = material_manager_->extract_material(geometry);
    //if (geometry.getTexCoordArray(0) != nullptr)
    //{
    //    chunk.material.texture_index = 0;
    //    chunk.material.texture = extract_texture(geometry, 0);
    //}

    chunk.faces_range  = collect_faces(chunk.vertex_range, geometry);

    geode2chunks_[current_geode_].push_back(chunks_.size());
    chunks_.push_back(chunk);
}

vector<size_t> const & geometry_visitor::get_chunks(osg::Geode const &geode) const
{
    return geode2chunks_.at(&geode);
}

chunk_info_opt_material const & geometry_visitor::get_chunk(size_t index) const
{
    return chunks_[index];
}

vector<chunk_info_opt_material> const & geometry_visitor::get_chunks() const
{
    return chunks_;
}

vector<vertex_info> const & geometry_visitor::get_verticies() const
{
    finalize();
    return verticies_;
}

vector<face> const & geometry_visitor::get_faces() const
{
    finalize();
    return faces_;
}

void geometry_visitor::finalize() const
{
    if (!finalized_)
    {
        //count_tangents_lengyel(verticies_, faces_);
        finalized_ = true;
    }
}

geom::range_2ui geometry_visitor::collect_verticies(osg::Geometry const &geometry)
{
    osg::Vec3Array const *vertex_array = dynamic_cast<osg::Vec3Array const*>(geometry.getVertexArray());
    osg::Vec2Array const *uv_array     = dynamic_cast<osg::Vec2Array const*>(geometry.getTexCoordArray(0));
    osg::Vec3Array const *norm_array   = dynamic_cast<osg::Vec3Array const*>(geometry.getNormalArray());

    if (!vertex_array)
        return geom::range_2ui(0, 0);

    Assert(vertex_array);
    Assert(!uv_array   || uv_array->size()   == vertex_array->size());
    Assert(!norm_array || norm_array->size() == vertex_array->size());

    size_t vertex_start = verticies_.size();
    for(size_t i = 0; i < vertex_array->size(); ++i)
    {
        vertex_info vertex;
        vertex.pos = geom::point_3f((*vertex_array)[i][0], (*vertex_array)[i][1], (*vertex_array)[i][2]);
        if(uv_array)
            vertex.uv = geom::point_2f((*uv_array)[i][0], (*uv_array)[i][1]);
        if(norm_array)
            vertex.norm = geom::point_3f((*norm_array)[i][0], (*norm_array)[i][1], (*norm_array)[i][2]);

        verticies_.push_back(vertex);
    }

    return {unsigned(vertex_start), unsigned(vertex_start + vertex_array->size())};
}

geom::range_2ui geometry_visitor::collect_faces(geom::range_2ui vertex_range, osg::Geometry const &geometry)
{
    geom::range_2ui faces_range = {(uint32_t)faces_.size(), (uint32_t)faces_.size()};
    for (unsigned i = 0; i < geometry.getNumPrimitiveSets(); ++i)
    {
        geom::range_2ui sub_range = collect_faces(vertex_range, *geometry.getPrimitiveSet(i));
        faces_range.hi_ = sub_range.hi();
    }
    return faces_range;
}

geom::range_2ui geometry_visitor::collect_faces(geom::range_2ui vertex_range, osg::PrimitiveSet const &primitive_set)
{
    geom::range_2ui faces_range = {(uint32_t)faces_.size(), (uint32_t)faces_.size()};
    if (primitive_set.getMode() != osg::PrimitiveSet::TRIANGLES)
        return faces_range;

    unsigned elements_count = primitive_set.getNumIndices();
    Assert(elements_count % 3 == 0);

    for (unsigned i = 0; i < elements_count; i += 3)
    {
        face face;
        for (unsigned j = 0; j < 3; ++j)
        {
            face.v[j] = primitive_set.index(i + j);
        }
        faces_.push_back(face);
    }

    faces_range.hi_ += elements_count / 3;
    return faces_range;
}

void geometry_visitor::extract_texture_info(osg::Drawable& node, chunk_info_opt_material& chunk)
{
    osg::Drawable* drawable = &node;
    if(drawable && drawable->getStateSet())
    {
        osg::StateSet* state_set = drawable->getStateSet();

        osg::Material* mat = (osg::Material*) state_set->getAttribute(osg::StateAttribute::MATERIAL);

        if(!mat)
        {
            OSG_WARN << "AOA plugin: no material in node \"" << node.getName() << "\" skipping \n";
            return;
        }

        auto mat_data = material_loader_.get_material_data(mat->getName());

        if(!mat_data)
        {
            OSG_WARN << "AOA plugin: material \"" << mat->getName() << "\" not found, skipping\n";
            return;
        }

        if(mat_data->explicit_material.empty())
        {
            for(unsigned int i = 0; i < state_set->getTextureAttributeList().size(); ++i)
            {
                osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(state_set->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
                if(texture && texture->getImage())
                {
                    chunk.material.textures.push_back(texture->getImage()->getFileName());
                }
            }            
        }
        else
        {
            chunk.material.explicit_material = mat_data->explicit_material;
        }
    }

    //traverse(node);
}

void geometry_visitor::fill_aabb(chunk_info_opt_material &chunk) const
{
    float min_x, min_y, min_z;
    min_x = min_y = min_z = std::numeric_limits<float>::max();

    float max_x, max_y, max_z;
    max_x = max_y = max_z = std::numeric_limits<float>::min();

    auto update_min_max = [](float &min, float &max, float value)
    {
        min = std::min(min, value);
        max = std::max(max, value);
    };

    for (size_t i = chunk.vertex_range.lo(); i < chunk.vertex_range.hi(); ++i)
    {
        update_min_max(min_x, max_x, verticies_[i].pos.x);
        update_min_max(min_y, max_y, verticies_[i].pos.y);
        update_min_max(min_z, max_z, verticies_[i].pos.z);
    }

    chunk.aabb = geom::rectangle_3f(geom::point_3f(min_x, min_y, min_z), geom::point_3f(max_x, max_y, max_z));
}

}
