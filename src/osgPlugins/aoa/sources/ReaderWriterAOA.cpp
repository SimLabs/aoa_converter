
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
#include "tesselate_visitor.h"
#include "fix_materials_visitor.h"
#include "lights_generation_visitor.h"
#include "material_loader.h"
#include "debug_utils.h"
#include "plugin_config.h"
#include "aoa_to_osg.h"

#include <filesystem>
#include <thread>
#include "osgUtil/Tessellator"
#include "aurora_aoa_reader.h"
#include "aurora_write_processor.h"
#include "aurora_mesh_subdivider.h"
#include "osg/ShapeDrawable"

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

        supportsOption("USE_DUMMY", "Forces to process aoa file without conversion to osg format");
        supportsOption("TESSELATE=<maxsize>", "Tesselation parameter");
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

    ReadResult readNode(const std::string& file_name, const Options* options) const override
    {
        if(!std::filesystem::exists(file_name))
            return ReadResult(ReadResult::FILE_NOT_FOUND);
        else if (options && options->getOptionString().find("USE_DUMMY") != std::string::npos)
        {
            aoa_path = file_name;

            auto dummy_res = new osg::Node;// new osg::Geode;
            //dummy_res->addDrawable(new osg::ShapeDrawable(new osg::Sphere()));
            return dummy_res;
        }
        else
        {
            auto osg_root = aoa_to_osg(file_name);
            if(!osg_root) return osg_root;

            if (options) {
                auto optionsString = options->getOptionString();
                OSG_INFO << "Options: " << optionsString << std::endl;

                std::string tesselateOption = "TESSELATE";
                if (auto tesselatePos = optionsString.find(tesselateOption); tesselatePos != std::string::npos) {
                    std::istringstream is(optionsString.substr(tesselatePos + tesselateOption.size() + 1));

                    float tesselateParam;
                    is >> tesselateParam;

                    OSG_INFO << "Splitting to size " << tesselateParam << std::endl;
                    tesselate_visitor tesselation_visitor(tesselateParam);
                    osg_root->accept(tesselation_visitor);
                }
            }

            // convert all textures to some format
            //aurora::convert_textures_visitor texture_visitor("dds");
            //osg_root->accept(texture_visitor);
            //texture_visitor.write(osgDB::getFilePath(file_name));

            return osg_root;
        }
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

            if (options)
            {
                const static char strip = '\'';
                const static char sep = ',';

                std::string option_string = options->getOptionString();
                {
                    auto stripped_begin = option_string.find_first_not_of(strip);
                    auto stripped_end = option_string.find_last_not_of(strip);
                    option_string = option_string.substr(stripped_begin, stripped_end - stripped_begin + 1);
                }

                for (size_t offset = 0; offset < std::string::npos; ) {
                    size_t next_sep = option_string.find_first_of(sep, offset);
                    if (next_sep != std::string::npos && next_sep - offset > 0 || offset + 1 < option_string.size())
                    {
                        auto arg = option_string.substr(offset, next_sep - offset);
                        char *next_arg = new char[arg.size() + 1];
                        std::copy(arg.begin(), arg.end(), next_arg);
                        next_arg[arg.size()] = '\0';
                        argv.push_back(next_arg);
                    }
                    offset = next_sep == std::string::npos ? next_sep : next_sep + 1;
                }
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

            // write aoa from cache if USE_DUMMY
            if (arguments.read("USE_DUMMY"))
            { 
                if (float tesselate_param; arguments.read("TESSELATE", tesselate_param)) 
                {
                    OSG_INFO << "Splitting to size " << tesselate_param << std::endl;
                    mesh_subdivider subdivider(aoa_path, tesselate_param);

                    write_processor wp;
                    reflect(wp, subdivider.processed_aoa());
                    std::ofstream(file_name) << wp.result();

                    auto aod = subdivider.processed_aod();

                    auto data_file_name = fs::path(file_name).replace_extension("aod").string();
                    std::ofstream(data_file_name, std::ios_base::binary | std::ios_base::out
                    ).write(reinterpret_cast<char *>(aod.data()), aod.size()).flush();
                } 
                else
                {
                    throw std::logic_error("AOA to AOA conversion redundant: copy files instead ;)");
                }
            } 
            else
            {
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
                //aurora::convert_textures_visitor texture_visitor("dds");
                //osg_root.accept(texture_visitor);
                //texture_visitor.write(osgDB::getFilePath(file_name));

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
                while (remove_hanging_tv.remove_hanging_transforms());

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

private:
    mutable std::string aoa_path;
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(aoa, ReaderWriterAOA)