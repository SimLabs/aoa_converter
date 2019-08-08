#include "sampler.h"

//#include "renderer.h"

#include "dds.h"

//#include "sphere.h"
//#include "zbuffer.h"

//#include "scoped_file.h"
#include <map>

#define		TR_MIN(x,y)						((x) < (y)? (x) : (y))
#define		TR_MAX(x,y)						((x) > (y)? (x) : (y))
#define		TR_MAX4(a,b,c,d)				(TR_MAX(TR_MAX(a,b),TR_MAX(c,d)))
#define		TR_MIN4(a,b,c,d)				(TR_MIN(TR_MIN(a,b),TR_MIN(c,d)))

#define MEM_Alloc(type, size) new type[size]
#define MEM_Free(type, ptr) delete [] ptr

namespace
{
    const char* fourcc(uint32_t enc) 
    {
        static thread_local char c[5] = { '\0' };
        c[0] = enc >> 0 & 0xFF;
        c[1] = enc >> 8 & 0xFF;
        c[2] = enc >> 16 & 0xFF;
        c[3] = enc >> 24 & 0xFF;
        return c;
    }
}

namespace Sampler
{
	StateObject g_predefinedStateObjects[PREDEFINED_STATE_OBJECTS_NUM]; 

	const FormatInfo g_samplerFormatInfo[FORMATS_NUM] =
	{
		FormatInfo
		(
			"UNKNOWN",
			0,
			0,
			0,
			GL_NONE,
			GL_NONE,
			GL_NONE
		),

		// NOTE: plane 8-bit per channel

		FormatInfo
		(
			"R8",
			1,
			TEXTURE_BYTE | TEXTURE_RENDERABLE,
			1,
			GL_UNSIGNED_BYTE,
			GL_RED,
			GL_R8
		),

		FormatInfo
		(
			"R8s",
			1,
			TEXTURE_BYTE | TEXTURE_SIGNED,
			1,
			GL_BYTE,
			GL_RED,
			GL_R8_SNORM
		),

		FormatInfo
		(
			"R8i",
			1,
			TEXTURE_BYTE | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			1,
			GL_BYTE,
			GL_RED_INTEGER,
			GL_R8I
		),

		FormatInfo
		(
			"R8ui",
			1,
			TEXTURE_BYTE | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			1,
			GL_UNSIGNED_BYTE,
			GL_RED_INTEGER,
			GL_R8UI
		),

		FormatInfo
		(
			"RG8",
			2,
			TEXTURE_BYTE | TEXTURE_RENDERABLE,
			2,
			GL_UNSIGNED_BYTE,
			GL_RG,
			GL_RG8
		),

		FormatInfo
		(
			"RG8s",
			2,
			TEXTURE_BYTE | TEXTURE_SIGNED,
			2,
			GL_BYTE,
			GL_RG,
			GL_RG8_SNORM
		),

		FormatInfo
		(
			"RG8i",
			2,
			TEXTURE_BYTE | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			2,
			GL_BYTE,
			GL_RG_INTEGER,
			GL_RG8I
		),

		FormatInfo
		(
			"RG8ui",
			2,
			TEXTURE_BYTE | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			2,
			GL_UNSIGNED_BYTE,
			GL_RG_INTEGER,
			GL_RG8UI
		),

		FormatInfo
		(
			"sRGBA8",
			4,
			TEXTURE_BYTE | TEXTURE_SRGB | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_BYTE,
			GL_RGBA,
			GL_SRGB8_ALPHA8
		),

		FormatInfo
		(
			"RGBA8",
			4,
			TEXTURE_BYTE | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_BYTE,
			GL_RGBA,
			GL_RGBA8
		),

		FormatInfo
		(
			"RGBA8s",
			4,
			TEXTURE_BYTE | TEXTURE_SIGNED,
			4,
			GL_BYTE,
			GL_RGBA,
			GL_RGBA8_SNORM
		),

		FormatInfo
		(
			"RGBA8i",
			4,
			TEXTURE_BYTE | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			4,
			GL_BYTE,
			GL_RGBA_INTEGER,
			GL_RGBA8I
		),

		FormatInfo
		(
			"RGBA8ui",
			4,
			TEXTURE_BYTE | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_BYTE,
			GL_RGBA_INTEGER,
			GL_RGBA8UI
		),

		// NOTE: plane 16-bit per channel

		FormatInfo
		(
			"R16",
			1,
			TEXTURE_SHORT | TEXTURE_RENDERABLE,
			2,
			GL_UNSIGNED_SHORT,
			GL_RED,
			GL_R16
		),

		FormatInfo
		(
			"R16s",
			1,
			TEXTURE_SHORT | TEXTURE_SIGNED,
			2,
			GL_SHORT,
			GL_RED,
			GL_R16_SNORM
		),

		FormatInfo
		(
			"R16i",
			1,
			TEXTURE_SHORT | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			2,
			GL_SHORT,
			GL_RED_INTEGER,
			GL_R16I
		),

		FormatInfo
		(
			"R16ui",
			1,
			TEXTURE_SHORT | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			2,
			GL_UNSIGNED_SHORT,
			GL_RED_INTEGER,
			GL_R16UI
		),

		FormatInfo
		(
			"R16f",
			1,
			TEXTURE_HALF | TEXTURE_RENDERABLE,
			2,
			GL_HALF_FLOAT,
			GL_RED,
			GL_R16F
		),

		FormatInfo
		(
			"RG16",
			2,
			TEXTURE_SHORT | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_SHORT,
			GL_RG,
			GL_RG16
		),

		FormatInfo
		(
			"RG16s",
			2,
			TEXTURE_SHORT | TEXTURE_SIGNED,
			4,
			GL_SHORT,
			GL_RG,
			GL_RG16_SNORM
		),

		FormatInfo
		(
			"RG16i",
			2,
			TEXTURE_SHORT | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			4,
			GL_SHORT,
			GL_RG_INTEGER,
			GL_RG16I
		),

		FormatInfo
		(
			"RG16ui",
			2,
			TEXTURE_SHORT | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_SHORT,
			GL_RG_INTEGER,
			GL_RG16UI
		),

		FormatInfo
		(
			"RG16f",
			2,
			TEXTURE_HALF | TEXTURE_RENDERABLE,
			4,
			GL_HALF_FLOAT,
			GL_RG,
			GL_RG16F
		),

		FormatInfo
		(
			"RGBA16",
			4,
			TEXTURE_SHORT | TEXTURE_RENDERABLE,
			8,
			GL_UNSIGNED_SHORT,
			GL_RGBA,
			GL_RGBA16
		),

		FormatInfo
		(
			"RGBA16s",
			4,
			TEXTURE_SHORT | TEXTURE_SIGNED,
			8,
			GL_SHORT,
			GL_RGBA,
			GL_RGBA16_SNORM
		),

		FormatInfo
		(
			"RGBA16i",
			4,
			TEXTURE_SHORT | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			8,
			GL_SHORT,
			GL_RGBA_INTEGER,
			GL_RGBA16I
		),

		FormatInfo
		(
			"RGBA16ui",
			4,
			TEXTURE_SHORT | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			8,
			GL_UNSIGNED_SHORT,
			GL_RGBA_INTEGER,
			GL_RGBA16UI
		),

		FormatInfo
		(
			"RGBA16f",
			4,
			TEXTURE_HALF | TEXTURE_RENDERABLE,
			8,
			GL_HALF_FLOAT,
			GL_RGBA,
			GL_RGBA16F
		),

		// NOTE: plane 32-bit per channel

		FormatInfo
		(
			"R32i",
			1,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			4,
			GL_INT,
			GL_RED_INTEGER,
			GL_R32I
		),

		FormatInfo
		(
			"R32ui",
			1,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT,
			GL_RED_INTEGER,
			GL_R32UI
		),

		FormatInfo
		(
			"R32f",
			1,
			TEXTURE_FLOAT | TEXTURE_RENDERABLE,
			4,
			GL_FLOAT,
			GL_RED,
			GL_R32F
		),

		FormatInfo
		(
			"RG32i",
			2,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			8,
			GL_INT,
			GL_RG_INTEGER,
			GL_RG32I
		),

		FormatInfo
		(
			"RG32ui",
			2,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			8,
			GL_UNSIGNED_INT,
			GL_RG_INTEGER,
			GL_RG32UI
		),

		FormatInfo
		(
			"RG32f",
			2,
			TEXTURE_FLOAT | TEXTURE_RENDERABLE,
			8,
			GL_FLOAT,
			GL_RG,
			GL_RG32F
		),

		FormatInfo
		(
			"RGB32i",
			3,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			12,
			GL_INT,
			GL_RGB_INTEGER,
			GL_RGB32I
		),

		FormatInfo
		(
			"RGB32ui",
			3,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			12,
			GL_UNSIGNED_INT,
			GL_RGB_INTEGER,
			GL_RGB32UI
		),

		FormatInfo
		(
			"RGB32f",
			3,
			TEXTURE_FLOAT | TEXTURE_RENDERABLE,
			12,
			GL_FLOAT,
			GL_RGB,
			GL_RGB32F
		),

		FormatInfo
		(
			"RGBA32i",
			4,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_SIGNED | TEXTURE_RENDERABLE,
			16,
			GL_INT,
			GL_RGBA_INTEGER,
			GL_RGBA32I
		),

		FormatInfo
		(
			"RGBA32ui",
			4,
			TEXTURE_INT | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			16,
			GL_UNSIGNED_INT,
			GL_RGBA_INTEGER,
			GL_RGBA32UI
		),

		FormatInfo
		(
			"RGBA32f",
			4,
			TEXTURE_FLOAT | TEXTURE_RENDERABLE,
			16,
			GL_FLOAT,
			GL_RGBA,
			GL_RGBA32F
		),

		// NOTE: depth-stencil

		FormatInfo
		(
			"DEPTH16",
			1,
			TEXTURE_DEPTH | TEXTURE_RENDERABLE,
			2,
			GL_UNSIGNED_SHORT,
			GL_DEPTH_COMPONENT,
			GL_DEPTH_COMPONENT16
		),

		FormatInfo
		(
			"DEPTH24",
			1,
			TEXTURE_DEPTH | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT,
			GL_DEPTH_COMPONENT,
			GL_DEPTH_COMPONENT24
		),

		FormatInfo
		(
			"DEPTH24_STENCIL8",
			2,
			TEXTURE_DEPTH | TEXTURE_STENCIL | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT_24_8,
			GL_DEPTH_STENCIL,
			GL_DEPTH24_STENCIL8
		),

		FormatInfo
		(
			"DEPTH32",
			1,
			TEXTURE_DEPTH | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT,
			GL_DEPTH_COMPONENT,
			GL_DEPTH_COMPONENT32
		),

		FormatInfo
		(
			"DEPTH32f",
			1,
			TEXTURE_DEPTH | TEXTURE_FLOAT | TEXTURE_RENDERABLE,
			4,
			GL_FLOAT,
			GL_DEPTH_COMPONENT,
			GL_DEPTH_COMPONENT32F
		),

		FormatInfo
		(
			"DEPTH32f_STENCIL8",
			2,
			TEXTURE_DEPTH | TEXTURE_FLOAT | TEXTURE_STENCIL | TEXTURE_RENDERABLE,
			8,
			GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
			GL_DEPTH_STENCIL,
			GL_DEPTH32F_STENCIL8
		),

		// NOTE: packed

		FormatInfo
		(
			"RGBA4444",
			4,
			TEXTURE_PACKED | TEXTURE_RENDERABLE,
			2,
			GL_UNSIGNED_SHORT_4_4_4_4,
			GL_RGBA,
			GL_RGBA4
		),

		FormatInfo
		(
			"RGBA5551",
			4,
			TEXTURE_PACKED | TEXTURE_RENDERABLE,
			2,
			GL_UNSIGNED_SHORT_5_5_5_1,
			GL_RGBA,
			GL_RGB5_A1
		),

		FormatInfo
		(
			"RGBA1010102",
			4,
			TEXTURE_PACKED | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT_2_10_10_10_REV,
			GL_RGBA,
			GL_RGB10_A2
		),

		FormatInfo
		(
			"RGBA1010102ui",
			4,
			TEXTURE_PACKED | TEXTURE_INTEGER | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT_2_10_10_10_REV,
			GL_RGBA,
			GL_RGB10_A2UI
		),

		FormatInfo
		(
			"RGB111110f",
			3,
			TEXTURE_PACKED_FLOAT | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT_10F_11F_11F_REV,
			GL_RGB,
			GL_R11F_G11F_B10F
		),

		FormatInfo
		(
			"RGB9e5",
			3,
			TEXTURE_PACKED_FLOAT | TEXTURE_RENDERABLE,
			4,
			GL_UNSIGNED_INT_5_9_9_9_REV,
			GL_RGB,
			GL_RGB9_E5
		),

		// NOTE: compressed

		FormatInfo
		(
			"R_bc4u",
			1,
			TEXTURE_BC4,
			8,
			GL_NONE,
			GL_RED,
			GL_COMPRESSED_RED_RGTC1
		),

		FormatInfo
		(
			"R_bc4s",
			1,
			TEXTURE_BC4 | TEXTURE_SIGNED,
			8,
			GL_NONE,
			GL_RED,
			GL_COMPRESSED_SIGNED_RED_RGTC1
		),

		FormatInfo
		(
			"RG_bc5u",
			2,
			TEXTURE_BC5,
			16,
			GL_NONE,
			GL_RG,
			GL_COMPRESSED_RG_RGTC2
		),

		FormatInfo
		(
			"RG_bc5s",
			2,
			TEXTURE_BC5 | TEXTURE_SIGNED,
			16,
			GL_NONE,
			GL_RG,
			GL_COMPRESSED_SIGNED_RG_RGTC2
		),

		FormatInfo
		(
			"RGB_bc6u",
			3,
			TEXTURE_BC6,
			16,
			GL_NONE,
			GL_RGB,
			GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
		),

		FormatInfo
		(
			"RGB_bc6s",
			3,
			TEXTURE_BC6 | TEXTURE_SIGNED,
			16,
			GL_NONE,
			GL_RGB,
			GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
		),

		FormatInfo
		(
			"RGBA_bc1",
			4,
			TEXTURE_BC1,
			8,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		),

		FormatInfo
		(
			"sRGBA_bc1",
			4,
			TEXTURE_BC1 | TEXTURE_SRGB,
			8,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
		),

		FormatInfo
		(
			"RGBA_bc2",
			4,
			TEXTURE_BC2,
			16,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
		),

		FormatInfo
		(
			"sRGBA_bc2",
			4,
			TEXTURE_BC2 | TEXTURE_SRGB,
			16,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
		),

		FormatInfo
		(
			"RGBA_bc3",
			4,
			TEXTURE_BC3,
			16,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
		),

		FormatInfo
		(
			"sRGBA_bc3",
			4,
			TEXTURE_BC3 | TEXTURE_SRGB,
			16,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
		),

		FormatInfo
		(
			"RGBA_bc7",
			4,
			TEXTURE_BC7,
			16,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_RGBA_BPTC_UNORM
		),

		FormatInfo
		(
			"sRGBA_bc7",
			4,
			TEXTURE_BC7 | TEXTURE_SRGB,
			16,
			GL_NONE,
			GL_RGBA,
			GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
		),

        FormatInfo
        (
            "RGB8",
            3,
            TEXTURE_BYTE | TEXTURE_RENDERABLE,
            3,
            GL_UNSIGNED_BYTE,
            GL_RGB,
            GL_RGB8
        ),

        // the following two were not tested

        FormatInfo
        (
            "BGR8",
            3,
            TEXTURE_BYTE | TEXTURE_RENDERABLE,
            3,
            GL_UNSIGNED_BYTE,
            GL_BGR,
            GL_RGB8
        ),

        FormatInfo
        (
            "BGRA8",
            4,
            TEXTURE_BYTE | TEXTURE_RENDERABLE,
            4,
            GL_UNSIGNED_BYTE,
            GL_BGRA,
            GL_RGBA8
        ),
	};

