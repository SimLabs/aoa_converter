#pragma once

//#include <cstddef>
//#include "glext.h"
#include <osg/Texture>

#define GL_MIRRORED_REPEAT                0x8370

#define GL_MIRROR_CLAMP_ATI               0x8742
#define GL_MIRROR_CLAMP_TO_EDGE_ATI       0x8743

#define GL_TEXTURE_1D_ARRAY               0x8C18
#define GL_TEXTURE_2D_ARRAY               0x8C1A

#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
#define GL_TEXTURE_CUBE_MAP_ARRAY         0x9009

#define GL_RENDERBUFFER                   0x8D41

#define GL_RED_INTEGER                    0x8D94
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99

#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I

#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B

#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_DEPTH32F_STENCIL8              0x8CAD

#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD

#define GL_RGB10_A2UI                     0x906F
#define GL_R11F_G11F_B10F                 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
#define GL_RGB9_E5                        0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV       0x8C3E

#define GL_COMPRESSED_RED_RGTC1           0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
#define GL_COMPRESSED_RG_RGTC2            0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE

#define GL_COMPRESSED_RGBA_BPTC_UNORM     0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8

#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F

#include <cstdint>
#include <cstring>
#include <cassert>

#define MAKE_FOURCC(c0, c1, c2, c3)			(c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))
#define BTX_MAGIC							MAKE_FOURCC('B', 'T', 'X', '2')

class cBuffer;
using TR_BYTE = char;
constexpr uint32_t TR_UNKNOWN = 0xFFFFFFFF;

template<typename T> class ctBits
{
private:

    T m_state;

public:

    inline ctBits() : m_state(T(0))
    {
    }

    inline ctBits(T bits) : m_state(bits)
    {
    }

    inline void SetAllState(T bits)
    {
        m_state = bits;
    }

    inline void SetState(T mask)
    {
        m_state |= mask;
    }

    inline void ClearState(T mask)
    {
        m_state &= ~mask;
    }

    inline void ToggleState(T mask)
    {
        m_state ^= mask;
    }

    inline bool IsStateSet(T mask) const
    {
        return (m_state & mask) != 0;
    }

    inline bool IsStatesSet(T mask) const
    {
        return (m_state & mask) == mask;
    }

    inline T GetState(T mask) const
    {
        return m_state & mask;
    }
};

// NOTE: typedefs

typedef ctBits<uint32_t>		cBits32;
typedef ctBits<uint16_t>		cBits16;
typedef ctBits<uint8_t>			cBits8;

namespace Sampler
{
	const int ALL_MIPMAPS			= 127;
	const int AVAILABLE_MIPMAPS		= 128;
	const int MAX_NAMELEN			= 260;

	//=================================================================================================

	// IMPORTANT: keep order! Expression like "Format = (TextureFormat)(AnotherFormat + const)" can be used!

	// 		Xs	- signed normalized
	// 		ui	- unsigned integer
	// 		i	- signed integer
	// 		uf	- unsigned float
	// 		f	- float
	// 		sX	- sRGB-space
	// 		bc1	- DXT1
	// 		bc2	- DXT3
	// 		bc3	- DXT5
	// 		bc4	- RGTC1
	// 		bc5	- RGTC2
	// 		bc6	- BPTC float
	// 		bc7	- BPTC fixed
	// 		e	- shared exponent

	enum Format
	{
		FORMAT_NONE = 0,

		// NOTE: plane 8-bit per channel

		R8,
		R8s,
		R8i,
		R8ui,

		RG8,
		RG8s,
		RG8i,
		RG8ui,

		sRGBA8,
		RGBA8,
		RGBA8s,
		RGBA8i,
		RGBA8ui,

		// NOTE: plane 16-bit per channel

		R16,
		R16s,
		R16i,
		R16ui,
		R16f,

		RG16,
		RG16s,
		RG16i,
		RG16ui,
		RG16f,

		RGBA16,
		RGBA16s,
		RGBA16i,
		RGBA16ui,
		RGBA16f,

		// NOTE: plane 32-bit per channel

		R32i,
		R32ui,
		R32f,

		RG32i,
		RG32ui,
		RG32f,

		RGB32i,
		RGB32ui,
		RGB32f,

		RGBA32i,
		RGBA32ui,
		RGBA32f,

