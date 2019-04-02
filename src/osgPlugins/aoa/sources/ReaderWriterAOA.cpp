
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

#include "geometry_visitor.h"
#include "aurora_aoa_writer.h"
#include "convert_textures_visitor.h"


class MakeTransformsStaticVisitor: public osg::NodeVisitor
{
    using osg::NodeVisitor::NodeVisitor;

    void apply(osg::Transform& t) override
    {
        t.setDataVariance(osg::Object::DataVariance::STATIC);
        traverse(t);
    }
};

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

    void write_debug_obj_file(aurora::geometry_visitor& geom_visitor, string file_name) const
    {
        std::ofstream obj_file(fs::path(file_name).replace_extension("obj").string());

        for(auto const& v : geom_visitor.get_verticies())
        {
            obj_file << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z << std::endl;
        }

        for(auto const& v : geom_visitor.get_verticies())
        {
            obj_file << "vn " << v.norm.x << " " << v.norm.y << " " << v.norm.z << std::endl;
        }


        for(auto const& face : geom_visitor.get_faces())
        {
            unsigned v[3] = { face.v[0] + 1, face.v[1] + 1, face.v[2] + 1 };
            obj_file << "f ";
            for(unsigned i = 0; i < 3; ++i)
            {
                string foo = std::to_string(v[i]);
                foo = foo + "//" + foo + " ";
                obj_file << foo;
            }
            obj_file << std::endl;
        }
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

        try 
        {
            // convert all textures to bmp
            aurora::convert_textures_visitor texture_visitor;
            const_cast<osg::Node&>(node).accept(texture_visitor);
            texture_visitor.write(osgDB::getFilePath(file_name));

            // apply transforms from ancestor nodes to geometry
            // optimizer will only do this for transform nodes whose data variance is STATIC so we set it here 
            MakeTransformsStaticVisitor make_tranforms_static(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
            const_cast<osg::Node&>(node).accept(make_tranforms_static);

            // run the optimizer
            osgUtil::Optimizer optimizer;
            optimizer.optimize(&const_cast<osg::Node&>(node), osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS);


            aurora::geometry_visitor geom_visitor;

            const_cast<osg::Node&>(node).accept(geom_visitor);

            OSG_INFO << "EXTRACTED " << geom_visitor.get_chunks().size() << " CHUNKS" << std::endl;
            OSG_INFO << "EXTRACTED " << geom_visitor.get_faces().size() << " FACES" << std::endl;
            OSG_INFO << "EXTRACTED " << geom_visitor.get_verticies().size() << " VIRTICES" << std::endl;
            OSG_INFO << "Writing node to AOA file " << file_name << std::endl;

            aurora::aoa_writer file_writer(fs::path(file_name).replace_extension("aod").string());
            file_writer.save_data(geom_visitor);

            // ======================= DEBUG OUTPUT ============================
            //write_debug_obj_file(geom_visitor, file_name);
            //osgDB::writeNodeFile(node, fs::path(file_name).replace_extension("osg").string());
            //osgDB::writeNodeFile(node, fs::path(file_name).replace_extension("fbx").string());
            // ==================================================================
        }
        catch(std::exception const& e)
        {
            OSG_FATAL << e.what();
            return WriteResult(WriteResult::ERROR_IN_WRITING_FILE);
        }
        catch(const char* e)
        {
            OSG_FATAL << e;
            return WriteResult(WriteResult::ERROR_IN_WRITING_FILE);
        }
        catch(...)
        {
            OSG_FATAL << "caught unknown exception";
            return WriteResult(WriteResult::ERROR_IN_WRITING_FILE);
        }

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