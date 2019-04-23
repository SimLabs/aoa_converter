#pragma once

#include <osg/NodeVisitor>
#include <osg/Texture>
#include <osg/Texture3D>
#include <osgDB/WriteFile>

namespace aurora
{

struct convert_textures_visitor: osg::NodeVisitor
{
    convert_textures_visitor() :
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {}

    virtual void apply(osg::Node& node) override
    {
        if(node.getStateSet()) apply(*node.getStateSet());
        traverse(node);
    }

    void apply(osg::StateSet& stateset)
    {
        // search for the existence of any texture object attributes
        for(unsigned int i = 0; i < stateset.getTextureAttributeList().size(); ++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset.getTextureAttribute(i, osg::StateAttribute::TEXTURE));
            if(texture)
            {
                _textureSet.insert(texture);
            }
        }
    }

    void write(const std::string &dir)
    {
        for(TextureSet::iterator itr = _textureSet.begin();
            itr != _textureSet.end();
            ++itr)
        {
            osg::Texture* texture = const_cast<osg::Texture*>(itr->get());

            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture);
            osg::Texture3D* texture3D = dynamic_cast<osg::Texture3D*>(texture);

            osg::ref_ptr<osg::Image> image = texture2D ? texture2D->getImage() : (texture3D ? texture3D->getImage() : 0);
            if(image.valid())
            {
                std::string name = fs::path(image->getFileName()).replace_extension("dds").string();
                image->setFileName(name);
                std::string path = dir.empty() ? name : osgDB::concatPaths(dir, name);
                if(fs::path(path).has_parent_path())
                    fs::create_directories(fs::path(path).parent_path());
                osgDB::writeImageFile(*image, path);
                osg::notify(osg::NOTICE) << "Image written to '" << path << "'." << std::endl;
            }
        }
    }

    typedef std::set< osg::ref_ptr<osg::Texture> > TextureSet;
    TextureSet                          _textureSet;
};


}