	//===========================================================================================

	void FreePredefinedStateObjects()
	{
		for( unsigned i = 0; i < PREDEFINED_STATE_OBJECTS_NUM; i++ )
			g_predefinedStateObjects[i].Free();
	}

	void SetupPredefinedStateObjects()
	{
		//StateObject* pStateObject = g_predefinedStateObjects + ClampNearest;

		//pStateObject->m_filterMin	= Nearest;
		//pStateObject->m_filterMag	= Nearest;
		//pStateObject->m_wrap[0]		= ClampToEdge;
		//pStateObject->m_wrap[1]		= ClampToEdge;
		//pStateObject->m_wrap[2]		= ClampToEdge;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + RepeatNearest;

		//pStateObject->m_filterMin	= Nearest;
		//pStateObject->m_filterMag	= Nearest;
		//pStateObject->m_wrap[0]		= Repeat;
		//pStateObject->m_wrap[1]		= Repeat;
		//pStateObject->m_wrap[2]		= Repeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + MirroredRepeatNearest;

		//pStateObject->m_filterMin	= Nearest;
		//pStateObject->m_filterMag	= Nearest;
		//pStateObject->m_wrap[0]		= MirroredRepeat;
		//pStateObject->m_wrap[1]		= MirroredRepeat;
		//pStateObject->m_wrap[2]		= MirroredRepeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + ClampNearestMipmapNearest;

		//pStateObject->m_filterMin	= NearestMipmapNearest;
		//pStateObject->m_filterMag	= Nearest;
		//pStateObject->m_wrap[0]		= ClampToEdge;
		//pStateObject->m_wrap[1]		= ClampToEdge;
		//pStateObject->m_wrap[2]		= ClampToEdge;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + RepeatNearestMipmapNearest;

		//pStateObject->m_filterMin	= NearestMipmapNearest;
		//pStateObject->m_filterMag	= Nearest;
		//pStateObject->m_wrap[0]		= Repeat;
		//pStateObject->m_wrap[1]		= Repeat;
		//pStateObject->m_wrap[2]		= Repeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + MirroredRepeatNearestMipmapNearest;

		//pStateObject->m_filterMin	= NearestMipmapNearest;
		//pStateObject->m_filterMag	= Nearest;
		//pStateObject->m_wrap[0]		= MirroredRepeat;
		//pStateObject->m_wrap[1]		= MirroredRepeat;
		//pStateObject->m_wrap[2]		= MirroredRepeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + ClampLinear;

		//pStateObject->m_filterMin	= Linear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= ClampToEdge;
		//pStateObject->m_wrap[1]		= ClampToEdge;
		//pStateObject->m_wrap[2]		= ClampToEdge;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + RepeatLinear;

		//pStateObject->m_filterMin	= Linear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= Repeat;
		//pStateObject->m_wrap[1]		= Repeat;
		//pStateObject->m_wrap[2]		= Repeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + MirroredRepeatLinear;

		//pStateObject->m_filterMin	= Linear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= MirroredRepeat;
		//pStateObject->m_wrap[1]		= MirroredRepeat;
		//pStateObject->m_wrap[2]		= MirroredRepeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + ClampLinearMipmapNearest;

		//pStateObject->m_filterMin	= LinearMipmapNearest;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= ClampToEdge;
		//pStateObject->m_wrap[1]		= ClampToEdge;
		//pStateObject->m_wrap[2]		= ClampToEdge;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + RepeatLinearMipmapNearest;

		//pStateObject->m_filterMin	= LinearMipmapNearest;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= Repeat;
		//pStateObject->m_wrap[1]		= Repeat;
		//pStateObject->m_wrap[2]		= Repeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + MirroredRepeatLinearMipmapNearest;

		//pStateObject->m_filterMin	= LinearMipmapNearest;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= MirroredRepeat;
		//pStateObject->m_wrap[1]		= MirroredRepeat;
		//pStateObject->m_wrap[2]		= MirroredRepeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + ClampLinearMipmapLinear;

		//pStateObject->m_filterMin	= LinearMipmapLinear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= ClampToEdge;
		//pStateObject->m_wrap[1]		= ClampToEdge;
		//pStateObject->m_wrap[2]		= ClampToEdge;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + RepeatLinearMipmapLinearAnisotropy;

		//pStateObject->m_filterMin	= LinearMipmapLinear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= Repeat;
		//pStateObject->m_wrap[1]		= Repeat;
		//pStateObject->m_wrap[2]		= Repeat;
		//pStateObject->m_anisotropy	= 16.0f;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + RepeatLinearMipmapLinear;

		//pStateObject->m_filterMin	= LinearMipmapLinear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= Repeat;
		//pStateObject->m_wrap[1]		= Repeat;
		//pStateObject->m_wrap[2]		= Repeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + MirroredRepeatLinearMipmapLinear;

		//pStateObject->m_filterMin	= LinearMipmapLinear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= MirroredRepeat;
		//pStateObject->m_wrap[1]		= MirroredRepeat;
		//pStateObject->m_wrap[2]		= MirroredRepeat;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + ClampLinearShadow;

		//pStateObject->m_filterMin	= Linear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= ClampToEdge;
		//pStateObject->m_wrap[1]		= ClampToEdge;
		//pStateObject->m_wrap[2]		= ClampToEdge;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;
		//pStateObject->m_compareFunc	= GL_LESS;

		//pStateObject->Setup();

		////=================================================================================

		//pStateObject = g_predefinedStateObjects + ClampLinearSkipSrgb;

		//pStateObject->m_filterMin	= Linear;
		//pStateObject->m_filterMag	= Linear;
		//pStateObject->m_wrap[0]		= ClampToEdge;
		//pStateObject->m_wrap[1]		= ClampToEdge;
		//pStateObject->m_wrap[2]		= ClampToEdge;
		//pStateObject->m_anisotropy	= NO_ANISOTROPY;
		//pStateObject->SetState(SAMPLER_SRGB_SKIP_DECODE);

		//pStateObject->Setup();
	}