		// NOTE: depth-stencil

		DEPTH16,
		DEPTH24,
		DEPTH24_STENCIL8,
		DEPTH32,
		DEPTH32f,
		DEPTH32f_STENCIL8,

		// NOTE: packed

		RGBA4444,
		RGBA5551,
		RGBA1010102,
		RGBA1010102ui,
		RGB111110f,
		RGB9e5,

		// NOTE: compressed

		R_bc4u,
		R_bc4s,
		RG_bc5u,
		RG_bc5s,
		RGB_bc6u,
		RGB_bc6s,
		RGBA_bc1,
		sRGBA_bc1,
		RGBA_bc2,
		sRGBA_bc2,
		RGBA_bc3,
		sRGBA_bc3,
		RGBA_bc7,
		sRGBA_bc7,

        // types introduced for DDS
        RGB8,
        BGR8,
        BGRA8,

		FORMATS_NUM
	};

	//=================================================================================================

	enum eTextureFormatFlag : unsigned
	{
		TEXTURE_BYTE			= 0x00000001,
		TEXTURE_SHORT			= 0x00000002,
		TEXTURE_INT				= 0x00000004,
		TEXTURE_HALF			= 0x00000008,
		TEXTURE_FLOAT			= 0x00000010,
		TEXTURE_PACKED			= 0x00000020,
		TEXTURE_BC1				= 0x00000040,
		TEXTURE_BC2				= 0x00000080,
		TEXTURE_BC3				= 0x00000100,
		TEXTURE_BC4				= 0x00000200,
		TEXTURE_BC5				= 0x00000400,
		TEXTURE_BC6				= 0x00000800,
		TEXTURE_BC7				= 0x00001000,
		TEXTURE_DEPTH			= 0x00002000,
		TEXTURE_STENCIL			= 0x00004000,
		TEXTURE_SIGNED			= 0x00008000,
		TEXTURE_INTEGER			= 0x00010000,
		TEXTURE_SRGB			= 0x00020000,
		TEXTURE_PACKED_FLOAT	= 0x00040000,
		TEXTURE_RENDERABLE		= 0x00080000,

		TEXTURE_BC				= TEXTURE_BC1 | TEXTURE_BC2 | TEXTURE_BC3 | TEXTURE_BC4 | TEXTURE_BC5 | TEXTURE_BC6 | TEXTURE_BC7,

		TEXTURE_ALL				= 0xFFFFFFFF
	};

	//=================================================================================================

	enum Wrap : unsigned short
	{
		Repeat							= GL_REPEAT,
		Clamp							= GL_CLAMP,
		ClampToEdge						= GL_CLAMP_TO_EDGE,
		MirroredRepeat					= GL_MIRRORED_REPEAT,
		MirrorClamp						= GL_MIRROR_CLAMP_ATI,
		MirrorClampToEdge				= GL_MIRROR_CLAMP_TO_EDGE_ATI,

	};

	enum Filter : unsigned short
	{
		Nearest							= GL_NEAREST,
		Linear							= GL_LINEAR,
		NearestMipmapNearest			= GL_NEAREST_MIPMAP_NEAREST,
		LinearMipmapNearest				= GL_LINEAR_MIPMAP_NEAREST,
		NearestMipmapLinear				= GL_NEAREST_MIPMAP_LINEAR,
		LinearMipmapLinear				= GL_LINEAR_MIPMAP_LINEAR,
	};

	enum Target
	{
		Texture1d						= GL_TEXTURE_1D,
		Texture1dArray					= GL_TEXTURE_1D_ARRAY,
		Texture2d						= GL_TEXTURE_2D,
		Texture2dMultisample			= GL_TEXTURE_2D_MULTISAMPLE,
		Texture2dArray					= GL_TEXTURE_2D_ARRAY,
		Texture2dMultisampleArray		= GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
		Texture3d						= GL_TEXTURE_3D,
		TextureCubeMap					= GL_TEXTURE_CUBE_MAP,
		TextureCubeMapArray				= GL_TEXTURE_CUBE_MAP_ARRAY,
		TextureRectangle				= GL_TEXTURE_RECTANGLE,
		TextureBuffer					= GL_TEXTURE_BUFFER,
		RenderBuffer					= GL_RENDERBUFFER,
		
