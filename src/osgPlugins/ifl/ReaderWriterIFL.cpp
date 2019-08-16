#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osg/Texture2DArray>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <cctype>
#include <filesystem>

// Macro similar to what's in FLT/TRP plugins (except it uses wide char under Windows if OSG_USE_UTF8_FILENAME)
#if defined(_WIN32)
#include <windows.h>
#include <osg/Config>
#include <osgDB/ConvertUTF>
#ifdef OSG_USE_UTF8_FILENAME
#define DELETEFILE(file) DeleteFileW(osgDB::convertUTF8toUTF16((file)).c_str())
#else
#define DELETEFILE(file) DeleteFileA((file))
#endif

#else   // Unix

#include <stdio.h>
#define DELETEFILE(file) remove((file))

#endif

using std::filesystem::path;

class ReaderWriterIFL : public osgDB::ReaderWriter
{
public:

    ReaderWriterIFL()
    {
        supportsExtension("ifl","IFL image format");
    }

    virtual const char* className() const
    {
        return "IFL Image Reader/Writer";
    }

    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );

        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        osgDB::ifstream stream(fileName.c_str(), std::ios::in | std::ios::binary);
        if(!stream) return ReadResult::FILE_NOT_HANDLED;
        ReadResult rr = readObject(stream, options, path(file).parent_path());
        if(rr.validObject()) rr.getObject()->setName(file);
        return rr;
    }

    virtual ReadResult readObject(std::istream& fin, const Options* options) const
    {
        return readObject(fin, options, path{});
    }

    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        return ReadResult::NOT_IMPLEMENTED;
    }

    virtual ReadResult readImage(std::istream& fin, const Options* options) const
    {
        return ReadResult::NOT_IMPLEMENTED;
    }

    ReadResult readObject(std::istream& fin, const Options* options, path basedir) const
    {
        osg::ref_ptr<osg::Texture2DArray> tex = new osg::Texture2DArray();

        unsigned layer = 0;
        for(std::string line; std::getline(fin, line);)
        {
            // strip trailing whitespaces
            {
                auto it = line.rbegin();
                while(it != line.rend() && std::isspace(*it))
                    it++;

                line.erase(it.base(), line.end());
            }

            if(!line.empty())
            {
                auto imageRes = osgDB::Registry::instance()->readImage((basedir / line).string(), options);
                if(imageRes.validImage())
                {
                    tex->setImage(layer++, imageRes.getImage());
                }
                else
                {
                    OSG_WARN << "IFL plugin: could not read texture" << (basedir / line).string() << std::endl;
                }
            }
        }
        return tex;
    }

    virtual WriteResult writeObject(const osg::Object& object,const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        auto tex = dynamic_cast<const osg::Texture2DArray*>(&object);
        if (!tex) return WriteResult::NOT_IMPLEMENTED;

        std::string ext = osgDB::getFileExtension(file);
        if(!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        osgDB::ofstream fout(file.c_str(), std::ios::out | std::ios::binary);
        if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

        WriteResult res(writeObject(object, fout, options, path(file).parent_path()));
        if(!res.success()) {
            // Remove file on failure
            fout.close();
            DELETEFILE(file.c_str());
        }
        return res;
    }

    WriteResult writeObject(const osg::Object& object,std::ostream& fout,const Options* options, path basedir) const
    {
        auto tex = dynamic_cast<const osg::Texture2DArray*>(&object);
        if(!tex) return WriteResult::FILE_NOT_HANDLED;

        for(unsigned i = 0; i < tex->getNumImages(); ++i)
        {
            std::string imageFile = tex->getImage(i)->getFileName();
            auto relative_name = relative(path(imageFile), path(object.getName()).parent_path()).string();
            if(!relative_name.empty())
            {
                if(osgDB::Registry::instance()->writeImage(*tex->getImage(i), (basedir / relative_name).string(), options).success())
                    fout << relative_name << std::endl;
            }
            else
                OSG_WARN << "IFL plugin: could not get relative path for texture " << imageFile << std::endl;
        }

        return WriteResult::FILE_SAVED;
    }

    virtual WriteResult writeObject(const osg::Object& object, std::ostream& fout, const Options* options) const override
    {
        return writeObject(object, fout, options, path{});
    }

    virtual WriteResult writeImage(const osg::Image &image,const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        return WriteResult::NOT_IMPLEMENTED;
    }

    virtual WriteResult writeImage(const osg::Image& image,std::ostream& fout,const Options* options) const
    {
        return WriteResult::NOT_IMPLEMENTED;
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(ifl, ReaderWriterIFL)