	//===========================================================================================

	const FormatInfo* GetFormatInfo(Format format)
	{
		return g_samplerFormatInfo + format;
	}

	Format FindFormat(Format baseFormat, unsigned flagsToRemove, unsigned flagsToAdd, unsigned channels)
	{
		const FormatInfo* pFormatInfo = GetFormatInfo(baseFormat);

		unsigned flags = pFormatInfo->Flags(TEXTURE_ALL);
		flags &= ~flagsToRemove;
		flags |= flagsToAdd;

		if( channels == TR_UNKNOWN )
			channels = pFormatInfo->Channels();

		for( unsigned i = 0; i < FORMATS_NUM; i++ )
		{
			pFormatInfo = GetFormatInfo((Format)i);

			if( pFormatInfo->Flags(TEXTURE_ALL) == flags && pFormatInfo->Channels() == channels )
				return (Format)i;
		}

		return FORMAT_NONE;
	}

	//============================================================================================

	//void ModifyRect(unsigned& x, unsigned& y, unsigned& w, unsigned& h)
	//{
	//	const TR_ViewPort* pVp = g_pRenderer->ViewportCurrentPtr();

	//	if( x == USE_VIEWPORT_POS )
	//		x = pVp->x;

	//	if( y == USE_VIEWPORT_POS )
	//		y = pVp->y;

	//	if( w == USE_VIEWPORT_SIZE )
	//		w = pVp->w;
	//
	//	if( h == USE_VIEWPORT_SIZE )
	//		h = pVp->h;
	//}

	//=================================================================================================

	//void Object::Copy
	//(
	//	const Object* pSrc, const Object* pDst,
	//	unsigned uiSrcLevel, unsigned uiSrcX, unsigned uiSrcY, unsigned uiSrcZ, 
	//	unsigned uiDstLevel, unsigned uiDstX, unsigned uiDstY, unsigned uiDstZ, 
	//	unsigned uiWidth, unsigned uiHeight, unsigned uiDepth
	//)
	//{
	//	assert( uiSrcLevel < pSrc->Mipmaps() );

	//	if( !uiWidth )
	//		uiWidth = pSrc->Width(uiSrcLevel);

	//	if( !uiHeight )
	//		uiHeight = pSrc->Height(uiSrcLevel);

	//	if( !uiDepth )
	//		uiDepth = pSrc->Depth(uiSrcLevel);

	//	assert( uiSrcX + uiWidth <= pSrc->Width(uiSrcLevel) );
	//	assert( uiSrcY + uiHeight <= pSrc->Height(uiSrcLevel) );
	//	assert( uiSrcZ + uiDepth <= pSrc->Depth(uiSrcLevel) );

	//	assert( uiDstLevel < pDst->Mipmaps() );
	//	assert( uiDstX + uiWidth <= pDst->Width(uiDstLevel) );
	//	assert( uiDstY + uiHeight <= pDst->Height(uiDstLevel) );
	//	assert( uiDstZ + uiDepth <= pDst->Depth(uiDstLevel) );

	//	glCopyImageSubData
	//	(
	//		pSrc->m_glTexture, pSrc->GetTarget(), uiSrcLevel, uiSrcX, uiSrcY, uiSrcZ,
	//		pDst->m_glTexture, pDst->GetTarget(), uiDstLevel, uiDstX, uiDstY, uiDstZ,
	//		uiWidth, uiHeight, uiDepth
	//	);
	//}

	//=================================================================================================

	unsigned Object::_GetMipmapsFromDimensions() const
	{
		unsigned maxSize = TR_MAX(m_uiWidth, m_uiHeight);

		if( !IsArrayTarget() )
			maxSize = TR_MAX(maxSize, m_uiDepth);
	
		unsigned mipmapsNeeded = 0;

		while( maxSize > 0 )
		{
			maxSize >>= 1;
			mipmapsNeeded++;
		}

		return mipmapsNeeded;
	}

	//============================================================================================

	void Object::_Zero()
	{
		m_format			= FORMAT_NONE;
		m_pucPixels			= NULL;
		m_uiWidth			= 0;
		m_uiHeight			= 0;
		m_uiDepth			= 0;
		m_uiMipmaps			= 0;
		m_hashName			= 0;
        m_bLabelled = false;
		m_iRef				= 0;
		m_uiSize_cpu		= 0;
		m_uiSize_gpu		= 0;

		SetAllState(0);

		m_uiCurrentBatch	= 0;
		m_uiBatchTick		= 0;
	
		m_glTexture			= 0;
		m_target			= Texture2d;
		m_pStateObject		= NULL;

		m_uiSwizzle			= MAKE_FOURCC('R', 'G', 'B', 'A');

		// FIXME: ???

		static unsigned s_uiId = 0;

		sprintf(m_pczFilename, "Unnamed%u", ++s_uiId);
	}

	//=============================================================================================

	void Object::_FreeMemory()
	{
		if( m_pucPixels )
		{
			if( IsStateSet(READ_ONLY) )
				m_pucPixels = NULL;
			else
				MEM_Free(TR_BYTE, m_pucPixels);
		}

		m_uiSize_cpu = 0;

		ClearState(BATCHED);
	}

    void Object::_ApplyLabel()
    {
        if (m_bLabelled)
            return;

        char const *label = GetFilename();
        if (label && *label == '\0')
            label = nullptr;

#if 0
	    glObjectLabel(GL_TEXTURE, m_glTexture, -1, label);
#endif
	    
	    m_bLabelled = true;
    }

    //=============================================================================================

	unsigned Object::PixelCount(unsigned firstMipmap, unsigned mipmaps) const
	{
		unsigned n = 0;

		if( mipmaps == AVAILABLE_MIPMAPS )
			mipmaps = m_uiMipmaps;

		if( firstMipmap < mipmaps )
		{
			do
			{
				unsigned w = Width(firstMipmap);
				unsigned h = Height(firstMipmap);
				unsigned d = Depth(firstMipmap);

				n += w * h * d;
			}
			while( ++firstMipmap < mipmaps );
		}

		return n;
	}

	//=============================================================================================

	unsigned Object::MipmappedSize(unsigned firstMipmap, unsigned mipmaps, Format format, unsigned w, unsigned h, unsigned d) const
	{
		if( format == FORMAT_NONE )
			format = m_format;

		if( mipmaps == AVAILABLE_MIPMAPS )
			mipmaps = m_uiMipmaps;

		if( !w )
			w = Width(firstMipmap);

		if( !h )
			h = Height(firstMipmap);

		if( !d )
			d = Depth(firstMipmap);

		const FormatInfo* pFormatInfo = GetFormatInfo(format);
		bool bCompressed = pFormatInfo->IsCompressed();
		
		unsigned size = 0;

		while( mipmaps )
		{
			// FIXME: what about 3D compression?

			if( bCompressed )
				size += ((w + 3) >> 2) * ((h + 3) >> 2) * d;
			else
				size += w * h * d;

			if( !Is3DTarget() )
			{
				if( w == 1 && h == 1 )
					break;
			}
			else
			{
				if( w == 1 && h == 1 && d == 1 )
					break;

				if( d > 1 )
					d >>= 1;
			}

			if( w > 1 )
				w >>= 1;

			if( h > 1 )
				h >>= 1;

			mipmaps--;
		}

		size *= pFormatInfo->Bytes();

		return size;
	}

	//=============================================================================================

	void Object::Create(Target target, Format format, TR_BYTE* pData, unsigned w, unsigned h, unsigned d, unsigned mipmaps, unsigned flags, char const *label /*= nullptr*/)
	{
		_FreeMemory();
		Unload();
		_Zero();

		SetAllState(flags);

		m_target		= target;
		m_format		= format;
		m_uiWidth		= w;
		m_uiHeight		= h;
		m_uiDepth		= d;
		m_uiMipmaps		= TR_MIN(mipmaps, _GetMipmapsFromDimensions());
		m_uiSize_cpu	= MipmappedSize();

		if( IsStateSet(READ_ONLY) )
			m_pucPixels = pData;
		else
		{
			unsigned size = MipmappedSize();

			m_pucPixels = MEM_Alloc(TR_BYTE, size);

			if( pData )
				memcpy(m_pucPixels, pData, size);
			else
				memset(m_pucPixels, 0, size);
		}

        if (label)
	        SetFilename(label);
	}

