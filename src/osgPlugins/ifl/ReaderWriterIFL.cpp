#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <cctype>
#include <filesystem>

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
        return readImage(file,options);
    }

    virtual ReadResult readObject(std::istream& fin, const Options* options) const
    {
        return readImage(fin,options);
    }

    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );

        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        osgDB::ifstream stream(fileName.c_str(), std::ios::in | std::ios::binary);
        if(!stream) return ReadResult::FILE_NOT_HANDLED;
        ReadResult rr = readImage(stream, options, path(file).parent_path());
        if(rr.validImage()) rr.getImage()->setFileName(file);
        return rr;
    }

    virtual ReadResult readImage(std::istream& fin, const Options* options) const
    {
        return readImage(fin, options, path{});
    }

    ReadResult readImage(std::istream& fin, const Options* options, path basedir) const
    {
        std::string line;
        std::getline(fin, line);

        // strip trailing whitespaces
        {
            auto it = line.rbegin();
            while(it != line.rend() && std::isspace(*it))
                it++;

            line.erase(it.base(), line.end());
        }

        if(!line.empty())
            return osgDB::Registry::instance()->getReaderWriterForExtension("btx")->readImage((basedir / line).string(), options);
        else
            return ReadResult::ERROR_IN_READING_FILE;
    }

    virtual WriteResult writeObject(const osg::Object& object,const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
        if (!image) return WriteResult::FILE_NOT_HANDLED;

        return writeImage(*image,file,options);
    }

    virtual WriteResult writeObject(const osg::Object& object,std::ostream& fout,const Options* options) const
    {
        const osg::Image* image = dynamic_cast<const osg::Image*>(&object);
        if(!image) return WriteResult::FILE_NOT_HANDLED;

        return writeImage(*image, fout, options);
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
