#include "sampler.h"
#include "dds.h"

#include <utility>
#include <vector>
#include <algorithm>
#include <optional>
#include <iostream>
#include <fstream>

#include <osg/Image>

#include "btx_to_dds.h"

using DirectX::DDS_HEADER;
using DirectX::DDS_HEADER_DXT10;

#pragma pack(push, 1)

struct BTXHeader
{
    unsigned		magic;
    unsigned		hash;
    unsigned		dataSize;
    unsigned		swizzle;
    unsigned short	target;
    unsigned short	format;
    unsigned short	width;
    unsigned short	height;
    unsigned short	depth;
    unsigned short	mipmaps;
    unsigned short	batchesNum;
    unsigned short	reserved;
};

struct BTXBatch
{
    unsigned		offset;
    unsigned		size;
    unsigned short	x;
    unsigned short	y;
    unsigned short	w;
    unsigned short	h;
    unsigned short	layerOrFace;
    unsigned char	mipmap;
    unsigned char	action;
};

#pragma pack(pop)

using std::pair;
using std::vector;
using std::optional;

template<class It>
struct generator_iterator
{
    generator_iterator(It begin, It end, int increment)
        : it_(begin)
        , end_(end)
        , increment_(increment)
    {}

    generator_iterator& operator++()
    {
        if(it_ != end_)
            std::advance(it_, increment_);
        return *this;
    }

    generator_iterator operator++(int)
    {
        auto result = *this;
        ++*this;
        return result;
    }

    operator bool() const
    {
        return it_ != end_;
    }

    It base() const
    {
        return it_;
    }

private:
    It  it_;
    It  end_;
    int increment_;
};

template<class It> 
auto make_generator_iterator(It begin, It end, int increment = 1)
{
    return generator_iterator<It>(begin, end, increment);
}

void assemble_batches(BTXHeader const& btx_header, vector<pair<BTXBatch, vector<char>>> & batches, std::ostream& out)
{
    std::sort(batches.begin(), batches.end(), [](auto const& l, auto const& r)
    {
        return std::make_tuple(l.first.mipmap, l.first.layerOrFace, l.first.y, l.first.x) < std::make_tuple(r.first.mipmap, r.first.layerOrFace, r.first.y, r.first.x);
    });

    auto format_info = Sampler::GetFormatInfo(Sampler::Format(btx_header.format));

    unsigned block_width_pixels = format_info->Flags(Sampler::eTextureFormatFlag::TEXTURE_BC) ? 4 : 1;
    unsigned bytes_in_block     = format_info->Bytes();

    auto it_begin = batches.begin();
    auto it_end   = batches.begin();

    vector<generator_iterator<vector<char>::const_iterator>> group_chunks;

    auto next_group = [&it_begin, &it_end, &batches, &group_chunks, bytes_in_block, block_width_pixels]()
    {
        it_begin = it_end;

        while(it_end != batches.end() && it_end->first.mipmap == it_begin->first.mipmap 
            && it_end->first.layerOrFace == it_begin->first.layerOrFace
            && it_end->first.y == it_begin->first.y)
            ++it_end;

        group_chunks.clear();

        for(auto b = it_begin; b != it_end; ++b)
        {
            group_chunks.emplace_back(make_generator_iterator(b->second.cbegin(), b->second.cend(), 
                std::max(bytes_in_block, b->first.w / block_width_pixels * bytes_in_block))
            );
        }
    };

    auto out_it = std::ostreambuf_iterator(out);

    while(it_begin != batches.end())
    {
        next_group();
        while(true)
        {
            bool not_done = false;
            for(auto& c: group_chunks)
            {
                if(c)
                {
                    auto b = c.base();
                    auto e = (++c).base();
                    std::copy(b, e, out_it);
                    not_done = true;
                }
            }
            if(!not_done)
                break;
        }
    }
}

struct DDSDescription
{
    uint32_t                           magic = MAKE_FOURCC('D', 'D', 'S', ' ');
    DDS_HEADER                         header;
    std::optional<DDS_HEADER_DXT10>    header10;
};

std::ostream& operator<< (std::ostream& out, DDSDescription const& descr)
{
    out.seekp(0);
    out.write((char *) &descr.magic, sizeof(descr.magic));
    out.write((char *) &descr.header, sizeof(descr.header));
    if(descr.header10)
        out.write((char *) &descr.header10.value(), sizeof(descr.header10.value()));
    return out;
}

