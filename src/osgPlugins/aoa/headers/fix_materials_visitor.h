#pragma once
#include <osg/NodeVisitor>

namespace aurora
{

struct material_loader;

struct fix_materials_visitor: osg::NodeVisitor
{
    fix_materials_visitor(material_loader& l, string relative_dir);
    ~fix_materials_visitor();

    void apply(osg::Geometry& g) override;

private:
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

}
