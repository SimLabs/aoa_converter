#pragma once

#include "osg/Geometry"
#include "osg/Geode"
#include "osg/NodeVisitor"

//#include "texture_twin.h"
//#include "data.h"
//#include "material_manager.h"

#include "geom.h"
#include "vao.h"

namespace aoa
{

struct geometry_visitor : osg::NodeVisitor
{
    geometry_visitor();

    void apply(osg::Geode    &geode)    override;
    void apply(osg::Geometry &geometry) override;

    vector<size_t> const &get_chunks(osg::Geode const &geode) const;

    chunk_info_opt_material const &get_chunk(size_t index) const;

    vector<chunk_info_opt_material> const &get_chunks   ();
    vector<vertex_info>             const &get_verticies();
    vector<face>                    const &get_faces    ();

private:
    void finalize();

    geom::range_2ui collect_verticies(osg::Geometry const &geometry);
    geom::range_2ui collect_faces    (geom::range_2ui vertex_range, osg::Geometry     const &geometry);
    geom::range_2ui collect_faces    (geom::range_2ui vertex_range, osg::PrimitiveSet const &primitive_set);

    void fill_aabb(chunk_info_opt_material &chunk) const;

private:
    // geode -> indicies of chunks with optional material
    map<osg::Geode const*, vector<size_t>> geode2chunks_;
    osg::Geode const*                      current_geode_;
    
    vector<vertex_info>             verticies_;
    vector<face>                    faces_;
    vector<chunk_info_opt_material> chunks_;

    bool finalized_;

    //material_manager_ptr material_manager_;
};

using geometry_visitor_ptr = shared_ptr<geometry_visitor>;

} // namespace scg