		TextureCubeMapPositiveX			= GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		TextureCubeMapNegativeX			= GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		TextureCubeMapPositiveY			= GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		TextureCubeMapNegativeY			= GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		TextureCubeMapPositiveZ			= GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		TextureCubeMapNegativeZ			= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

	enum PredefinedStateObject
	{
		ClampNearest,
		RepeatNearest,
		MirroredRepeatNearest,
		ClampNearestMipmapNearest,
		RepeatNearestMipmapNearest,
		MirroredRepeatNearestMipmapNearest,
		ClampLinear,
		RepeatLinear,
		MirroredRepeatLinear,
		ClampLinearMipmapNearest,
		RepeatLinearMipmapNearest,
		MirroredRepeatLinearMipmapNearest,
		ClampLinearMipmapLinear,
		RepeatLinearMipmapLinear,
		RepeatLinearMipmapLinearAnisotropy,
		MirroredRepeatLinearMipmapLinear,
		ClampLinearShadow,
		ClampLinearSkipSrgb,

		PREDEFINED_STATE_OBJECTS_NUM
	};

	//=================================================================================================

	enum Unit
	{
		Unit0,
		Unit1,
		Unit2,
		Unit3,
		Unit4,
		Unit5,
		Unit6,
		Unit7,
		Unit8,
		Unit9,
		Unit10,
		Unit11,
		Unit12,
		Unit13,
		Unit14,
		Unit15,
		Unit16,
		Unit17,
		Unit18,
		Unit19,
		Unit20,
		Unit21,
		Unit22,
		Unit23,
		Unit24,
		Unit25,
		Unit26,
		Unit27,
		Unit28,
		Unit29,
		Unit30,
		Unit31,
	};

	//=================================================================================================

	// IMPORTANT: 0xFFFF0000 are user state bytes!

	enum State
	{
		// NOTE: flags only for IsStateSet()

		MULTISAMPLED					= 0x00000001,
		MIPMAPPED						= 0x00000002,

		// NOTE: flags for SetState() and IsStateSet()

		// IMPORTANT: Avoid change READ_ONLY flag to TRUE after ReadFile, CreateCubemap or Create
		//            calls: memory leaks may occurred!

		KEEP_STATE						= 0x00000004,
		READ_ONLY						= 0x00000008,
		KEEP_MEMORY						= 0x00000010,
		MAPPED_DATA						= 0x00000020,
		SEAMLESS						= 0x00000040,

		// NOTE: internal flags

		BATCHED							= 0x00000080,
		OBJECT_REFERENCE				= 0x00000100,
		UNUSED							= 0x80000000,

		USE_VIEWPORT_POS				= 999999,
		USE_VIEWPORT_SIZE				= 1000000,

		// NOTE: batch settings
	
		BATCH_WIDTH						= 64,
		BATCH_MAX_SIZE					= BATCH_WIDTH * BATCH_WIDTH * 4 * sizeof(float),

		// NOTE: flags for LoadFile()

		SKIP_BMP_PALETTE				= 0x00000001,
		SKIP_2DA						= 0x00000002,
		SKIP_BATCHIFY					= 0x00000004,
		BATCHIFY_NOTIFICATION			= 0x00000008,
	};

	//=================================================================================================

	enum MipOperator
	{
		MIP_AVERAGE,
		MIP_MIN,
		MIP_MAX,
		MIP_NEAREST,
		MIP_RANDOM_NEAREST
	};

	enum RescaleFilter
	{
		NEAREST_FILTER,
		LINEAR_FILTER
	};

	enum BatchType : unsigned char
	{
		BTX_DATA,
		BTX_MIP_COMPLETE,
		BTX_TEX_COMPLETE,
		BTX_EMPTY,
	};

	//=================================================================================================

	struct FormatInfo
	{
		private:

			const char*			m_pczName;
			unsigned			m_type;
			unsigned			m_sourceFormat;
			unsigned			m_internalFormat;
			unsigned			m_uiFlags;
			unsigned char		m_ucChannels;
			unsigned char		m_ucByteSize;

		public:

			FormatInfo
			(
				const char*			pcName,
				unsigned char		ucChannels,
				unsigned			uiFlags,
				unsigned char		ucByteSize,
				unsigned			type,
				unsigned			sourceFormat,
				unsigned			internalFormat
			) : 
				m_pczName(pcName),
				m_type(type),
				m_sourceFormat(sourceFormat),
				m_internalFormat(internalFormat),
				m_uiFlags(uiFlags),
				m_ucChannels(ucChannels),
				m_ucByteSize(ucByteSize)
			{
			}

