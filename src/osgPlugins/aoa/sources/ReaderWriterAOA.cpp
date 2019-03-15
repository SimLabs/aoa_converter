
#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif



#include <stdlib.h>
#include <string>

#include <osg/Notify>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Vec3f>

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexMat>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

//#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tessellator>
#include <osgUtil/Optimizer>

//#include "common_includes.h"
//#include "writer_visitor.h"
//#include "atlas_building/atlas_builder.h"
#include <map>
#include <set>

//#include "transformations.h"
//#include "optimization/inactive_removing.h"
//#include "optimization/propagate_state.h"
//#include "optimization/optimization.h"
//#include "temp/node_types.h"
//#include "post_optimization/markings.h"
//#include "optimization/preprocessing.h"

//using namespace scg;

class ReaderWriterAOA : public osgDB::ReaderWriter
{
public:
    ReaderWriterAOA()
    {
        supportsExtension("aoa","Aurora engine format");
        //supportsOption("scgConfig=<dir>","Path to config file");
        //supportsOption("noTesselateLargePolygons","Do not do the default tesselation of large polygons");
        //supportsOption("noTriStripPolygons","Do not do the default tri stripping of polygons");
        //supportsOption("generateFacetNormals","generate facet normals for verticies without normals");
        //supportsOption("noReverseFaces","avoid to reverse faces when normals and triangles orientation are reversed");

        //supportsOption("DIFFUSE=<unit>", "Set texture unit for diffuse texture");
        //supportsOption("AMBIENT=<unit>", "Set texture unit for ambient texture");
        //supportsOption("SPECULAR=<unit>", "Set texture unit for specular texture");
        //supportsOption("SPECULAR_EXPONENT=<unit>", "Set texture unit for specular exponent texture");
        //supportsOption("OPACITY=<unit>", "Set texture unit for opacity/dissolve texture");
        //supportsOption("BUMP=<unit>", "Set texture unit for bumpmap texture");
        //supportsOption("DISPLACEMENT=<unit>", "Set texture unit for displacement texture");
        //supportsOption("REFLECTION=<unit>", "Set texture unit for reflection texture");
    }

    const char* className() const override { return "Aurora engine AOA Writer"; }

    ReadResult readNode(const std::string& /*file_name*/, const Options* /*options*/) const override
    {
        return ReadResult();
    }

    ReadResult readNode(std::istream& /*fin*/, const Options* /*options*/) const override
    {
        return ReadResult();
    }

    WriteResult writeObject(const osg::Object& obj,const std::string& file_name, const Options* options = nullptr) const override
    {
        OSG_INFO << "Writing object to AOA file " << file_name << std::endl;
        const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
        if (node)
            return writeNode(*node, file_name, options);
        else
            return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    WriteResult writeNode(const osg::Node& node, const std::string& file_name, const Options* options = nullptr) const override
    {
//        DebugBreak();
        if (!acceptsExtension(osgDB::getFileExtension(file_name)))
            return WriteResult(WriteResult::FILE_NOT_HANDLED);

        OSG_INFO << "Writing node to AOA file " << file_name << std::endl;

        //config_t config = make_config(file_name, node, config_path(options));

        //print_node_types(const_cast<osg::Node&>(node));

        //osg::ref_ptr<osg::Node> root = perform_preprocessing(config, const_cast<osg::Node&>(node));

        //twin_holder_ptr twins = make_shared<twin_holder>(config);
        //apply_atlas_builder(*root, twins);

        //auto resource = make_shared<resource_manager>(config, twins);

        //osgDB::ofstream fout(file_name.c_str());
        //writer_visitor nv(fout, resource);

        //root->accept(nv);

        //resource->write_data();

        return WriteResult(WriteResult::FILE_SAVED);
    }
    
    WriteResult writeObject(const osg::Object& /*obj*/,std::ostream& /*fout*/, const Options* /*options*/ = nullptr) const override
    {
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    WriteResult writeNode(const osg::Node& /*node*/, std::ostream& /*fout*/, const Options* = nullptr) const override
    {
        return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }
    

private:
    //optional<string> config_path(Options const *options = nullptr) const
    //{
    //    if (!options)
    //        return none;

    //    if (options->getPluginStringData("scgConfig") != "")
    //    {
    //        return options->getPluginStringData("scgConfig");
    //    }

    //    return none;
    //}
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(aoa, ReaderWriterAOA)