
#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <boost/tokenizer.hpp>

#include <osg/Notify>
#include <osg/NodeVisitor>
#include <osg/MatrixTransform>

#include <osgDB/Registry>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgUtil/Optimizer>

#include "write_aoa_visitor.h"
#include "aurora_aoa_writer.h"
#include "convert_textures_visitor.h"
#include "fix_materials_visitor.h"
#include "lights_generation_visitor.h"
#include "material_loader.h"
#include "debug_utils.h"
#include "plugin_config.h"
#include "aurora_aoa_reader.h"

using namespace aurora;

struct make_transforms_static_visitor: osg::NodeVisitor
{
    make_transforms_static_visitor()
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {}


    void apply(osg::Node& node) override
    {
        traverse(node);
    }

    void apply(osg::Transform& t) override
    {
        t.setDataVariance(osg::Object::DataVariance::STATIC);
        traverse(t);
    }
};

struct remove_hanging_transforms_visitor : osg::NodeVisitor
{
    remove_hanging_transforms_visitor()
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {}

    void apply(osg::Group& n) override
    {
        if(n.getNumChildren() == 0)
            nodes_to_remove_.insert(&n);
        else 
            traverse(n);
    }

    bool remove_hanging_transforms()
    {
        for(auto& t: nodes_to_remove_)
        {
            assert(t->getNumParents() == 1);
            t->getParent(0)->removeChild(t.get());
        }

        bool sts = nodes_to_remove_.size() != 0;
        nodes_to_remove_.clear();
        return sts;
    }

private:
    std::set<osg::ref_ptr<osg::Node>> nodes_to_remove_;
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

    const char* className() const override { return "Aurora engine AOA Writer"; }

    ReadResult readNode(const std::string& file_name, const Options* /*options*/) const override
    {
        auto aoa = aurora::read_aoa(file_name);
        return ReadResult(ReadResult::FILE_LOADED);
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
        if (!acceptsExtension(osgDB::getFileExtension(file_name)))
            return WriteResult(WriteResult::FILE_NOT_HANDLED);

        try 
        {
            std::vector<std::string> split_opts;
            vector<char*> argv;
            // dummy value, cause first argument must be present
            argv.push_back("aoa-plugin");
            int argc = argv.size();

            if(options)
            {
                using boost::tokenizer;
                using boost::escaped_list_separator;
                using so_tokenizer = tokenizer<escaped_list_separator<char>>;

                so_tokenizer tok(options->getOptionString(), escaped_list_separator<char>('\\', ' ', '\"'));
                for(so_tokenizer::iterator beg = tok.begin(); beg != tok.end(); ++beg)
                {
                    split_opts.push_back(*beg);
                }
                std::transform(begin(split_opts), end(split_opts), back_inserter(argv), [](auto& s){ return const_cast<char*>(s.data()); });
                argc = argv.size();
            }

            osg::ArgumentParser arguments(&argc, argv.data());


            string materials_file;
            arguments.read("--aoa-materials-file", materials_file);
            if(materials_file.empty())
                OSG_WARN << "AOA plugin: materials file is not specified\n";
    
            material_loader mat_loader(materials_file);

            // initialize config file
            string config_path;
            arguments.read("--aoa-config", config_path);
            if(config_path.empty())
                config_path = "aoa.config.json";
            auto config = get_config(config_path);

            //////////////////////////////////////////////
            // add transform
            osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform(config.get_full_transform());
            transform->addChild(const_cast<osg::Node*>(&node));
            osg::Node& osg_root = *transform;
            OSG_INFO << "flip Y and Z: " << config.flip_YZ << "\n";
            //////////////////////////////////////////////

            fix_materials_visitor fix_mats_vis(mat_loader, fs::path(materials_file).parent_path().string());
            osg_root.accept(fix_mats_vis);

            // convert all textures to some format
            aurora::convert_textures_visitor texture_visitor("dds");
            osg_root.accept(texture_visitor);
            texture_visitor.write(osgDB::getFilePath(file_name));

            aurora::aoa_writer file_writer(file_name);
            lights_generation_visitor generate_lights_v(file_writer);
            osg_root.accept(generate_lights_v);
            generate_lights_v.generate_lights();

            // apply transforms from ancestor nodes to geometry
            // optimizer will only do this for transform nodes whose data variance is STATIC so we set it here 
            make_transforms_static_visitor make_tranforms_static;
            osg_root.accept(make_tranforms_static);

            // remove leaf Transform nodes because otherwise the optimizer will not be able to flatten all transforms properly
            remove_hanging_transforms_visitor remove_hanging_tv;
            
            do osg_root.accept(remove_hanging_tv);
            while(remove_hanging_tv.remove_hanging_transforms());

            // run the optimizer
            osgUtil::Optimizer optimizer;
            optimizer.optimize(&osg_root, osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS | (config.index_mesh ? osgUtil::Optimizer::INDEX_MESH : 0));

            OSG_INFO << "Writing node to AOA file " << file_name << std::endl;

            aurora::write_aoa_visitor write_aoa_v(mat_loader, file_writer);
            osg_root.accept(write_aoa_v);
            write_aoa_v.write_aoa();


            // ======================= DEBUG OUTPUT ============================
            //write_aoa_v.write_debug_obj_file(fs::path(file_name).replace_extension("obj").string());
            //debug_utils::write_node(osg_root, fs::path(file_name).replace_extension("after.stripped.osg").string(), true);
            // ==================================================================
        }
        catch(std::exception const& e)
        {
            return WriteResult(string("AOA plugin: ") + e.what());
        }
        catch(const char* e)
        {
            return WriteResult(string("AOA plugin: ") + e);
        }
        catch(...)
        {
            return WriteResult("AOA plugin: caught unknown exception");
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
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(aoa, ReaderWriterAOA)