			// NOTE: GL types

			inline unsigned Type() const
			{
				return m_type;
			}

			inline unsigned SourceFormat() const
			{
				return m_sourceFormat;
			}

			inline unsigned InternalFormat() const
			{
				return m_internalFormat;
			}

			// NOTE: strings

			inline const char* Name() const
			{
				return m_pczName;
			}

			// NOTE: misc

			inline unsigned	Channels() const
			{
				return m_ucChannels;
			}

			inline unsigned Flags(unsigned mask) const
			{
				return m_uiFlags & mask;
			}

			inline bool IsRenderable() const
			{
				return Flags(TEXTURE_RENDERABLE) != 0;
			}

			inline bool IsPlain() const
			{
				return !Flags(TEXTURE_PACKED | TEXTURE_BC | TEXTURE_PACKED_FLOAT | TEXTURE_DEPTH | TEXTURE_STENCIL);
			}

			inline bool IsPacked() const
			{
				return Flags(TEXTURE_PACKED) != 0;
			}

			inline bool IsDepth() const
			{
				return Flags(TEXTURE_DEPTH) != 0;
			}

			inline bool IsStencil() const
			{
				return Flags(TEXTURE_STENCIL) != 0;
			}

			inline bool IsCompressed() const
			{
				return Flags(TEXTURE_BC) != 0;
			}

			inline bool IsFloat() const
			{
				return Flags(TEXTURE_FLOAT | TEXTURE_HALF) != 0;
			}

			inline bool IsHalf() const
			{
				return Flags(TEXTURE_HALF) != 0;
			}

			// IMPORTANT: for plain formats only

			inline unsigned BytesPerChannel() const
			{
				return Bytes() / Channels();
			}

			// IMPORTANT: Returns bytes per block for compressed formats!

			inline unsigned Bytes() const
			{
				return m_ucByteSize;
			}
	};

	//=================================================================================================

	#define NO_ANISOTROPY		0.0f

	enum eSamplerState
	{
		SAMPLER_SRGB_SKIP_DECODE	= 0x00000001,
	};

	class StateObject : public cBits32
	{
		public:

			float			m_anisotropy;
			Filter			m_filterMin;
			Filter			m_filterMag;
			Wrap			m_wrap[3];
			unsigned short	m_compareFunc;

		private:

			// IMPORTANT: must be last!

			unsigned		m_samplerObject;

		public:
		
			StateObject();

			inline ~StateObject()
			{
				Free();
			}

			void Free();
			void Setup();
			void SetAnisotropy();
			void Bind(unsigned unit) const;

			inline bool IsSame(const StateObject* pData) const
			{
				return !memcmp(this, pData, sizeof(StateObject) - sizeof(unsigned));
			}

			inline void Copy(const StateObject* pData)
			{
				assert( !m_samplerObject );

				memcpy(this, pData, sizeof(StateObject) - sizeof(unsigned));
			}

			inline bool IsMipmapped() const
			{
				return m_filterMin > Linear;
			}
	};

	//=================================================================================================

	void				ModifyRect(unsigned& x, unsigned& y, unsigned& w, unsigned& h);
	const FormatInfo*	GetFormatInfo(Format format);
	Format				FindFormat(Format baseFormat, unsigned flagsToRemove, unsigned flagsToAdd, unsigned channels);
	void				SetupPredefinedStateObjects();
	void				FreePredefinedStateObjects();

	//=================================================================================================

	#pragma pack(push, 1)

	struct sBTXHeader
	{
		unsigned		uiMagic;
		unsigned		uiHash;
		unsigned		uiDataSize;
		unsigned		uiSwizzle;
		unsigned short	usTarget;
		unsigned short	usFormat;
		unsigned short	usWidth;
		unsigned short	usHeight;
		unsigned short	usDepth;
		unsigned short	usMipmaps;
		unsigned short	usBatchesNum;
		unsigned short	usReserved;
	};

