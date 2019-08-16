#pragma once

#include <osg/NodeVisitor>
#include <osg/Texture>

struct ConvertTexturesVisitor : osg::NodeVisitor
{
    ConvertTexturesVisitor(std::string convert_to)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , extension_(convert_to)
    {}

    virtual void apply(osg::Node& node) override;

    void write(const std::string &dir);
private:
    void apply(osg::StateSet& stateset);
    void convertImage(osg::Image& image, std::string const& dir);

    std::string extension_;
    typedef std::set< osg::ref_ptr<osg::Texture> > TextureSet;
    TextureSet                          _textureSet;
};

