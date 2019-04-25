#pragma once

#include <osg/NodeVisitor>
#include <osgDB/WriteFile>

namespace debug_utils
{
    struct strip_arrays_visitor: osg::NodeVisitor
    {
        strip_arrays_visitor(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {}

        void apply(osg::Node& n) override
        {
            traverse(n);
        }

        void apply(osg::Geometry& n) override
        {
            n.setVertexArray(nullptr);
            n.setTexCoordArray(0, nullptr);
            n.setNormalArray(nullptr);
            n.setColorArray(nullptr);
        }
    };

    inline void write_node(osg::Node& node, string filename, bool strip_arrays = false)
    {
        if(strip_arrays)
        {
            debug_utils::strip_arrays_visitor strip_arrays_v;
            node.accept(strip_arrays_v);
        }

        osgDB::writeNodeFile(node, filename);
    }
}