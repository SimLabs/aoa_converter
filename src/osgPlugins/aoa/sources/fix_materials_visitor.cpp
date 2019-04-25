#include "fix_materials_visitor.h"
#include "material_loader.h"
#include <osg/Geode>
#include <osg/Material>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

namespace aurora
{

struct fix_materials_visitor::impl
{
    impl(material_loader& l, string relative_dir)
        : loader(l)
        , relative_dir(relative_dir)
    {
    }

    material_loader& loader;
    std::string      relative_dir;
};

fix_materials_visitor::fix_materials_visitor(material_loader& l, string relative_dir)
    : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    , pimpl_(std::make_unique<fix_materials_visitor::impl>(l, relative_dir))
{

}

fix_materials_visitor::~fix_materials_visitor() = default;

void fix_materials_visitor::apply(osg::Geometry & g)
{
    auto pGeometry = &g;
    if(pGeometry && pGeometry->getStateSet() && pGeometry->getStateSet()->getAttribute(osg::StateAttribute::MATERIAL))
    {
        osg::StateSet* state_set = pGeometry->getStateSet();
        osg::Material* mat = (osg::Material*) pGeometry->getStateSet()->getAttribute(osg::StateAttribute::MATERIAL);
        unsigned num_textures = state_set->getNumTextureAttributeLists();

        auto mat_data = pimpl_->loader.get_material_data(mat->getName());
        if(!mat_data)
        {
            OSG_WARN << "material \"" << mat->getName() << "\" not found, skipping\n";
            return;
        }
        else if(mat_data->explicit_material.empty())
        {
            osg::ref_ptr<osg::Image> tex_image = osgDB::readRefImageFile((fs::path(pimpl_->relative_dir) / mat_data->albedo_tex).string());
            if(tex_image)
            {
                tex_image->setFileName(mat_data->albedo_tex);
                osg::ref_ptr<osg::Texture> texture = new osg::Texture2D(tex_image);
                state_set->setTextureAttribute(num_textures++, texture, osg::StateAttribute::ON);
            }
        }
    }
    traverse(g);
}

} // aurora