	//=============================================================================================

	TR_BYTE* Object::Pixels(unsigned mipmap) const
	{
		return (mipmap < m_uiMipmaps && m_pucPixels) ? (m_pucPixels + (mipmap ? MipmappedSize(0, mipmap) : 0)) : NULL;
	}

	//=============================================================================================

	//bool Object::_ReadFileRAW(const char* filename)
	//{
	//	void* file = g_OpenFile(filename, _O_RDONLY | _O_BINARY);

	//	if( !file )
	//	{
	//		TR_PRINT(MSG_ERROR, "Sampler::Object::ReadFileRAW(\"%s\"): Can't open file!", filename);
	//		return false;
	//	}

	//	unsigned size = MipmappedSize();

	//	if( size != g_LengthFile(file) )
	//	{
	//		TR_PRINT(MSG_ERROR, "Sampler::Object::ReadFileRAW(\"%s\"): Incorrect file size!", filename);
	//		return false;
	//	}

	//	m_pucPixels = MEM_Alloc(TR_BYTE, size);

	//	g_ReadFile(file, m_pucPixels, size);
	//	g_CloseFile(file);

	//	return true;
	//}
			
	//=============================================================================================

	//bool Object::SaveFileRAW(const char* filename) const
	//{
	//	if( !IsValid(BATCHED) )
	//		return false;

	//	void* file = g_CreateFile(filename, _O_BINARY);

	//	if( !file )
	//	{
	//		TR_PRINT(MSG_ERROR, "Sampler::Object::SaveFileRAW(\"%s\"): Can't create file!", filename);
	//		return false;
	//	}

	//	unsigned size = MipmappedSize();

	//	g_WriteFile(file, m_pucPixels, size);
	//	g_CloseFile(file);

	//	return true;
	//}

	////=============================================================================================

	//bool Object::LoadFile(const char* filename, unsigned uiFlags)
	//{
	//	const char* scope = "Sampler::Object::LoadFile(\"";
	//	const char* ext = strrchr(filename, '.');

	//	if( !ext )
	//	{
	//		TR_PRINT(MSG_ERROR, "%s\"%s\"): No extension!", scope, filename);
	//		return false;
	//	}

	//	ext++;

	//	_FreeMemory();
	//	Unload();

	//	bool bResult = false;

	//	if( !_stricmp(ext, "raw") )
	//		bResult = _ReadFileRAW(filename);
	//	else
	//	{
	//		_Zero();

	//		if( !_stricmp(ext, "bmp") )
	//			bResult = _ReadFileBMP(filename, (uiFlags & SKIP_BMP_PALETTE) != 0);
	//		else if( !_stricmp(ext, "btx") )
	//			bResult = _ReadFileBTX(filename);
 //           else if(!_stricmp(ext, "dds"))
 //               bResult = _ReadFileDDS(filename);
	//		else
	//			TR_PRINT(MSG_ERROR, "%s%s): \"%s\" unknown file format!", scope, filename, ext);
	//	}

	//	if( bResult )
	//	{
	//		SetFilename(filename);

	//		// FIXME: do this better?

	//		//UpdateHash();
	//		m_uiSize_cpu = MipmappedSize();
	//	}
	//	else
	//	{
	//		// NOTE: reset values to defaults

	//		_FreeMemory();

	//		m_uiWidth		= 1;
	//		m_uiHeight		= 1;
	//		m_uiDepth		= 1;
	//		m_format		= FORMAT_NONE;
	//		m_uiMipmaps		= 1;
	//	}

	//	return bResult;
	//}

	////=============================================================================================

	//bool Object::SaveFileBTX(const char* pczFilename) const
	//{
	//	const char* pczScope = "Sampler::Object::SaveFileBTX(\"";

	//	if( !IsValid() )
	//		return false;

	//	void* file = g_CreateFile(pczFilename, _O_BINARY);

	//	if( !file )
	//	{
	//		TR_PRINT(MSG_ERROR, "%s%s\"): Can't create file!", pczScope, pczFilename);
	//		return false;
	//	}

	//	// NOTE: write header

	//	sBTXHeader header;
	//	memset(&header, 0, sizeof(header));

	//	header.uiMagic		= BTX_MAGIC;
	//	header.uiSwizzle	= m_uiSwizzle;
	//	header.uiDataSize	= MipmappedSize();
	//	header.usTarget		= m_target;
	//	header.usFormat		= m_format;
	//	header.usWidth		= m_uiWidth;
	//	header.usHeight		= m_uiHeight;
	//	header.usDepth		= m_uiDepth;
	//	header.usMipmaps	= m_uiMipmaps;

	//	if( IsStateSet(BATCHED) )
	//	{
	//		const sBTXBatch* pBatch = (const sBTXBatch*)m_pucPixels;
	//
	//		for(;;)
	//		{
	//			header.usBatchesNum++;

	//			if( pBatch->ucAction == BTX_TEX_COMPLETE )
	//				break;

	//			pBatch++;
	//		}
	//	}

	//	unsigned uiPixelsOffset = header.usBatchesNum * sizeof(sBTXBatch);
	//	unsigned char* pPixels = m_pucPixels + uiPixelsOffset;

	//	header.uiHash = COM_BlockChecksum(pPixels, header.uiDataSize);

	//	g_WriteFile(file, &header, sizeof(header));

	//	// NOTE: write batches

	//	if( IsStateSet(BATCHED) )
	//	{
	//		const sBTXBatch* pBatch = (const sBTXBatch*)m_pucPixels;

	//		for(;;)
	//		{
	//			g_WriteFile(file, pBatch, sizeof(sBTXBatch));

	//			if( pBatch->ucAction == BTX_TEX_COMPLETE )
	//				break;

	//			pBatch++;
	//		}
	//	}

	//	// NOTE: write data

	//	g_WriteFile(file, pPixels, header.uiDataSize);
	//	g_CloseFile(file);

	//	return true;
	//}

	////=============================================================================================

	//bool Object::SaveFilePNG(const char* pczFilename) const
	//{
	//	const char* pczScope = "Sampler::Object::SaveFilePNG(\"";
	//	const FormatInfo* pFormatInfo = GetFormatInfo(m_format);

	//	if( !IsValid(BATCHED) )
	//	{
	//		g_pConsole->Print(MSG_WARNING, "%s%s\"): Batched textures can't be save!\n", pczScope, pczFilename);
	//		return false;
	//	}

	//	if( !pFormatInfo->IsPlain() || pFormatInfo->Flags(TEXTURE_FLOAT | TEXTURE_HALF) || m_uiMipmaps > 1 || m_uiDepth > 1 )
	//	{
	//		g_pConsole->Print(MSG_WARNING, "%s%s\"): 2D plain non-float image without mipmaps is required!\n", pczScope, pczFilename);
	//		return false;
	//	}

	//	void* file = g_CreateFile(pczFilename, _O_BINARY);

	//	if( !file )
	//	{
	//		TR_PRINT(MSG_ERROR, "%s%s\"): Can't open file!", pczScope, pczFilename);
	//		return false;
	//	}

	//	size_t totalSize = 0;
	//	void* pBuffer = tdefl_write_image_to_png_file_in_memory(m_pucPixels, m_uiWidth, m_uiHeight, pFormatInfo->Channels(), pFormatInfo->BytesPerChannel(), &totalSize);

	//	g_WriteFile(file, pBuffer, totalSize);
	//	g_CloseFile(file);

	//	mz_free(pBuffer);

	//	return true;
	//}

	////=============================================================================================

	//bool Object::_ReadFileBTX(const char* pczFilename)
	//{
	//	const char* pczScope = "Sampler::Object::_ReadFileBTX(\"";

	//	void* file = g_OpenFile(pczFilename, _O_RDONLY|_O_BINARY);

	//	if( !file )
	//	{
	//		TR_PRINT(MSG_ERROR, "%s%s\"): Can't open file!", pczScope, pczFilename);
	//		return false;
	//	}

	//	sBTXHeader header;
	//	g_ReadFile(file, &header, sizeof(header));

	//	if( header.uiMagic != BTX_MAGIC )
	//	{
	//		TR_PRINT(MSG_ERROR, "%s%s\"): This is not a BTX file!\n", pczScope, pczFilename);
	//		g_CloseFile(file);

	//		return false;
	//	}

	//	// NOTE: setup

	//	m_uiSwizzle			= header.uiSwizzle;
	//	m_target			= (Sampler::Target)header.usTarget;
	//	m_format			= (Sampler::Format)header.usFormat;
	//	m_uiWidth			= header.usWidth;
	//	m_uiHeight			= header.usHeight;
	//	m_uiDepth			= header.usDepth;
	//	m_uiMipmaps			= header.usMipmaps;
	//	m_uiCurrentBatch	= 0;

	//	// NOTE: read data

	//	unsigned uiTotalSize = header.usBatchesNum * sizeof(sBTXBatch) + header.uiDataSize;

	//	m_pucPixels = MEM_Alloc(TR_BYTE, uiTotalSize);

	//	if( header.usBatchesNum > 0 )
	//		SetState(BATCHED);

	//	g_ReadFile(file, m_pucPixels, uiTotalSize);
	//	g_CloseFile(file);

	//	return true;
	//}

 //   bool Object::_ReadFileDDS(const char * filename)
 //   {
 //       // DDS loading adapted from https://github.com/paroj/nv_dds/blob/master/nv_dds.cpp
 //       // License: http://developer.download.nvidia.com/licenses/general_license.txt

 //       const char* pczScope = "Sampler::Object::_ReadFileDDS(\"";

 //       scoped_file file(filename, _O_RDONLY | _O_BINARY);

 //       if(!file)
 //       {
 //           TR_PRINT(MSG_ERROR, "%s%s\"): Can't open file!", pczScope, filename);
 //           return false;
 //       }

 //       char filecode[4];
 //       file.read(&filecode, sizeof(filecode));

