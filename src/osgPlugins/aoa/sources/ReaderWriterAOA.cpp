
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
#include <osgDB/WriteFile>
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
#include "fix_materials_visitor.h"

using namespace aurora;

class MakeTransformsStaticVisitor: public osg::NodeVisitor
{
    using osg::NodeVisitor::NodeVisitor;

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

string material_name_for_chunk(string name, material_info const& data)
{
    if(!data.explicit_material.empty())
    {
        return data.explicit_material;
    } 
    else 
    {
        return name + "_mtl";
    }
}

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
            osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform(osg::Matrix(-1, 0, 0, 0, 
                                                                                                 0, 0, 1, 0,
                                                                                                 0, 1, 0, 0,  
                                                                                                 0, 0, 0, 1));
            transform->addChild(const_cast<osg::Node*>(&node));

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

            bool flip_yz = arguments.read("--aoa-flip-yz");
            osg::Node& osg_root = flip_yz ?  *transform : const_cast<osg::Node&>(node);
            OSG_INFO << "flip Y and Z: " << flip_yz << "\n";

            string materials_file;
            arguments.read("--aoa-materials-file", materials_file);
            if(materials_file.empty())
                OSG_WARN << "AOA plugin: materials file is not specified\n";
    
            material_loader mat_loader(materials_file);

            fix_materials_visitor fix_mats_vis(mat_loader, fs::path(materials_file).parent_path().string());
            osg_root.accept(fix_mats_vis);

            // convert all textures to bmp
            aurora::convert_textures_visitor texture_visitor;
            const_cast<osg::Node&>(node).accept(texture_visitor);
            texture_visitor.write(osgDB::getFilePath(file_name));

            // apply transforms from ancestor nodes to geometry
            // optimizer will only do this for transform nodes whose data variance is STATIC so we set it here 
            MakeTransformsStaticVisitor make_tranforms_static(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
            osg_root.accept(make_tranforms_static);

            // run the optimizer
            osgUtil::Optimizer optimizer;
            optimizer.optimize(&osg_root, osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS);


            aurora::geometry_visitor geom_visitor(mat_loader);

            osg_root.accept(geom_visitor);

            OSG_INFO << "EXTRACTED " << geom_visitor.get_chunks().size() << " CHUNKS" << std::endl;
            OSG_INFO << "EXTRACTED " << geom_visitor.get_faces().size() << " FACES" << std::endl;
            OSG_INFO << "EXTRACTED " << geom_visitor.get_verticies().size() << " VIRTICES" << std::endl;
            OSG_INFO << "Writing node to AOA file " << file_name << std::endl;

            aurora::aoa_writer file_writer(fs::path(file_name).replace_extension("aod").string());
            vector<aod::omni_light> omni_lights;
            vector<aod::spot_light> spot_lights;

            aod::omni_light light;
            light.position[0] = 0;
            light.position[1] = -10;
            light.position[2] = 13.;

            light.power = 1000.;
            light.color = geom::colorb(255, 0, 0);
            light.r_min = 255;

            omni_lights.push_back(light);


            aod::spot_light spot;
            spot.position[0] = 0;
            spot.position[1] = -30;
            spot.position[2] = 13.;
            spot.power = 1000000;
            spot.color = geom::color_blue();
            spot.r_min = 1;
            spot.dir_x = 0.;
            spot.dir_y = 1.;
            spot.dir_z = 0.;
            spot.mask = 0x7;
            spot.half_fov = geom::grad2rad(45.);
            spot.angular_power = 1.;

            spot_lights.push_back(spot);

            file_writer.set_omni_lights_buffer_data(omni_lights);
            file_writer.set_spot_lights_buffer_data(spot_lights);
            file_writer.set_index_buffer_data(geom_visitor.get_faces());
            file_writer.set_vertex_buffer_data(geom_visitor.get_verticies());

            aoa_writer::node_ptr root = file_writer.create_root_node(fs::path(file_name).stem().string());

            if(omni_lights.size())
                root->set_omni_lights(0, omni_lights.size());
            if(spot_lights.size())
                root->set_spot_lights(0, spot_lights.size());

            for(auto const& chunk : geom_visitor.get_chunks())
            {
                file_writer.add_material(material_name_for_chunk(chunk.name, chunk.material), chunk.material);
                root->create_child(chunk.name)
                    ->add_mesh(chunk.aabb, chunk.faces_range.lo() * 3,
                               chunk.faces_range.hi() - chunk.faces_range.lo(),
                               chunk.vertex_range.lo(),
                               chunk.vertex_range.hi() - chunk.vertex_range.lo(),
                               material_name_for_chunk(chunk.name, chunk.material));
            }

            file_writer.save_data();

            // ======================= DEBUG OUTPUT ============================
            //write_debug_obj_file(geom_visitor, file_name);
            //osgDB::writeNodeFile(osg_root, fs::path(file_name).replace_extension("osg").string());
            //osgDB::writeNodeFile(osg_root, fs::path(file_name).replace_extension("fbx").string());
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