	struct sBTXBatch
	{
		unsigned		uiOffset;
		unsigned		uiSize;
		unsigned short	usX;
		unsigned short	usY;
		unsigned short	usW;
		unsigned short	usH;
		unsigned short	usLayerOrFace;
		unsigned char	ucMipmap;
		unsigned char	ucAction;
	};

	#pragma pack(pop)

	class Object : public cBits32
	{
		public:

			static void Copy
			(
				const Object* pSrc, const Object* pDst,
				unsigned uiSrcLevel = 0, unsigned uiSrcX = 0, unsigned uiSrcY = 0, unsigned uiSrcZ = 0,
				unsigned uiDstLevel = 0, unsigned uiDstX = 0, unsigned uiDstY = 0, unsigned uiDstZ = 0,
				unsigned uiWidth = 0, unsigned uiHeight = 0, unsigned uiDepth = 0
			);

		private:

			// NOTE: GPU part

			unsigned				m_glTexture;
			Target					m_target;
			const StateObject*		m_pStateObject;

			union
			{
				char				m_cSwizzle[4];
				unsigned			m_uiSwizzle;
			};

			// NOTE: CPU part

			Format					m_format;
			unsigned				m_uiWidth;
			unsigned				m_uiHeight;
			unsigned				m_uiDepth;
			unsigned				m_uiMipmaps;

			TR_BYTE*				m_pucPixels;

			// NOTE: misc

			unsigned				m_uiCurrentBatch;
			unsigned				m_uiBatchTick;
		
			int						m_iRef;

			char					m_pczFilename[MAX_NAMELEN];
			unsigned				m_hashName;
            bool                    m_bLabelled = false;

		public:

			unsigned				m_uiSize_gpu;
			unsigned				m_uiSize_cpu;

		private:

			void _ModifyRect(unsigned& x, unsigned& y, unsigned& w, unsigned& h, unsigned mipmap) const;
			void _Zero();
			void _TexSubImage(const unsigned char* pData, unsigned size, unsigned mipmap, unsigned layerOrFace, unsigned x, unsigned y, unsigned w, unsigned h, unsigned d, bool bPBO = false);

			bool _ReadFileBMP(const char* filename, bool skipPalette = false);
			bool _ReadFileRAW(const char* filename);
			bool _ReadFileBTX(const char* filename);
            bool _ReadFileDDS(const char* filename);

			unsigned _GetMipmapsFromDimensions() const;
			void _FreeMemory();

            void _ApplyLabel();

		public:

			Object()
			{
				// IMPORTANT: unique name will be generated in _Zero() only once!

				*m_pczFilename = '\0';

				_Zero();
			}

			~Object()
			{
				_FreeMemory();
				Unload();
			}

			// NOTE: Create image from file (BMP, TGA, DDS or RAW)
			// IMPORTANT: Format, Sizes and Target must be initialized before (use Create())!

			bool LoadFile(const char* filename, unsigned uiFlags = 0);

			// NOTE: Create image from memory (may be empty)

			void Create(Target target, Format format, TR_BYTE* pData, unsigned w, unsigned h, unsigned d = 1, unsigned mipmaps = 1, unsigned flags = 0, char const *label = nullptr);

			// NOTE: Dump only pixels to .raw file (no header)

			bool SaveFileRAW(const char* filename) const;

			// NOTE: Save file

			bool SaveFilePNG(const char* filename) const;
			bool SaveFileBTX(const char* filename) const;

			// NOTE: Mipmaps level generation (non-power of two textures are also supported)
			//       op - average, min, max, nearest - for non-3D textures
			//       op - average only - for 3D textures

			bool GenerateMipmaps(MipOperator op, unsigned mipmaps = ALL_MIPMAPS);

			// NOTE: Validate object

			inline bool IsValid(unsigned nonState = UNUSED) const
			{
				return m_format != FORMAT_NONE && m_pucPixels && !IsStatesSet(nonState);
			}

			inline bool IsUploaded()
			{
				return m_format != FORMAT_NONE && m_glTexture != 0;
			}

			// NOTE: Misc name

			inline const char* GetFilename() const				{ return m_pczFilename; }
			inline void SetFilename(const char* name)			{ memcpy(m_pczFilename, name, MAX_NAMELEN); m_bLabelled = false; }
			inline void SetHashName(unsigned _hash)				{ m_hashName = _hash; }
			inline unsigned GetHashName() const					{ return m_hashName; }