 //       if(strncmp(filecode, "DDS ", 4) != 0)
 //       {
 //           TR_PRINT(MSG_ERROR, "%s%s\"): This is not a DDS file!\n", pczScope, filename);
 //           return false;
 //       }

 //       // read in DDS header
 //       DirectX::DDS_HEADER ddsh;
 //       file.read(&ddsh, sizeof(DirectX::DDS_HEADER));

 //       uint32_t real_depth = ddsh.depth;
 //       uint32_t array_size = 1;

 //       // figure out what the image format is
 //       if(ddsh.ddspf.flags & DDS_constants::DDSF_FOURCC)
 //       {
 //           switch(ddsh.ddspf.fourCC) {
 //           case DDS_constants::FOURCC_DXT1:
 //               m_format = Format::RGBA_bc1;
 //               break;
 //           case DDS_constants::FOURCC_DXT3:
 //               m_format = Format::RGBA_bc2;
 //               break;
 //           case DDS_constants::FOURCC_DXT5:
 //               m_format = Format::RGBA_bc3;
 //               break;
 //           case MAKEFOURCC('D','X','1','0'):
 //               {
 //                   DirectX::DDS_HEADER_DXT10 header_dx10;
 //                   file.read(&header_dx10, sizeof(header_dx10));

 //                   static const std::map<DXGI_FORMAT, Format> dxgi_format_mapping = 
 //                   {
 //                       {DXGI_FORMAT_R8G8B8A8_UNORM, RGBA8},
 //                   };

 //                   auto const dxgi_it = dxgi_format_mapping.find(header_dx10.dxgiFormat);
 //                   if (dxgi_it == dxgi_format_mapping.end())
 //                   {
 //                       TR_PRINT(MSG_ERROR, "%s%s\"): unknown texture dxgi format '%s'\n", pczScope, filename, header_dx10.dxgiFormat);
 //                       return false;
 //                   }

 //                   m_format = dxgi_it->second;

 //                   array_size = real_depth = header_dx10.arraySize;
 //               }

 //               break;
 //           default:
 //               TR_PRINT(MSG_ERROR, "%s%s\"): unknown texture compression '%s'\n", pczScope, filename, fourcc(ddsh.ddspf.fourCC));
 //               return false;
 //           }
 //       }
 //       else if(ddsh.ddspf.RGBBitCount == 32 &&
 //               ddsh.ddspf.RBitMask == 0x00FF0000 &&
 //               ddsh.ddspf.GBitMask == 0x0000FF00 &&
 //               ddsh.ddspf.BBitMask == 0x000000FF &&
 //               ddsh.ddspf.ABitMask == 0xFF000000) 
 //       {
 //           m_format = Format::BGRA8;
 //       }
 //       else if(ddsh.ddspf.RGBBitCount == 32 &&
 //               ddsh.ddspf.RBitMask == 0x000000FF &&
 //               ddsh.ddspf.GBitMask == 0x0000FF00 &&
 //               ddsh.ddspf.BBitMask == 0x00FF0000 &&
 //               ddsh.ddspf.ABitMask == 0xFF000000) 
 //       {
 //           m_format = Format::RGBA8;
 //       }
 //       else if(ddsh.ddspf.RGBBitCount == 24 &&
 //               ddsh.ddspf.RBitMask == 0x000000FF &&
 //               ddsh.ddspf.GBitMask == 0x0000FF00 &&
 //               ddsh.ddspf.BBitMask == 0x00FF0000) 
 //       {
 //           m_format = Format::RGB8;
 //       }
 //       else if(ddsh.ddspf.RGBBitCount == 24 &&
 //               ddsh.ddspf.RBitMask == 0x00FF0000 &&
 //               ddsh.ddspf.GBitMask == 0x0000FF00 &&
 //               ddsh.ddspf.BBitMask == 0x000000FF) 
 //       {
 //           m_format = Format::BGR8;
 //       }
 //       else 
 //       {
 //           TR_PRINT(MSG_ERROR, "%s%s\"): unknown DDS texture format\n", pczScope, filename);
 //           return false;
 //       }

 //       auto clamp_size = [](unsigned size)
 //       {
 //           return size <= 0 ? 1 : size;
 //       };

 //       // default to flat texture type (1D, 2D, or rectangle)
 //       m_target = (ddsh.height > 1 && ddsh.width > 1) ? Target::Texture2d : Target::Texture1d;

 //       assert(ddsh.height >= 1 || ddsh.width >= 1);
	//    bool const is_1d = ddsh.height == 1 || ddsh.width == 1;

 //       if (array_size > 1)
 //       {
 //           assert(!is_1d);
 //           m_target = Target::Texture2dArray;
 //       }
 //       else
 //       {
 //           m_target = is_1d ? Target::Texture1d : Target::Texture2d;
 //       }
 //       

 //       // check if image is a cubemap
 //       if(ddsh.caps2 & DDS_constants::DDSF_CUBEMAP)
 //       {
 //           assert(array_size == 1);
 //           m_target = Target::TextureCubeMap;
 //       }

 //       // check if image is a volume texture
 //       if((ddsh.caps2 & DDS_constants::DDSF_VOLUME) && (ddsh.depth > 0))
 //       {
 //           assert(array_size == 1);
 //           m_target = Target::Texture3d;
 //       }

 //       // store primary surface width/height/depth
 //       m_uiWidth = ddsh.width;
 //       m_uiHeight = ddsh.height;
 //       m_uiDepth = clamp_size(real_depth);

 //       // store number of mipmaps
 //       m_uiMipmaps = clamp_size(ddsh.mipMapCount);

 //       // load surface
 //       unsigned whole_size = MipmappedSize();
 //       m_pucPixels = MEM_Alloc(TR_BYTE, whole_size);

 //       TR_BYTE* current_data = m_pucPixels;

 //       // probably will crash anyway
 //       assert(m_target != Target::TextureCubeMap);

 //       // load all surfaces for the image (6 surfaces for cubemaps)
 //       for(unsigned n = 0; n < (m_target == Target::TextureCubeMap ? 6u : 1u); n++) 
 //       {

 //           // read main image
 //           unsigned size = MipmappedSize(0, 1);
 //           file.read(current_data, size);
 //           current_data += size;

 //           // load all mipmaps for current surface
 //           for(unsigned i = 1; i < m_uiMipmaps; i++) 
 //           {

 //               // read mipmaps
 //               size = MipmappedSize(i, 1);
 //               file.read(current_data, size);
 //               current_data += size;
 //           }

 //           int offset = m_pucPixels + whole_size - current_data;
 //           assert(offset == 0);
 //       }

 //       return true;
 //   }

	////=============================================================================================

	//bool Object::_ReadFileBMP(const char* filename, bool skipPalette)
	//{
	//	BITMAPFILEHEADER fileHeader;
	//	BITMAPINFOHEADER header;

	//	void* file = g_OpenFile(filename, _O_RDONLY|_O_BINARY);

	//	if( !file )
	//	{
	//		TR_PRINT(MSG_ERROR, "Sampler::Object::_ReadFileBMP(\"%s\"): Can't open file!", filename);
	//		return false;
	//	}

	//	g_ReadFile(file, &fileHeader, sizeof(fileHeader));

	//	if( fileHeader.bfType != ((WORD)('M' << 8) | 'B') )
	//	{
	//		g_CloseFile(file);
	//		TR_PRINT(MSG_ERROR, "Sampler::Object::_ReadFileBMP(\"%s\"): Invalid BMP file!", filename);
	//		return false;
	//	}

	//	g_ReadFile(file, &header, sizeof(header));

	//	if( !header.biSizeImage )
	//		header.biSizeImage = header.biHeight * header.biWidth * (header.biBitCount >> 3);

	//	if( header.biPlanes != 1 || (header.biBitCount != 24 && header.biBitCount != 32 && header.biBitCount != 8) )
	//	{
	//		g_CloseFile(file);
	//		TR_PRINT(MSG_ERROR, "Sampler::Object::_ReadFileBMP(\"%s\"): Only 8, 24 or 32 bit images supported!", filename);
	//		return false;
	//	}

	//	if( header.biCompression )
	//	{
	//		g_CloseFile(file);
	//		TR_PRINT(MSG_ERROR, "Sampler::Object::_ReadFileBMP(\"%s\"): Only uncompressed BMP files supported!", filename);
	//		return false;
	//	}

	//	unsigned lineSize = (header.biWidth * (header.biBitCount >> 3) + 3) & (~3);

	//	m_uiWidth	= header.biWidth;
	//	m_uiHeight	= header.biHeight;
	//	m_uiDepth	= 1;
	//	m_uiMipmaps	= 1;
	//	m_target	= Texture2d;
	//	m_format	= (header.biBitCount == 8 && skipPalette) ? R8 : RGBA8;

	//	unsigned channels = GetFormatInfo(m_format)->Channels();
	//	unsigned stride = m_uiWidth * channels;

	//	TR_BYTE* tmp = MEM_Alloc(TR_BYTE, lineSize);
	//	m_pucPixels = MEM_Alloc(TR_BYTE, stride * m_uiHeight);

	//	TR_BYTE palette[1024];

	//	if( header.biBitCount == 8 )
	//		g_ReadFile(file, palette, sizeof(palette));

	//	TR_BYTE* dest = m_pucPixels;

	//	for( unsigned i = 0; i < m_uiHeight; i++ )
	//	{
	//		g_ReadFile(file, tmp, lineSize);

	//		for( unsigned j = 0; j < m_uiWidth; j++ )
	//		{
	//			if( header.biBitCount == 8 )
	//			{
	//				if( !skipPalette )
	//				{
	//					TR_BYTE* src = palette + (tmp[j] << 2);

	//					*dest++ = src[2];
	//					*dest++ = src[1];
	//					*dest++ = src[0];
	//				}
	//				else
	//					*dest++ = tmp[j];
	//			}
	//			else if( header.biBitCount == 24 )
	//			{
	//				uint n = j * 3;

	//				*dest++ = tmp[n + 2];
	//				*dest++ = tmp[n + 1];
	//				*dest++ = tmp[n + 0];
	//				*dest++ = 255;
	//			}
	//			else
	//			{
	//				uint n = j * 4;