//DDSDescription dds_header_from_btx(BTXHeader const& header)
//{
//    DDSDescription result;
//
//    bool isDXTC = false;
//
//    // Initialize ddsd structure and its members
//    DDS_HEADER ddsd;
//    memset(&ddsd, 0, sizeof(ddsd));
//    DDPIXELFORMAT  ddpf;
//    memset(&ddpf, 0, sizeof(ddpf));
//    //DDCOLORKEY     ddckCKDestOverlay;
//    //DDCOLORKEY     ddckCKDestBlt;
//    //DDCOLORKEY     ddckCKSrcOverlay;
//    //DDCOLORKEY     ddckCKSrcBlt;
//    DDSCAPS2       ddsCaps;
//    memset(&ddsCaps, 0, sizeof(ddsCaps));
//
//    ddsd.size = sizeof(ddsd);
//    ddpf.ddspf.size = sizeof(ddpf.ddspf);
//
//    // Default values and initialization of structures' flags
//    unsigned int SD_flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
//    unsigned int CAPS_flags = DDSCAPS_TEXTURE;
//    unsigned int PF_flags = 0;
//    unsigned int CAPS2_flags = 0;
//
//    auto format_info = Sampler::GetFormatInfo(Sampler::Format(header.format));
//
//    switch(format_info->InternalFormat())
//
//    // Get image properties
//    unsigned int dataType = img->getDataType();
//    unsigned int pixelFormat = img->getPixelFormat();
//    //unsigned int internalFormat = img->getInternalTextureFormat();
//    //unsigned int components     = osg::Image::computeNumComponents(pixelFormat);
//    unsigned int pixelSize = osg::Image::computePixelSizeInBits(pixelFormat, dataType);
//    unsigned int imageSize = img->getTotalSizeInBytes();
//
//    OSG_INFO << "WriteDDS, dataType = 0x" << std::hex << dataType << std::endl;
//
//    // Check that theorical image size (computation taking into account DXTC blocks) is not bigger than actual image size.
//     // This may happen, for instance, if some operation tuncated the data buffer non block-aligned. Example:
//     //  - Read DXT1 image, size = 8x7. Actually, image data is 8x8 because it stores 4x4 blocks.
//     //  - Some hypothetical operation wrongly assumes the data buffer is 8x7 and truncates the buffer. This may even lead to access violations.
//     //  - Then we write the DXT1 image: last block(s) is (are) corrupt.
//     // Actually what could be very nice is to handle some "lines packing" (?) in DDS reading, indicating that the image buffer has "additional lines to reach a multiple of 4".
//     // Please note this can also produce false positives (ie. when data buffer is large enough, but getImageSizeInBytes() returns a smaller value). There is no way to detect this, until we fix getImageSizeInBytes() with "line packing".
//    unsigned int imageSizeTheorical = ComputeImageSizeInBytes(img->s(), img->t(), img->r(), pixelFormat, dataType, img->getPacking());
//    if(imageSize < imageSizeTheorical) {
//        OSG_FATAL << "Image cannot be written as DDS (Maybe a corrupt S3TC-DXTC image, with non %4 dimensions)." << std::endl;
//        return false;
//    }
//
//    ddsd.dwWidth = img->s();
//    ddsd.dwHeight = img->t();
//    int r = img->r();
//
//    if(r > 1)  /* check for 3d image */
//    {
//        ddsd.dwDepth = r;
//        SD_flags |= DDSD_DEPTH;
//        CAPS_flags |= DDS_constants::DDSF_COMPLEX;
//        CAPS2_flags |= DDS_constants::DDSF_VOLUME;
//    }
//
//
//    return result;
//}

typedef unsigned int UI32;
typedef int I32;

struct  DDCOLORKEY
{
    DDCOLORKEY() :
        dwColorSpaceLowValue(0),
        dwColorSpaceHighValue(0) {}

    UI32    dwColorSpaceLowValue;
    UI32    dwColorSpaceHighValue;
};

struct DDPIXELFORMAT
{