			// NOTE: Format

			inline Format GetFormat() const						{ return m_format; }

			// NOTE: Number of image mipmaps levels

			inline unsigned Mipmaps() const						{ return m_uiMipmaps; }

			// NOTE: Format channels (1, 2, 3 or 4)

			inline unsigned Channels() const					{ return GetFormatInfo(m_format)->Channels(); }

			// NOTE: 1D/2D/3D sampler?

			inline bool Is1D() const							{ return (m_uiHeight == 1 && m_uiDepth == 1); }
			inline bool Is2D() const							{ return (m_uiHeight > 1 && m_uiDepth == 1); }
			inline bool Is3D() const							{ return (m_uiDepth > 1); }

			// NOTE: Target type

			inline bool Is1DArrayTarget() const					{ return m_target == Texture1dArray; }
			inline bool Is2DArrayTarget() const					{ return m_target == Texture2dArray; }
			inline bool IsCubeArrayTarget() const				{ return m_target == TextureCubeMapArray; }
			inline bool IsRectangleTarget() const				{ return m_target == TextureRectangle; }
			inline bool Is1DTarget() const						{ return m_target == Texture1d; }
			inline bool Is2DTarget() const						{ return m_target == Texture2d; }
			inline bool Is3DTarget() const						{ return m_target == Texture3d; }
			inline bool IsCubeTarget() const					{ return m_target == TextureCubeMap; }
			inline bool IsRenderBufferTarget() const			{ return m_target == RenderBuffer; }
			inline bool IsTextureBufferTarget() const			{ return m_target == TextureBuffer; }
			inline bool Is2DMultisampleTarget() const			{ return m_target == Texture2dMultisample; }
			inline bool Is2DArrayMultisampleTarget() const		{ return m_target == Texture2dMultisampleArray; }

			inline bool IsAnyCubeTarget() const					{ return IsCubeTarget() || IsCubeArrayTarget(); }
			inline bool IsArrayTarget() const					{ return Is1DArrayTarget() || Is2DArrayTarget() || IsCubeArrayTarget(); }
			inline bool IsMultisampleTarget() const				{ return Is2DMultisampleTarget() || Is2DArrayMultisampleTarget(); }
			inline bool IsStorage3D() const						{ return Is3DTarget() || Is2DArrayTarget() || IsCubeArrayTarget(); }

			// NOTE: Mipmapped width

			inline unsigned Width(unsigned mipmap = 0) const
			{
				unsigned w = m_uiWidth >> mipmap;
				return !w ? 1 : w;
			}

			// NOTE: Mipmapped height

			inline unsigned Height(unsigned mipmap = 0) const
			{
				unsigned h = m_uiHeight >> mipmap;
				return !h ? 1 : h;
			}

			// NOTE: Mipmapped depth

			inline unsigned Depth(unsigned mipmap = 0) const
			{
				if( !Is3DTarget() )
					return m_uiDepth;

				unsigned d = m_uiDepth >> mipmap;
				return !d ? 1 : d;
			}

			// NOTE: Mipmapped size in bytes

			unsigned MipmappedSize(unsigned firstMipMap = 0, unsigned mipmaps = AVAILABLE_MIPMAPS, Format format = FORMAT_NONE, unsigned w = 0, unsigned h = 0, unsigned d = 0) const;

			// NOTE: Mipmapped pixel count

			unsigned PixelCount(unsigned firstMipmap = 0, unsigned mipmaps = AVAILABLE_MIPMAPS) const;

			// NOTE: Get pixel (x % Width, y % Height)
			// IMPORTANT: Only for plain formats

			inline TR_BYTE* Pixel(unsigned x, unsigned y, unsigned mipmap = 0) const
			{
				unsigned w = Width(mipmap);

				x %= w;
				y %= Height(mipmap);

				return Pixels(mipmap) + (y * w + x) * GetFormatInfo(m_format)->Bytes();
			}

			// NOTE: Get pixel (x % Width, y % Height, z % Depth)
			// IMPORTANT: Only for plain formats

			inline TR_BYTE* Pixel(unsigned x, unsigned y, unsigned z, unsigned mipmap = 0) const
			{
				unsigned w = Width(mipmap);
				unsigned h = Height(mipmap);
				
				x %= w;
				y %= h;
				z %= Depth(mipmap);

				return Pixels(mipmap) + ((z * h + y) * w + x) * GetFormatInfo(m_format)->Bytes();
			}