	//				*dest++ = tmp[n + 2];
	//				*dest++ = tmp[n + 1];
	//				*dest++ = tmp[n + 0];
	//				*dest++ = tmp[n + 3];
	//			}
	//		}
	//	}

	//	MEM_Free(TR_BYTE, tmp);

	//	g_CloseFile(file);

	//	return true;
	//}

	//=============================================================================================

	// Generic mipmap generation function. Works with 1D, 2D and 3D surfaces

	template<typename T> void GenerateMipMap(MipOperator op, T* src, T* dest, unsigned w, unsigned h, unsigned d, unsigned c)
	{
		unsigned xOff = (w < 2) ? 0 : c;
		unsigned yOff = (h < 2) ? 0 : c * w;

		// NOTE: for NPOT textures

		h = TR_MAX((h >> 1) << 1, 1);
		w = TR_MAX((w >> 1) << 1, 1);
		d = TR_MAX((d >> 1) << 1, 1);

		if( d == 1 )
		{
			if( op == MIP_AVERAGE )
			{
				for( unsigned y = 0; y < h; y += 2 )
				{
					for( unsigned x = 0; x < w; x += 2 )
					{
						for( unsigned i = 0; i < c; i++ )
						{
							*dest++ = (src[0] + src[xOff] + src[yOff] + src[yOff + xOff]) / 4;
							src++;
						}

						src += xOff;
					}

					src += yOff;
				}
			}
			else
			{
				for( unsigned y = 0; y < h; y += 2 )
				{
					for( unsigned x = 0; x < w; x += 2 )
					{
						for( unsigned i = 0; i < c; i++ )
						{
							if( op == MIP_MIN )
								*dest++ = TR_MIN4(src[0], src[xOff], src[yOff], src[yOff + xOff]);
							else if( op == MIP_MAX )
								*dest++ = TR_MAX4(src[0], src[xOff], src[yOff], src[yOff + xOff]);
							else if( op == MIP_RANDOM_NEAREST )
							{
								unsigned id[4] = {0, xOff, yOff, yOff + xOff};
								*dest++ = src[ id[rand() % 4] ];
							}
							else
								*dest++ = src[0];

							src++;
						}

						src += xOff;
					}

					src += yOff;
				}
			}
		}
		else
		{
			unsigned zOff = (d < 2) ? 0 : c * w * h;

			for( unsigned z = 0; z < d; z += 2 )
			{
				for( unsigned y = 0; y < h; y += 2 )
				{
					for( unsigned x = 0; x < w; x += 2 )
					{
						for( unsigned i = 0; i < c; i++ )
						{
							*dest++ = (src[0] + src[xOff] + src[yOff] + src[yOff + xOff] + src[zOff] +
									src[zOff + xOff] + src[zOff + yOff] + src[zOff + yOff + xOff]) / 8;
							src++;
						}

						src += xOff;
					}

					src += yOff;
				}

				src += zOff;
			}
		}
	}

	//bool Object::GenerateMipmaps(MipOperator op, unsigned mipmaps)
	//{
	//	if( !IsValid(BATCHED | READ_ONLY) )
	//		return false;

	//	const FormatInfo* pFormatInfo = GetFormatInfo(m_format);

	//	mipmaps = TR_MIN(_GetMipmapsFromDimensions(), mipmaps);

	//	if( m_uiMipmaps == mipmaps )
	//		return true;
	//	
	//	if( !pFormatInfo->IsPlain() )
	//	{
	//		TR_PRINT(MSG_WARNING, "Sampler::Object::GenerateMipmaps(): Can't generate mipmaps for compressed/packed or depth/stencil formats!");
	//		return false;
	//	}

	//	m_uiMipmaps = mipmaps;

	//	m_pucPixels = MEM_Realloc(TR_BYTE, m_pucPixels, MipmappedSize());

	//	unsigned c = pFormatInfo->Channels();
	//	unsigned bpp = pFormatInfo->Bytes();

	//	// NOTE: Skip original image

	//	mipmaps--;

	//	if( !IsArrayTarget() && !IsCubeTarget() )
	//	{
	//		TR_BYTE* src = m_pucPixels;

	//		for( unsigned i = 0; i < mipmaps; i++ )
	//		{
	//			unsigned w = Width(i);
	//			unsigned h = Height(i);
	//			unsigned d = Depth(i);

	//			TR_BYTE* dst = src + w * h * d * bpp;

	//			if( pFormatInfo->Flags(TEXTURE_BYTE) )
	//			{
	//				if( pFormatInfo->Flags(TEXTURE_SIGNED) )
	//				{
	//					GenerateMipMap(op, (char*)src, (char*)dst, w, h, d, c);
	//				}
	//				else
	//				{
	//					GenerateMipMap(op, src, dst, w, h, d, c);
	//				}
	//			}
	//			else if( pFormatInfo->Flags(TEXTURE_SHORT) )
	//			{
	//				if( pFormatInfo->Flags(TEXTURE_SIGNED) )
	//				{
	//					GenerateMipMap(op, (short*)src, (short*)dst, w, h, d, c);
	//				}
	//				else
	//				{
	//					GenerateMipMap(op, (unsigned short*)src, (unsigned short*)dst, w, h, d, c);
	//				}
	//			}
	//			else if( pFormatInfo->Flags(TEXTURE_INT) )
	//			{
	//				if( pFormatInfo->Flags(TEXTURE_SIGNED) )
	//				{
	//					GenerateMipMap(op, (int*)src, (int*)dst, w, h, d, c);
	//				}
	//				else
	//				{
	//					GenerateMipMap(op, (unsigned*)src, (unsigned*)dst, w, h, d, c);
	//				}
	//			}
	//			else if( pFormatInfo->Flags(TEXTURE_HALF) )
	//			{
	//				GenerateMipMap(op, (half_float*)src, (half_float*)dst, w, h, d, c);
	//			}
	//			else
	//			{
	//				GenerateMipMap(op, (float*)src, (float*)dst, w, h, d, c);
	//			}

	//			src = dst;
	//		}
	//	}
	//	else
	//	{
	//		for( unsigned d = 0; d < m_uiDepth; d++ )
	//		{
	//			TR_BYTE* src = m_pucPixels + m_uiWidth * m_uiHeight * d * bpp;

	//			for( unsigned i = 0; i < mipmaps; i++ )
	//			{
	//				unsigned w = Width(i);
	//				unsigned h = Height(i);
	//				unsigned nw = Width(i + 1);
	//				unsigned nh = Height(i + 1);
	//				TR_BYTE* dst = src + (w * h * (m_uiDepth - d) + nw * nh * d)* bpp;

	//				if( pFormatInfo->Flags(TEXTURE_BYTE) )
	//				{
	//					if( pFormatInfo->Flags(TEXTURE_SIGNED) )
	//					{
	//						GenerateMipMap(op, (char*)src, (char*)dst, w, h, 1, c);
	//					}
	//					else
	//					{
	//						GenerateMipMap(op, src, dst, w, h, 1, c);
	//					}
	//				}
	//				else if( pFormatInfo->Flags(TEXTURE_SHORT) )
	//				{
	//					if( pFormatInfo->Flags(TEXTURE_SIGNED) )
	//					{
	//						GenerateMipMap(op, (short*)src, (short*)dst, w, h, 1, c);
	//					}
	//					else
	//					{
	//						GenerateMipMap(op, (unsigned short*)src, (unsigned short*)dst, w, h, 1, c);
	//					}
	//				}
	//				else if( pFormatInfo->Flags(TEXTURE_INT) )
	//				{
	//					if( pFormatInfo->Flags(TEXTURE_SIGNED) )
	//					{
	//						GenerateMipMap(op, (int*)src, (int*)dst, w, h, 1, c);
	//					}
	//					else
	//					{
	//						GenerateMipMap(op, (unsigned*)src, (unsigned*)dst, w, h, 1, c);
	//					}
	//				}
	//				else if( pFormatInfo->Flags(TEXTURE_HALF) )
	//				{
	//					GenerateMipMap(op, (half_float*)src, (half_float*)dst, w, h, 1, c);
	//				}
	//				else
	//				{
	//					GenerateMipMap(op, (float*)src, (float*)dst, w, h, 1, c);
	//				}

	//				src = dst;
	//			}
	//		}
	//	}

	//	return IsValid();
	//}

	//=============================================================================================

	//void Object::_TexSubImage(const TR_BYTE* pData, unsigned size, unsigned mipmap, unsigned layerOrFace, unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, bool bPBO)
	//{
	//	if( !bPBO )
	//		g_pRenderer->ChangeBuffer(0, OpenGL::PixelUnpackBuffer);

	//	const FormatInfo* pFormatInfo = GetFormatInfo(m_format);

	//	unsigned srcFormat	= pFormatInfo->SourceFormat();
	//	unsigned srcType	= pFormatInfo->Type();
	//	unsigned target		= IsCubeTarget() ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + layerOrFace : m_target;

	//	if( pFormatInfo->IsCompressed() )
	//	{
	//		unsigned internalFormat = pFormatInfo->InternalFormat();

	//		if( Is1DTarget() )
	//			glCompressedTextureSubImage1D(m_glTexture, target, mipmap, x, w, internalFormat, size, pData);
	//		else if( Is3DTarget() || Is2DArrayTarget() )
	//			glCompressedTextureSubImage3D(m_glTexture, target, mipmap, x, y, layerOrFace, w, h, d, internalFormat, size, pData);
	//		else
	//			glCompressedTextureSubImage2D(m_glTexture, target, mipmap, x, Is1DArrayTarget() ? layerOrFace : y, w, h, internalFormat, size, pData);
	//	}
	//	else
	//	{
	//		if( Is1DTarget() )
	//			glTextureSubImage1D(m_glTexture, target, mipmap, x, w, srcFormat, srcType, pData);
	//		else if( Is3DTarget() || Is2DArrayTarget() )
	//			glTextureSubImage3D(m_glTexture, target, mipmap, x, y, layerOrFace, w, h, d, srcFormat, srcType, pData);
	//		else
	//			glTextureSubImage2D(m_glTexture, target, mipmap, x, Is1DArrayTarget() ? layerOrFace : y, w, h, srcFormat, srcType, pData);
	//	}

