#include "ConvertTexturesVisitor.h"

#include <osg/Texture3D>
#include <osg/Texture2D>
#include <osgDB/WriteFile>

#include <filesystem>

namespace fs = std::filesystem;

void ConvertTexturesVisitor::apply(osg::Node & node)
{
    if(node.getStateSet()) apply(*node.getStateSet());
    traverse(node);
}

void ConvertTexturesVisitor::apply(osg::StateSet & stateset)
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

void ConvertTexturesVisitor::write(const std::string & dir)
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
            auto image_path = fs::path(image->getFileName());
            if(image_path.is_absolute())
                image_path = image_path.filename();
            std::string name = image_path.replace_extension(extension_).string();
            image->setFileName(name);
            std::string path = (fs::path(dir) / name).string();
            if(fs::path(path).has_parent_path())
                fs::create_directories(fs::path(path).parent_path());
            osgDB::writeImageFile(*image, path);
            osg::notify(osg::NOTICE) << "Image written to '" << path << "'." << std::endl;
        }
    }
}
