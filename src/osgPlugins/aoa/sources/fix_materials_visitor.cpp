#include "fix_materials_visitor.h"
#include <osg/Geode>
#include <osg/Material>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

namespace aurora
{

namespace
{

struct material_data
{
    string albedo_tex;
};

std::istream& split_next_line(std::istream& str, std::vector<std::string>& result)
{
    result.clear();
    std::string                line;
    std::getline(str, line);

    std::stringstream          line_stream(line);
    std::string                cell;

    while(std::getline(line_stream, cell, ','))
    {
        result.push_back(cell);
    }
    // This checks for a trailing comma with no data after it.
    if(!line_stream && cell.empty())
    {
        // If there was a trailing comma then add an empty element.
        result.push_back("");
    }
    return str;
}

struct material_loader
{
    material_loader(string filename)
    {
        std::ifstream input(filename);

        std::string line;
        // throw away header
        std::getline(input, line);
        
        vector<string> fields;
        while(split_next_line(input, fields))
        {
            assert(fields.size() == 2);
            material_data mat_data;
            mat_data.albedo_tex = fs::path(fields[1]).lexically_normal().string();
            materials_.emplace(fields[0], mat_data);
        }
    }

    optional<material_data> get_material_data(string mat_name)
    {
        auto it = materials_.find(mat_name);
        if(it != materials_.end())
            return it->second;
        else 
            return boost::none;
    }

private:
    std::map<std::string, material_data> materials_;
};

}

struct fix_materials_visitor::impl
{
    impl(string materials_file, string relative_dir)
        : loader(materials_file)
        , relative_dir(relative_dir)
    {
    }

    material_loader loader;
    std::string     relative_dir;
};

fix_materials_visitor::fix_materials_visitor(string materials_file, string relative_dir)
    : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    , pimpl_(std::make_unique<fix_materials_visitor::impl>(materials_file, relative_dir))
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

        osg::ref_ptr<osg::Image> tex_image = osgDB::readRefImageFile((fs::path(pimpl_->relative_dir) / mat_data->albedo_tex).string());
        if(tex_image)
        {
            tex_image->setFileName(mat_data->albedo_tex);
            osg::ref_ptr<osg::Texture> texture = new osg::Texture2D(tex_image);
            state_set->setTextureAttribute(num_textures++, texture, osg::StateAttribute::ON);
        }

    }
    traverse(g);
}

} // aurora