	//	g_pRenderer->m_info.uiTextureSubImageBytes += size;
	//	g_pRenderer->m_info.uiTextureSubImageCalls++;
	//}

	//=============================================================================================

	//void Object::AttachBuffer(cBuffer* pBuffer, unsigned uiSize, unsigned uiOffset)
	//{
	//	const FormatInfo* pFormatInfo = GetFormatInfo(m_format); 

	//	assert( m_glTexture && !pFormatInfo->IsCompressed() );
	//	//assert( pBuffer->GetTarget() == OpenGL::TextureBuffer );
	//	assert( pBuffer->GetId() != 0 );
	//	assert( m_uiHeight == 1 && m_uiDepth == 1 && m_uiMipmaps == 1 && !IsStateSet(MULTISAMPLED) );
	//	assert( uiOffset % ENGINE_ALIGN_TBO == 0 );

	//	if( uiSize == TR_UNKNOWN )
	//		uiSize = pBuffer->GetSize() - uiOffset;

	//	assert( uiOffset + uiSize <= pBuffer->GetSize() );

	//	glTextureBufferRange(m_glTexture, GL_TEXTURE_BUFFER,  pFormatInfo->InternalFormat(), pBuffer->GetId(), uiOffset, uiSize);
	//}

	//=============================================================================================

	//bool Object::UploadBatch(bool bInternal)
	//{
	//	if( !IsStateSet(BATCHED) )
	//		return false;

	//	if( !bInternal && (m_uiBatchTick == g_pRenderer->GetFrameIndex() || g_pRenderer->IsStateSet(RENDERER_NO_TEX_UPDATES)) )
	//		return false;

	//	const sBTXBatch* pBlock = GetBatch();

	//	unsigned mipmap		= pBlock->ucMipmap;
	//	TR_BYTE action		= pBlock->ucAction;

	//	if( action == BTX_DATA )
	//	{
	//		unsigned size			= pBlock->uiSize;
	//		unsigned x				= pBlock->usX;
	//		unsigned y				= pBlock->usY;
	//		unsigned w				= pBlock->usW;
	//		unsigned h				= pBlock->usH;
	//		unsigned layerOrFace	= pBlock->usLayerOrFace;

	//		if( bInternal )
	//		{
	//			// IMPORTANT: Do not use PBO for texture updates for the first time: 1x1xN can cause driver stalls (lots updates of 1x1)!

	//			_TexSubImage(m_pucPixels + pBlock->uiOffset, size, mipmap, layerOrFace, x, y, w, h, 1);
	//		}
	//		else
	//			g_pRenderer->AddUploadTexData(m_pucPixels + pBlock->uiOffset, this, mipmap, layerOrFace, x, y, w, h, true);
	//	}
	//	else if( action == BTX_EMPTY )
	//	{
	//		m_uiBatchTick = g_pRenderer->GetFrameIndex();
	//	}
	//	else
	//	{
	//		glTextureParameteri(m_glTexture, m_target, GL_TEXTURE_BASE_LEVEL, mipmap);

	//		if( action == BTX_TEX_COMPLETE )
	//			_FreeMemory();
	//	}

	//	m_uiCurrentBatch++;

	//	return action == BTX_MIP_COMPLETE || action == BTX_TEX_COMPLETE;
	//}

	//=============================================================================================

	//bool Object::Upload(unsigned samples)
	//{
	//	Unload();

	//	const FormatInfo* pFormatInfo = GetFormatInfo(m_format);
	//	unsigned internalFormat = pFormatInfo->InternalFormat();

	//	if( IsRenderBufferTarget() )
	//	{
	//		assert( m_uiDepth == 1 && m_uiMipmaps == 1 );

	//		SetState(KEEP_STATE);

	//		OpenGL::CreateRenderbuffer(&m_glTexture);

	//		if( samples )
	//		{
	//			SetState(MULTISAMPLED);

	//			glNamedRenderbufferStorageMultisample(m_glTexture, samples, internalFormat, m_uiWidth, m_uiHeight);
	//		}
	//		else
	//			glNamedRenderbufferStorage(m_glTexture, internalFormat, m_uiWidth, m_uiHeight);
	//	}
	//	else
	//	{
	//		OpenGL::CreateTexture(&m_glTexture);

	//		if( IsTextureBufferTarget() )
	//		{
	//			SetState(KEEP_STATE);
	//		}
	//		else if( Is2DMultisampleTarget() )
	//		{
	//			SetState(MULTISAMPLED);

	//			glTextureStorage2DMultisample(m_glTexture, m_target, samples, internalFormat, m_uiWidth, m_uiHeight, GL_TRUE);
	//		}
	//		else if( Is2DArrayMultisampleTarget() )
	//		{
	//			SetState(MULTISAMPLED);

	//			glTextureStorage3DMultisample(m_glTexture, m_target, samples, internalFormat, m_uiWidth, m_uiHeight, m_uiDepth, GL_TRUE);
	//		}
	//		else
	//		{				
	//			// FIXME: flag IsStateSet(SEAMLESS)?

	//			//if( IsAnyCubeTarget() )
	//			//	glTextureParameteri(m_glTexture, m_target, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_TRUE);

	//			Swizzle(NULL);

	//			if( m_uiMipmaps > 1 )
	//			{
	//				assert( !IsRectangleTarget() );

	//				SetState(MIPMAPPED);
	//			}

	//			if( !IsRectangleTarget() )
	//				glTextureParameteri(m_glTexture, m_target, GL_TEXTURE_MAX_LEVEL, m_uiMipmaps - 1);

	//			// NOTE: allocate

	//			if( Is1DTarget() )
	//				glTextureStorage1D(m_glTexture, m_target, m_uiMipmaps, internalFormat, m_uiWidth);
	//			else if( IsStorage3D() )
	//				glTextureStorage3D(m_glTexture, m_target, m_uiMipmaps, internalFormat, m_uiWidth, m_uiHeight, m_uiDepth);
	//			else if( Is2DArrayMultisampleTarget() )
	//				glTextureStorage3DMultisample(m_glTexture, m_target, samples,internalFormat, m_uiWidth, m_uiHeight, m_uiDepth, GL_TRUE);
	//			else if( Is2DMultisampleTarget() )
	//				glTextureStorage2DMultisample(m_glTexture, m_target, samples, internalFormat, m_uiWidth, m_uiHeight, GL_TRUE);
	//			else
	//				glTextureStorage2D(m_glTexture, m_target, m_uiMipmaps, internalFormat, m_uiWidth, Is1DArrayTarget() ? m_uiDepth : m_uiHeight);

	//			// NOTE: upload

	//			if( m_pucPixels )
	//			{
	//				if( IsStateSet(BATCHED) )
	//				{
	//					while( IsStateSet(BATCHED) )
	//					{
	//						bool bMipComplete = UploadBatch(true);

	//						if( !g_pRenderer->IsStateSet(RENDERER_NO_TEXTURE_STREAMING) && bMipComplete )
	//							break;
	//					}
	//				}
	//				else
	//				{
	//					unsigned mipmap = 0;
	//					unsigned offset = 0;

	//					while( mipmap < m_uiMipmaps )
	//					{
	//						unsigned size = MipmappedSize(mipmap, 1);

	//						unsigned w = Width(mipmap);
	//						unsigned h = Height(mipmap);
	//						unsigned d = Depth(mipmap);

	//						if( IsCubeTarget() )
	//						{
	//							size /= 6;

	//							for( unsigned i = 0; i < 6; i++ )
	//							{
	//								_TexSubImage(m_pucPixels + offset, size, mipmap, i, 0, 0, w, h, d);

	//								offset += size;
	//							}
	//						}
	//						else
	//						{
	//							_TexSubImage(m_pucPixels + offset, size, mipmap, 0, 0, 0, w, h, d);
	//						
	//							offset += size;
	//						}

	//						mipmap++;
	//					}
	//				}
	//			}
	//		}
	//	}

	//	m_uiSize_gpu = m_uiSize_cpu;

	//	if( IsStateSet(MULTISAMPLED) )
	//		m_uiSize_gpu *= TR_MAX(samples, 1);

	//	if( !IsStateSet(KEEP_MEMORY | BATCHED) )
	//		_FreeMemory();

	//	g_pRenderer->m_info.iTextureTotalBytes += m_uiSize_gpu;

 //       // Test for labels
 //       Bind(32);

	//	return true;
	//}

	//=================================================================================================

	//void Object::Update(const void* pData)
	//{
	//	assert( m_glTexture != 0 );

	//	assert( !IsRenderBufferTarget() );
	//	assert( !IsMultisampleTarget() );
	//	assert( !IsTextureBufferTarget() );

	//	unsigned mipmap			= 0;
	//	unsigned offset			= 0;

	//	TR_BYTE* ptr = (TR_BYTE*)pData;

	//	while( mipmap < m_uiMipmaps )
	//	{
	//		unsigned size = MipmappedSize(mipmap, 1);
	//		
	//		unsigned w = Width(mipmap);
	//		unsigned h = Height(mipmap);
	//		unsigned d = Depth(mipmap);

	//		if( IsCubeTarget() )
	//		{
	//			size /= 6;

	//			for( unsigned i = 0; i < 6; i++ )
	//			{
	//				_TexSubImage(ptr + offset, size, mipmap, i, 0, 0, w, h, d);

	//				offset += size;
	//			}

	//			size = 0;
	//		}
	//		else
	//			_TexSubImage(ptr + offset, size, mipmap, 0, 0, 0, w, h, d);

	//		offset += size;

	//		mipmap++;
	//	}
	//}

	//=================================================================================================

	//void Object::UpdateLayer(const void* pData, unsigned mipmap, unsigned layerOrFace)
	//{
	//	assert( m_glTexture != 0 );

	//	assert( !IsMultisampleTarget() );
	//	assert( !IsRenderBufferTarget() );
	//	assert( !IsTextureBufferTarget() );