			// NOTE: Get pixels
			// IMPORTANT: Returns NULL after last mipmap

			TR_BYTE* Pixels(unsigned mipmap = 0) const;
			
			// NOTE: w / h = 0			- means 'full current mipmap width / height'
			//       layerOrFace		- 'layer / z-offset' for texture arrays and 3D textures
			//							  'face' for cube maps (must be one from 0 to 5)
			//       USE_VIEWPORT_POS	- can be passed for x / y, in this case current viewport x / y will be used
			//       USE_VIEWPORT_SIZE	- can be passed for w / h, in this case current viewport width / height will be used

			// NOTE: CPU -> GPU or allocate on GPU

			bool Upload(unsigned samples = 0);
			bool UploadBatch(bool bInternal = false);

			// NOTE: CPU -> GPU

			// NOTE: update entire texture/buffer object
			//       pData == NULL - worker buffer will be used for transfer

			void Update(const void* pData);

			// NOTE: update entire layer or face in desired mipmap of texture object
			//       pData == NULL - worker buffer will be used for transfer

			void UpdateLayer(const void* pData, unsigned mipmap, unsigned layerOrFace);

			// NOTE: update region in layer or face of desired texture object mipmap
			//       pData == NULL - worker buffer will be used for transfer

			void UpdateRegion(const void* pData, unsigned mipmap, unsigned layerOrFace, unsigned x, unsigned y, unsigned w, unsigned h);
			void UpdateRegion(unsigned uiOffset, unsigned uiSize, unsigned mipmap, unsigned layerOrFace, unsigned x, unsigned y, unsigned w, unsigned h);

			// NOTE: GPU -> GPU

			void Transfer(unsigned mipmap, unsigned xOffset, unsigned yOffset, unsigned layerOrFace, unsigned x = 0, unsigned y = 0, unsigned w = 0, unsigned h = 0);

			// NOTE: GPU -> CPU (read with blocking call)

			void Readback(unsigned x = 0, unsigned y = 0, unsigned w = 0, unsigned h = 0, unsigned mipmap = 0);

			// NOTE: free GPU memory

			void Unload();

			// NOTE: free GPU and CPU memory

			inline void Free()
			{
				_FreeMemory();
				Unload();
				_Zero();
			}

			// NOTE: bind to specified texture unit

			void Bind(unsigned unit);

			// NOTE: bind to specified texture unit with special state

			void BindWithState(unsigned unit, PredefinedStateObject state);

			// NOTE: update mipmaps on GPU

			void UpdateMipmaps();

			// NOTE: attach buffer

			void AttachBuffer(cBuffer* pBuffer, unsigned uiSize = TR_UNKNOWN, unsigned uiOffset = 0);

			// NOTE: attach sampler

			inline void AttachState(const StateObject* pState)
			{
				if( !IsStateSet(KEEP_STATE) && m_pStateObject != pState )
				{
					assert( pState != NULL );

					// FIXME: useful, but textures are too crapy...
					//assert( (pState->IsMipmapped() && m_Mipmaps > 1) || !pState->IsMipmapped() );

					m_pStateObject = pState;
				}
			}

			void AttachState(PredefinedStateObject state);
			void Swizzle(const char* pcSwizzle4);

			// NOTE: Misc

			inline const sBTXBatch* GetBatch() const
			{
				assert( IsStateSet(BATCHED) );

				return ((sBTXBatch*)m_pucPixels) + m_uiCurrentBatch;
			}

			inline unsigned GetID() const
			{
				return m_glTexture;
			}

			inline Target GetTarget() const
			{
				return m_target;
			}

			inline void SetTarget(Target target)
			{
				m_target = target;
			}

			inline void RefIncr()
			{
				assert( m_iRef >= 0 );

				m_iRef++;
			}

			inline void RefDecr()
			{
				assert( m_iRef >= 1 );

				m_iRef--;
			}

			inline int GetRef()
			{
				assert( m_iRef >= 0 );

				return m_iRef;
			}

			inline unsigned GetMemSize() 
			{ 
				return ((m_uiSize_gpu > 0) ? m_uiSize_gpu : m_uiSize_cpu) + sizeof(*this); 
			}
	};
}
