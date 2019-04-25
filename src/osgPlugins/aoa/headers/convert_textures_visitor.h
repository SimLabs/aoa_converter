#pragma once

#include <osg/NodeVisitor>
#include <osg/Texture>

namespace aurora
{

struct convert_textures_visitor: osg::NodeVisitor
{
    convert_textures_visitor(string convert_to) 
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , extension_(convert_to)
    {}

    virtual void apply(osg::Node& node) override;

    void write(const std::string &dir);
private:
    void apply(osg::StateSet& stateset);

    string extension_;
    typedef std::set< osg::ref_ptr<osg::Texture> > TextureSet;
    TextureSet                          _textureSet;
};


}
