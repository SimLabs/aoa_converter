#pragma once

#include <osg/NodeVisitor>
#include <osg/Texture>

namespace aurora
{

struct convert_textures_visitor: osg::NodeVisitor
{
    convert_textures_visitor() :
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {}

    virtual void apply(osg::Node& node) override;

    void write(const std::string &dir);
private:
    void apply(osg::StateSet& stateset);

    typedef std::set< osg::ref_ptr<osg::Texture> > TextureSet;
    TextureSet                          _textureSet;
};


}