    DDPIXELFORMAT() :
        dwSize(0),
        dwFlags(0),
        dwFourCC(0),
        dwRGBBitCount(0),
        dwRBitMask(0),
        dwGBitMask(0),
        dwBBitMask(0),
        dwRGBAlphaBitMask(0) {}


    UI32    dwSize;
    UI32    dwFlags;
    UI32    dwFourCC;
    union
    {
        UI32    dwRGBBitCount;
        UI32    dwYUVBitCount;
        UI32    dwZBufferBitDepth;
        UI32    dwAlphaBitDepth;
        UI32    dwLuminanceBitDepth;
    };
    union
    {
        UI32    dwRBitMask;
        UI32    dwYBitMask;
    };
    union
    {
        UI32    dwGBitMask;
        UI32    dwUBitMask;
    };
    union
    {
        UI32    dwBBitMask;
        UI32    dwVBitMask;
    };
    union
    {
        UI32    dwRGBAlphaBitMask;
        UI32    dwYUVAlphaBitMask;
        UI32    dwRGBZBitMask;
        UI32    dwYUVZBitMask;
    };
};

struct  DDSCAPS2
{
    DDSCAPS2() :
        dwCaps(0),
        dwCaps2(0),
        dwCaps3(0),
        dwCaps4(0) {}

    UI32       dwCaps;
    UI32       dwCaps2;
    UI32       dwCaps3;
    union
    {
        UI32       dwCaps4;
        UI32       dwVolumeDepth;
    };
};

struct DDSURFACEDESC2
{
    DDSURFACEDESC2() :
        dwSize(0),
        dwFlags(0),
        dwHeight(0),
        dwWidth(0),
        lPitch(0),
        dwBackBufferCount(0),
        dwMipMapCount(0),
        dwAlphaBitDepth(0),
        dwReserved(0),
        lpSurface(0),
        dwTextureStage(0) {}


    UI32         dwSize;
    UI32         dwFlags;
    UI32         dwHeight;
    UI32         dwWidth;
    union
    {
        I32              lPitch;
        UI32     dwLinearSize;
    };
    union
    {
        UI32      dwBackBufferCount;
        UI32      dwDepth;
    };
    union
    {
        UI32     dwMipMapCount;
        UI32     dwRefreshRate;
    };
    UI32         dwAlphaBitDepth;
    UI32         dwReserved;
    UI32        lpSurface;         //Fred Marmond: removed from pointer type to UI32 for 64bits compatibility. it is unused data
    DDCOLORKEY    ddckCKDestOverlay;
    DDCOLORKEY    ddckCKDestBlt;
    DDCOLORKEY    ddckCKSrcOverlay;
    DDCOLORKEY    ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2      ddsCaps;
    UI32 dwTextureStage;
};