	//	unsigned w = Width(mipmap);
	//	unsigned h = Height(mipmap);
	//	unsigned size = MipmappedSize(mipmap, 1, FORMAT_NONE, w, h, 1);

	//	TR_BYTE* ptr = (TR_BYTE*)pData;

	//	_TexSubImage(ptr, size, mipmap, layerOrFace, 0, 0, w, h, 1);
	//}

	////=================================================================================================

	//void Object::UpdateRegion(const void* pData, unsigned mipmap, unsigned layerOrFace, unsigned x, unsigned y, unsigned w, unsigned h)
	//{
	//	unsigned uiSize = MipmappedSize(mipmap, 1, FORMAT_NONE, w, h, 1);

	//	TR_BYTE* ptr = (TR_BYTE*)pData;

	//	_TexSubImage(ptr, uiSize, mipmap, layerOrFace, x, y, w, h, 1);
	//}

	////=================================================================================================

	//void Object::UpdateRegion(unsigned uiOffset, unsigned uiSize, unsigned mipmap, unsigned layerOrFace, unsigned x, unsigned y, unsigned w, unsigned h)
	//{
	//	assert( m_glTexture != 0 );

	//	assert( !IsMultisampleTarget() );
	//	assert( !IsRenderBufferTarget() );
	//	assert( !IsTextureBufferTarget() );
	//	assert( uiOffset % TR_MAX(GetFormatInfo(m_format)->Bytes(), 4) == 0 );

	//	_TexSubImage(BYTE_OFFSET(uiOffset), uiSize, mipmap, layerOrFace, x, y, w, h, 1, true);
	//}

	////=================================================================================================

	//void Object::Transfer(unsigned mipmap, unsigned xOffset, unsigned yOffset, unsigned layerOrFace, unsigned x, unsigned y, unsigned w, unsigned h)
	//{
	//	assert( !IsTextureBufferTarget() );
	//	assert( !IsRenderBufferTarget() );
	//	assert( !GetFormatInfo(m_format)->IsCompressed() );

	//	_ModifyRect(x, y, w, h, mipmap);

	//	unsigned target = IsCubeTarget() ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + layerOrFace : m_target;

	//	if( Is1DTarget() )
	//		glCopyTextureSubImage1D(m_glTexture, target, mipmap, xOffset, x, y, w);
	//	else if( Is3DTarget() || Is2DArrayTarget() )
	//		glCopyTextureSubImage3D(m_glTexture, target, mipmap, xOffset, yOffset, layerOrFace, x, y, w, h);
	//	else
	//		glCopyTextureSubImage2D(m_glTexture, target, mipmap, xOffset, Is1DArrayTarget() ? layerOrFace : yOffset, x, y, w, h);
	//}

	////=============================================================================================

	//void Object::Readback(unsigned x, unsigned y, unsigned w, unsigned h, unsigned mipmap)
	//{
	//	_ModifyRect(x, y, w, h, mipmap);

	//	const FormatInfo* pFormatInfo = GetFormatInfo(m_format);
	//	TR_BYTE* pDst = Pixels(mipmap);

	//	assert( pDst );

	//	unsigned srcFormat	= pFormatInfo->SourceFormat();
	//	unsigned srcType	= pFormatInfo->Type();

	//	g_pRenderer->ChangeBuffer(0, OpenGL::PixelPackBuffer);

	//	glReadPixels(x, y, w, h, srcFormat, srcType, pDst);
	//}

	////=============================================================================================

	void Object::Unload()
	{
		//if( m_glTexture )
		//{
		//	assert( g_pRenderer );
		//	assert( g_pRenderer->IsStateSet(RENDERER_INITIALIZED) );

		//	if( IsRenderBufferTarget() )
		//		OpenGL::DeleteRenderbuffer(&m_glTexture);
		//	else
		//		g_pRenderer->DeleteTexture(&m_glTexture);

		//	g_pRenderer->m_info.iTextureTotalBytes -= m_uiSize_gpu;

		//	m_glTexture = 0;
		//}

		//m_uiSize_gpu = 0;
	}

	////=================================================================================================

	//void Object::Bind(unsigned unit)
	//{
	//	assert( m_glTexture );

	//	UploadBatch();

	//	if( m_pStateObject )
	//		m_pStateObject->Bind(unit);

	//	g_pRenderer->ChangeTexture(unit, m_target, m_glTexture);
 //       _ApplyLabel();
	//}

	////=================================================================================================

	//void Object::BindWithState(unsigned unit, PredefinedStateObject state)
	//{
	//	assert(m_glTexture);

	//	UploadBatch();

	//	StateObject* pState = g_predefinedStateObjects + state;

	//	if (pState)
	//		pState->Bind(unit);

	//	g_pRenderer->ChangeTexture(unit, m_target, m_glTexture);
 //       _ApplyLabel();
	//}

	////=================================================================================================

	//void Object::UpdateMipmaps()
	//{
	//	assert( IsStateSet(MIPMAPPED) );

	//	glGenerateTextureMipmap(m_glTexture, m_target);
	//}

	////=================================================================================================

	//void Object::AttachState(PredefinedStateObject state)
	//{
	//	AttachState(g_predefinedStateObjects + state);
	//}

	////=================================================================================================

	//void Object::Swizzle(const char* pcSwizzle4)
	//{
	//	bool bSet = false;
	//	
	//	if( pcSwizzle4 )
	//	{
	//		unsigned uiSwizzle = *(unsigned*)pcSwizzle4;

	//		if( m_glTexture && uiSwizzle != m_uiSwizzle )
	//			bSet = true;

	//		m_uiSwizzle = uiSwizzle;
	//	}
	//	else if( m_glTexture && m_uiSwizzle != MAKE_FOURCC('R', 'G', 'B', 'A') )
	//		bSet = true;

	//	if( bSet )
	//	{
	//		int swizzle[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};

	//		for( int i = 0; i < 4; i++ )
	//		{
	//			if( m_cSwizzle[i] == 'R' )
	//				swizzle[i] = GL_RED;
	//			else if( m_cSwizzle[i] == 'G' )
	//				swizzle[i] = GL_GREEN;
	//			else if( m_cSwizzle[i] == 'B' )
	//				swizzle[i] = GL_BLUE;
	//			else if( m_cSwizzle[i] == 'A' )
	//				swizzle[i] = GL_ALPHA;
	//			else if( m_cSwizzle[i] == '1' )
	//				swizzle[i] = GL_ONE;
	//			else if( m_cSwizzle[i] == '0' )
	//				swizzle[i] = GL_ZERO;
	//			else
	//				assert(false);
	//		}

	//		glTextureParameteriv(m_glTexture, m_target, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	//	}
	//}

	////=================================================================================================

	//void Object::_ModifyRect(unsigned& x, unsigned& y, unsigned& w, unsigned& h, unsigned mipmap) const
	//{
	//	const TR_ViewPort* pVp = g_pRenderer->ViewportCurrentPtr();

	//	if( x == USE_VIEWPORT_POS )
	//		x = pVp->x;

	//	if( y == USE_VIEWPORT_POS )
	//		y = pVp->y;

	//	if( !w )
	//		w = Width(mipmap);
	//	else if( w == USE_VIEWPORT_SIZE )
	//		w = pVp->w;
	//
	//	if( !h )
	//		h = Height(mipmap);
	//	else if( h == USE_VIEWPORT_SIZE )
	//		h = pVp->h;
	//}

	//=================================================================================================

	StateObject::StateObject() :
		m_anisotropy(1.0f)
		, m_filterMin(NearestMipmapNearest)
		, m_filterMag(Nearest)
		, m_compareFunc(GL_ALWAYS)
		, m_samplerObject(0)
	{
		m_wrap[0] = Repeat;
		m_wrap[1] = Repeat;
		m_wrap[2] = Repeat;
	}

	//=================================================================================================

	void StateObject::Free()
	{
		//g_pRenderer->DeleteSampler(&m_samplerObject);
	}

	//=================================================================================================

	void StateObject::Bind(unsigned unit) const
	{
		//g_pRenderer->ChangeSampler(unit, m_samplerObject);
	}

	//=================================================================================================

	//void StateObject::Setup()
	//{
	//	if( !m_samplerObject )
	//	{
	//		OpenGL::CreateSampler(&m_samplerObject);

	//		glSamplerParameteri(m_samplerObject, GL_TEXTURE_WRAP_S, m_wrap[COORD_S]);
	//		glSamplerParameteri(m_samplerObject, GL_TEXTURE_WRAP_T, m_wrap[COORD_T]);
	//		glSamplerParameteri(m_samplerObject, GL_TEXTURE_WRAP_R, m_wrap[COORD_R]);

	//		glSamplerParameteri(m_samplerObject, GL_TEXTURE_MAG_FILTER, m_filterMag);
	//		glSamplerParameteri(m_samplerObject, GL_TEXTURE_MIN_FILTER, m_filterMin);

	//		SetAnisotropy();

	//		if( m_compareFunc != GL_ALWAYS )
	//		{
	//			glSamplerParameteri(m_samplerObject, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//			glSamplerParameteri(m_samplerObject, GL_TEXTURE_COMPARE_FUNC, m_compareFunc);
	//		}
	//		else
	//			glSamplerParameteri(m_samplerObject, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	//		if( IsStateSet(SAMPLER_SRGB_SKIP_DECODE) )
	//			glSamplerParameteri(m_samplerObject, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
	//	}
	//}

	////=================================================================================================

	//void StateObject::SetAnisotropy()
	//{
	//	if( m_anisotropy != NO_ANISOTROPY && m_filterMin == GL_LINEAR_MIPMAP_LINEAR )
	//	{
	//		float anisotropy = g_pRenderer->m_fAnisotropy < 1.0f ? m_anisotropy : g_pRenderer->m_fAnisotropy;

	//		glSamplerParameterf(m_samplerObject, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	//		glSamplerParameterf(m_samplerObject, GL_TEXTURE_LOD_BIAS, g_pRenderer->m_fLodBias);
	//	}
	//}
}
