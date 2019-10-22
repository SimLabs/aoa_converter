#pragma once

#include <osg/NodeVisitor>

namespace aurora
{

struct tesselate_visitor: osg::NodeVisitor
{
    tesselate_visitor(float size_threshold)
        : NodeVisitor(TRAVERSE_ALL_CHILDREN)
        , size_threshold(size_threshold)
    {}

    void apply(osg::Geometry& geometry) override;
private:
    float size_threshold;
    void transform_if_needed(osg::Geometry &geometry);
};


}
