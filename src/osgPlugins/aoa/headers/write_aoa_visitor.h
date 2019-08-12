#pragma once

#include "osg/Geometry"
#include "osg/Geode"
#include "osg/NodeVisitor"

#include "vao.h"
#include "aurora_aoa_writer.h"
#include "plugin_config.h"

namespace aurora
{

struct material_loader;

struct write_aoa_visitor : osg::NodeVisitor
{
    write_aoa_visitor(material_loader&, aoa_writer&);

    void apply(osg::Group & group) override;
    void apply(osg::Geode    &geode)    override;
    void apply(osg::Geometry &geometry) override;
    void apply(osg::LightSource& light_source) override;
    void apply(osg::LOD& lod) override;

    void write_aoa();
    void write_debug_obj_file(string file_name) const;

private:
    aoa_writer::node_ptr current_node() { return aoa_nodes_stack_.top(); }
    auto create_node_scope(osg::Node& n);
    string get_unique_node_name(string desired);
    vector<size_t> const &get_chunks(osg::Geode const &geode) const;

    chunk_info_opt_material const &get_chunk(size_t index) const;

    vector<chunk_info_opt_material> const &get_chunks   () const;
    vector<vertex_info>             const &get_verticies() const;
    vector<face>                    const &get_faces    () const;

    geom::range_2ui collect_verticies(osg::Geometry const &geometry);
    geom::range_2ui collect_faces    (geom::range_2ui vertex_range, osg::Geometry     const &geometry);
    geom::range_2ui collect_faces    (geom::range_2ui vertex_range, osg::PrimitiveSet const &primitive_set);
    void extract_texture_info(osg::Drawable & node, chunk_info_opt_material & chunk);

    void fill_aabb(chunk_info_opt_material &chunk) const;

private:

    material_loader&                       material_loader_;
    aoa_writer&                            aoa_writer_;
    // geode -> indicies of chunks with optional material
    map<osg::Geode const*, vector<size_t>> geode2chunks_;
    osg::Geode const*                      current_geode_;
    
    vector<vertex_info>             verticies_;
    vector<face>                    faces_;
    vector<chunk_info_opt_material> chunks_;

    std::stack<aoa_writer::node_ptr> aoa_nodes_stack_;
    bool                             root_visited_ = false;
    std::set<string>                 node_names_;
};

using write_aoa_visitor_ptr = shared_ptr<write_aoa_visitor>;

}