bool WriteDDSHeader(BTXHeader const& header, std::ostream& fout, bool autoFlipDDSWrite = false)
{
    bool isDXTC(false);

    // Initialize ddsd structure and its members
    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    DDPIXELFORMAT  ddpf;
    memset(&ddpf, 0, sizeof(ddpf));

    optional<DDS_HEADER_DXT10> dds_dx10header;

    //DDCOLORKEY     ddckCKDestOverlay;
    //DDCOLORKEY     ddckCKDestBlt;
    //DDCOLORKEY     ddckCKSrcOverlay;
    //DDCOLORKEY     ddckCKSrcBlt;
    DDSCAPS2       ddsCaps;
    memset(&ddsCaps, 0, sizeof(ddsCaps));

    ddsd.dwSize = sizeof(ddsd);
    ddpf.dwSize = sizeof(ddpf);

    // Default values and initialization of structures' flags
    unsigned int SD_flags = DDS_constants::DDSF_CAPS | DDS_constants::DDSF_HEIGHT | DDS_constants::DDSF_WIDTH | DDS_constants::DDSF_PIXELFORMAT;
    unsigned int CAPS_flags = DDS_constants::DDSF_TEXTURE;
    unsigned int PF_flags = 0;
    unsigned int CAPS2_flags = 0;

    auto format_info = Sampler::GetFormatInfo(Sampler::Format(header.format));

    unsigned block_width_pixels = format_info->Flags(Sampler::eTextureFormatFlag::TEXTURE_BC) ? 4 : 1;
    unsigned bytes_in_block = format_info->Bytes();
    unsigned row_size_bytes = std::max(bytes_in_block, header.width / block_width_pixels * bytes_in_block);

    // Get image properties
    unsigned int dataType = format_info->Type();
    unsigned int pixelFormat = format_info->InternalFormat();
    //unsigned int internalFormat = img->getInternalTextureFormat();
    //unsigned int components     = osg::Image::computeNumComponents(pixelFormat);
    unsigned int pixelSize = osg::Image::computePixelSizeInBits(pixelFormat, dataType);
    unsigned int imageSize = header.dataSize;

    OSG_INFO << "WriteDDS, dataType = 0x" << std::hex << dataType << std::endl;

    // Check that theorical image size (computation taking into account DXTC blocks) is not bigger than actual image size.
     // This may happen, for instance, if some operation tuncated the data buffer non block-aligned. Example:
     //  - Read DXT1 image, size = 8x7. Actually, image data is 8x8 because it stores 4x4 blocks.
     //  - Some hypothetical operation wrongly assumes the data buffer is 8x7 and truncates the buffer. This may even lead to access violations.
     //  - Then we write the DXT1 image: last block(s) is (are) corrupt.
     // Actually what could be very nice is to handle some "lines packing" (?) in DDS reading, indicating that the image buffer has "additional lines to reach a multiple of 4".
     // Please note this can also produce false positives (ie. when data buffer is large enough, but getImageSizeInBytes() returns a smaller value). There is no way to detect this, until we fix getImageSizeInBytes() with "line packing".

     //unsigned int imageSizeTheorical = ComputeImageSizeInBytes( img->s(), img->t(), img->r(), pixelFormat, dataType, img->getPacking() );
     //if (imageSize < imageSizeTheorical) {
     //    OSG_FATAL << "Image cannot be written as DDS (Maybe a corrupt S3TC-DXTC image, with non %4 dimensions)." << std::endl;
     //    return false;
     //}

    ddsd.dwWidth = header.width;
    ddsd.dwHeight = header.height;
    int r = header.depth;

    if(r > 1)  /* check for 3d image */
    {
        //if(header.target == Sampler::Target::Texture1dArray ||
        //    header.target == Sampler::Target::Texture2dArray)
        //{
        //    dds_dx10header.emplace();
        //    dds_dx10header->arraySize = r;

        //    if(header.target == Sampler::Target::Texture1dArray)
        //        dds_dx10header->resourceDimension = DDS_RESOURCE_DIMENSION::DDS_DIMENSION_TEXTURE1D;
        //    else
        //        dds_dx10header->resourceDimension = DDS_RESOURCE_DIMENSION::DDS_DIMENSION_TEXTURE2D;
        //}
        ddsd.dwDepth = r;
        SD_flags |= DDS_constants::DDSF_DEPTH;
        CAPS_flags |= DDS_constants::DDSF_COMPLEX;
        CAPS2_flags |= DDS_constants::DDSF_VOLUME;
    }

    // Determine format - set flags and ddsd, ddpf properties
    switch(pixelFormat)
    {
        //Uncompressed
    case GL_RGBA:
    {
        ddpf.dwRBitMask = 0x000000ff;
        ddpf.dwGBitMask = 0x0000ff00;
        ddpf.dwBBitMask = 0x00ff0000;
        ddpf.dwRGBAlphaBitMask = 0xff000000;
        PF_flags |= (DDS_constants::DDSF_ALPHAPIXELS | DDS_constants::DDSF_RGB);
        ddpf.dwRGBBitCount = pixelSize;
        ddsd.lPitch = row_size_bytes;
        SD_flags |= DDS_constants::DDSF_PITCH;
    }
    break;
    case GL_BGRA:
    {
        ddpf.dwBBitMask = 0x000000ff;
        ddpf.dwGBitMask = 0x0000ff00;
        ddpf.dwRBitMask = 0x00ff0000;
        ddpf.dwRGBAlphaBitMask = 0xff000000;
        PF_flags |= (DDS_constants::DDSF_ALPHAPIXELS | DDS_constants::DDSF_RGB);
        ddpf.dwRGBBitCount = pixelSize;
        ddsd.lPitch = row_size_bytes;
        SD_flags |= DDS_constants::DDSF_PITCH;
    }
    break;
    case GL_LUMINANCE_ALPHA:
    {
        ddpf.dwRBitMask = 0x000000ff;
        ddpf.dwRGBAlphaBitMask = 0x0000ff00;
        PF_flags |= (DDS_constants::DDSF_ALPHAPIXELS | DDS_constants::DDSF_LUMINANCE);
        ddpf.dwRGBBitCount = pixelSize;
        ddsd.lPitch = row_size_bytes;
        SD_flags |= DDS_constants::DDSF_PITCH;
    }
    break;
    case GL_RGB:
    {
        ddpf.dwRBitMask = 0x000000ff;
        ddpf.dwGBitMask = 0x0000ff00;
        ddpf.dwBBitMask = 0x00ff0000;
        PF_flags |= DDS_constants::DDSF_RGB;
        ddpf.dwRGBBitCount = pixelSize;
        ddsd.lPitch = row_size_bytes;
        SD_flags |= DDS_constants::DDSF_PITCH;
    }
    break;
    case GL_BGR:
    {
        ddpf.dwBBitMask = 0x000000ff;
        ddpf.dwGBitMask = 0x0000ff00;
        ddpf.dwRBitMask = 0x00ff0000;
        PF_flags |= DDS_constants::DDSF_RGB;
        ddpf.dwRGBBitCount = pixelSize;
        ddsd.lPitch = row_size_bytes;
        SD_flags |= DDS_constants::DDSF_PITCH;
    }
    break;
    case GL_LUMINANCE:
    case GL_DEPTH_COMPONENT:
    {
        ddpf.dwRBitMask = 0x000000ff;
        PF_flags |= DDS_constants::DDSF_LUMINANCE;
        ddpf.dwRGBBitCount = pixelSize;
        ddsd.lPitch = row_size_bytes;
        SD_flags |= DDS_constants::DDSF_PITCH;
    }
    break;
    case GL_ALPHA:
    {
        ddpf.dwRGBAlphaBitMask = 0x000000ff;
        PF_flags |= DDS_constants::DDSF_ALPHA;
        ddpf.dwRGBBitCount = pixelSize;
        ddsd.lPitch = row_size_bytes;
        SD_flags |= DDS_constants::DDSF_PITCH;
    }
    break;

    //Compressed
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    {
        isDXTC = true;
        ddpf.dwFourCC = DDS_constants::FOURCC_DXT1;
        PF_flags |= (DDS_constants::DDSF_ALPHAPIXELS | DDS_FOURCC);
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    {
        isDXTC = true;
        ddpf.dwFourCC = DDS_constants::FOURCC_DXT3;
        PF_flags |= (DDS_constants::DDSF_ALPHAPIXELS | DDS_FOURCC);
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
    {
        isDXTC = true;
        ddpf.dwFourCC = DDS_constants::FOURCC_DXT5;
        PF_flags |= (DDS_constants::DDSF_ALPHAPIXELS | DDS_FOURCC);
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    {
        isDXTC = true;
        ddpf.dwFourCC = DDS_constants::FOURCC_DXT1;
        PF_flags |= DDS_FOURCC;  /* No alpha here */
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
    {
        ddpf.dwFourCC = DDS_constants::FOURCC_ATI1;
        PF_flags |= DDS_FOURCC;  /* No alpha here */
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    case GL_COMPRESSED_RED_RGTC1_EXT:
    {
        ddpf.dwFourCC = DDS_constants::FOURCC_ATI1;
        PF_flags |= DDS_FOURCC;  /* No alpha here */
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
    {
        ddpf.dwFourCC = DDS_constants::FOURCC_ATI2;
        PF_flags |= DDS_FOURCC;  /* No alpha here */
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
    {
        ddpf.dwFourCC = DDS_constants::FOURCC_ATI2;
        PF_flags |= DDS_FOURCC;  /* No alpha here */
        ddsd.dwLinearSize = imageSize;
        SD_flags |= DDS_constants::DDSF_LINEARSIZE;
    }
    break;
    default:
        OSG_WARN << "Warning:: unhandled pixel format in image, file cannot be written." << std::endl;
        return false;
    }

    // set even more flags
    if(header.mipmaps == 1) {

        OSG_INFO << "no mipmaps to write out." << std::endl;

        //} else if( img->getPacking() > 1 ) {

        //   OSG_WARN<<"Warning: mipmaps not written. DDS requires packing == 1."<<std::endl;

    }
    else { // image contains mipmaps and has 1 byte alignment

        SD_flags |= DDS_constants::DDSF_MIPMAPCOUNT;
        CAPS_flags |= DDS_constants::DDSF_COMPLEX | DDS_constants::DDSF_MIPMAP;

        ddsd.dwMipMapCount = header.mipmaps;

        OSG_INFO << "writing out with mipmaps ddsd.dwMipMapCount" << ddsd.dwMipMapCount << std::endl;
    }

    // Assign flags and structure members of ddsd
    ddsd.dwFlags = SD_flags;
    ddpf.dwFlags = PF_flags;
    ddsCaps.dwCaps = CAPS_flags;
    ddsCaps.dwCaps2 = CAPS2_flags;

    ddsd.ddpfPixelFormat = ddpf;
    ddsd.ddsCaps = ddsCaps;

    //osg::ref_ptr<const osg::Image> source;
    //if (autoFlipDDSWrite && img->getOrigin() == osg::Image::BOTTOM_LEFT)
    //{
    //    OSG_INFO<<"Flipping dds image on write"<<std::endl;

    //    osg::ref_ptr<osg::Image> copy( new osg::Image(*img,osg::CopyOp::DEEP_COPY_ALL) );
    //    const int s(copy->s());
    //    const int t(copy->t());
    //    if (!isDXTC || ((s>4 && s%4==0 && t>4 && t%4==0) || s<=4)) // Flip may crash (access violation) or fail for non %4 dimensions (except for s<4). Tested with revision trunk 2013-02-22.
    //    {
    //        copy->flipVertical();
    //    }
    //    else
    //    {
    //        OSG_WARN << "WriteDDSFile warning: Vertical flip was skipped. Image dimensions have to be multiple of 4." << std::endl;
    //    }
    //    source = copy;
    //}
    //else
    //{
    //    source = img;
    //}

    // Write DDS file
    fout.write("DDS ", 4); /* write FOURCC */
    fout.write(reinterpret_cast<char*>(&ddsd), sizeof(ddsd)); /* write file header */

    //for(osg::Image::DataIterator itr(source.get()); itr.valid(); ++itr)
    //{
    //    fout.write(reinterpret_cast<const char*>(itr.data()), itr.size() );
    //}

    // Check for correct saving
    if(fout.fail())
        return false;

    // If we get that far the file was saved properly //
    return true;
}



bool btx_to_dds_impl(BTXHeader const& header, std::istream& in, std::ostream& out)
{
    //auto dds_header = dds_header_from_btx(header);
    //out << dds_header;

    if(!WriteDDSHeader(header, out))
        return false;

    in.seekg(sizeof(header));

    if(header.batchesNum)
    {
        vector<pair<BTXBatch, vector<char>>> batches;
        for(int i = 0; i < header.batchesNum; ++i)
        {
            BTXBatch batch;
            in.read(reinterpret_cast<char*>(&batch), sizeof(batch));
            auto pos = in.tellg();
            vector<char> batch_data(batch.size);
            in.seekg(sizeof(header) + batch.offset);
            in.read(batch_data.data(), batch.size);
            in.seekg(pos);

            // only batches with data
            if(batch.action == 0)
                batches.emplace_back(batch, move(batch_data));
        }

        assemble_batches(header, batches, out);
    }
    else
    {
        out << in.rdbuf();
    }

    return !out.fail();
}

bool btx_to_dds(std::istream& in, std::string out_path)
{
    in.seekg(0);
    BTXHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if(in.gcount() != sizeof(header))
        return false;

    if(header.magic != MAKE_FOURCC('B', 'T', 'X', '2'))
        return false;

    std::ofstream out(out_path, std::ios_base::binary | std::ios_base::out);

    if(!out.good())
        return false;

    return btx_to_dds_impl(header, in, out);
}

bool btx_to_dds(std::istream& in, std::ostream& out)
{
    in.seekg(0);
    BTXHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if(in.gcount() != sizeof(header))
        return false;

    if(header.magic != MAKE_FOURCC('B', 'T', 'X', '2'))
        return false;

    return btx_to_dds_impl(header, in, out);
}

