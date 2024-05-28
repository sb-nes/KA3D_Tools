using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    public enum ImageFileType
    {
        PNG,
        JPG,
        TIFF,
        BMP,
    }

	enum SurfaceFormat
	{
		/** The surface format is unknown. */
		SURFACE_UNKNOWN,
		/** 24-bit RGB pixel format. */
		SURFACE_R8G8B8,
		/** 24-bit RGB pixel format. */
		SURFACE_B8G8R8,
		/** 32-bit RGB pixel format with alpha. */
		SURFACE_A8R8G8B8,
		/** 32-bit RGB pixel format where 8 bits are reserved for each color. */
		SURFACE_X8R8G8B8,
		/** 32-bit RGB pixel format where 8 bits are reserved for each color.  */
		SURFACE_X8B8G8R8,
		/** 32-bit RGB pixel format with alpha.  */
		SURFACE_A8B8G8R8,
		/** 16-bit RGB pixel format. (PS2) */
		SURFACE_R5G6B5,
		/** 16-bit RGB pixel format. */
		SURFACE_R5G5B5,
		/** 18-bit RGB pixel format (32bits per pixel). */
		SURFACE_R6G6B6,
		/** 4-bit palettized pixel format. (PC/PS2) */
		SURFACE_P4,
		/** 8-bit palettized pixel format. (PC/PS2)	*/
		SURFACE_P8,
		/** 8-bit luminosity format. (PC/PS2) */
		SURFACE_L8,
		/** 16-bit pixel format where 5 bits are reserved for color and 1 bit is reserved for transparency. */
		SURFACE_A1R5G5B5,
		/** 16-bit RGB pixel format where 4 bits are reserved for each color. */
		SURFACE_X4R4G4B4,
		/** 16-bit RGBA pixel format. */
		SURFACE_A4R4G4B4,
		/** 16-bit RGBA pixel format. */
		SURFACE_A4B4G4R4,
		/** 16-bit RGBA pixel format. */
		SURFACE_R4G4B4A4,
		/** 16-bit pixel format where 5 bits are reserved for color and 1 bit is reserved for transparency. */
		SURFACE_A1B5G5R5,
		/** 16-bit pixel format where 5 bits are reserved for color and 1 bit is reserved for transparency. */
		SURFACE_R5G5B5A1,
		/** 8-bit RGB texture format. (PS2) */
		SURFACE_R3G3B2,
		/** 8-bit RGB texture format. */
		SURFACE_R3G2B3,
		/** 8-bit alpha-only. */
		SURFACE_A8,
		/** 16-bit RGB pixel format with alpha.	*/
		SURFACE_A8R3G3B2,
		/** 16-bit RGB pixel format with alpha.	*/
		SURFACE_A8R3G2B3,
		/** DirectX compressed texture */
		SURFACE_DXT1,
		/** DirectX compressed texture */
		SURFACE_DXT3,
		/** DirectX compressed texture */
		SURFACE_DXT5,
		/** 16-bit float format, 16 bits red channel. */
		SURFACE_R16F,
		/** 32-bit float format, 16 bits red and green channels. */
		SURFACE_G16R16F,
		/** 64-bit float format, 16 bits for the alpha, blue, green, red. */
		SURFACE_A16B16G16R16F,
		/** 32-bit float format, 32 bits red channel. */
		SURFACE_R32F,
		/** 64-bit float format, 32 bits red and green channels. */
		SURFACE_G32R32F,
		/** 128-bit float format, 32 bits for the alpha, blue, green, red. */
		SURFACE_A32B32G32R32F,
		/** 32-bit depth buffer format */
		SURFACE_D32,
		/** 24-bit depth buffer format */
		SURFACE_D24,
		/** 16-bit depth buffer format */
		SURFACE_D16,
		/** 32-bit depth buffer format, depth using 24 bits and stencil 8 bits */
		SURFACE_D24S8,
		/** Surface format list terminator */
		SURFACE_LAST,
	};

	public class KA3D_Image : IDisposable
	{
		public string[] FORMAT_NAMES = // it should be constant
		{
			/** The surface format is unknown. */
			"UNKNOWN",
			/** 24-bit RGB pixel format. */
			"R8G8B8",
			/** 24-bit RGB pixel format. */
			"B8G8R8",
			/** 32-bit RGB pixel format with alpha. */
			"A8R8G8B8",
			/** 32-bit RGB pixel format where 8 bits are reserved for each color. */
			"X8R8G8B8",
			/** 32-bit RGB pixel format where 8 bits are reserved for each color.  */
			"X8B8G8R8",
			/** 32-bit RGB pixel format with alpha.  */
			"A8B8G8R8",
			/** 16-bit RGB pixel format. (PS2) */
			"R5G6B5",
			/** 16-bit RGB pixel format. */
			"R5G5B5",
			/** 18-bit RGB pixel format. */
			"R6G6B6",
			/** 4-bit palettized pixel format. (PC/PS2) */
			"P4",
			/** 8-bit palettized pixel format. (PC/PS2)	*/
			"P8",
			/** 8-bit luminosity format. (PC/PS2) */
			"L8",
			/** 16-bit pixel format where 5 bits are reserved for color and 1 bit is reserved for transparency. */
			"A1R5G5B5",
			/** 16-bit RGB pixel format where 4 bits are reserved for each color. */
			"X4R4G4B4",
			/** 16-bit RGBA pixel format. */
			"A4R4G4B4",
			/** 16-bit RGBA pixel format. */
			"A4B4G4R4",
			/** 16-bit RGBA pixel format. */
			"R4G4B4A4",
			/** 16-bit pixel format where 5 bits are reserved for color and 1 bit is reserved for transparency. */
			"A1B5G5R5",
			/** 16-bit pixel format where 5 bits are reserved for color and 1 bit is reserved for transparency. */
			"R5G5B5A1",
			/** 8-bit RGB texture format. (PS2) */
			"R3G3B2",
			/** 8-bit RGB texture format. */
			"R3G2B3",
			/** 8-bit alpha-only. */
			"A8",
			/** 16-bit RGB pixel format with alpha.	*/
			"A8R3G3B2",
			/** 16-bit RGB pixel format with alpha.	*/
			"A8R3G2B3",
			/** DirectX compressed texture */
			"DXT1",
			/** DirectX compressed texture */
			"DXT3",
			/** DirectX compressed texture */
			"DXT5",
			/** 16-bit float format, 16 bits red channel. */
			"R16F",
			/** 32-bit float format, 16 bits red and green channels. */
			"G16R16F",
			/** 64-bit float format, 16 bits for the alpha, blue, green, red. */
			"A16B16G16R16F",
			/** 32-bit float format, 32 bits red channel. */
			"R32F",
			/** 64-bit float format, 32 bits red and green channels. */
			"G32R32F",
			/** 128-bit float format, 32 bits for the alpha, blue, green, red. */
			"A32B32G32R32F",
			/** 32-bit depth buffer format */
			"D32",
			/** 24-bit depth buffer format */
			"D24",
			/** 16-bit depth buffer format */
			"D16",
			/** "32", depth using 24 bits and stencil 8 bits */
			"D24S8",
		};

		/// Descriptions of surface formats: 
		///	{format, bitcount, red mask, green mask, blue mask, alpha mask}. 

		public UInt32[,] FORMAT_DESC =
		{
			{ 0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000},
			{24, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000},
			{32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000},
			{32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000},
			{32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000},
			{32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000},
			{16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000},
			{16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00000000},
			{32, (0x3F << 12),  (0x3F << 6),     (0x3F), 0x00000000},
			{ 4, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ 8, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ 8, 0x000000FF, 0x000000FF, 0x000000FF, 0x00000000},
			{16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000},
			{16, 0x00000f00, 0x000000f0, 0x0000000f, 0x00000000},
			{16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000},
			{16, 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000},
			{16, 0x0000f000, 0x00000f00, 0x000000f0, 0x0000000f},
			{16, 0x0000001f, 0x000003e0, 0x00007c00, 0x00008000},
			{16, (0x1F << 11),  (0x1F << 6),    (0x1F << 1),    0x00000001},
			{ 8, 0x000000e0, 0x0000001c, 0x00000003, 0x00000000},
			{ 8, 0x000000e0, 0x00000018, 0x00000007, 0x00000000},
			{ 8, 0x00000000, 0x00000000, 0x00000000, 0x000000ff},
			{16, 0x000000e0, 0x0000001c, 0x00000003, 0x0000ff00},
			{16, 0x000000e0, 0x00000018, 0x00000007, 0x0000ff00},
			{ 0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ 0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ 0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{16, 0,0,0,0},
			{32, 0,0,0,0},
			{64, 0,0,0,0},
			{32, 0,0,0,0},
			{64, 0,0,0,0},
			{128, 0,0,0,0},
			{32, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{24, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{16, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{32, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
		};

        public void Dispose()
        {
            
        }

        /*
		int[,] FORMAT_DESC = 
		{
			{ SurfaceFormat::SURFACE_UNKNOWN   ,  0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_R8G8B8    , 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000},
			{ SurfaceFormat::SURFACE_B8G8R8    , 24, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000},
			{ SurfaceFormat::SURFACE_A8R8G8B8  , 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000},
			{ SurfaceFormat::SURFACE_X8R8G8B8  , 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000},
			{ SurfaceFormat::SURFACE_X8B8G8R8  , 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000},
			{ SurfaceFormat::SURFACE_A8B8G8R8  , 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000},
			{ SurfaceFormat::SURFACE_R5G6B5    , 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000},
			{ SurfaceFormat::SURFACE_R5G5B5    , 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00000000},
			{ SurfaceFormat::SURFACE_R6G6B6    , 32, (0x3F << 12),  (0x3F << 6),     (0x3F), 0x00000000},
			{ SurfaceFormat::SURFACE_P4       ,  4,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_P8       ,  8,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_L8       ,  8,  0x000000FF, 0x000000FF, 0x000000FF, 0x00000000},
			{ SurfaceFormat::SURFACE_A1R5G5B5  , 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000},
			{ SurfaceFormat::SURFACE_X4R4G4B4  , 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x00000000},
			{ SurfaceFormat::SURFACE_A4R4G4B4  , 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000},
			{ SurfaceFormat::SURFACE_A4B4G4R4  , 16, 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000},
			{ SurfaceFormat::SURFACE_R4G4B4A4  , 16, 0x0000f000, 0x00000f00, 0x000000f0, 0x0000000f},
			{ SurfaceFormat::SURFACE_A1B5G5R5  , 16, 0x0000001f, 0x000003e0, 0x00007c00, 0x00008000},
			{ SurfaceFormat::SURFACE_R5G5B5A1  , 16, (0x1F << 11),	(0x1F << 6),	(0x1F << 1),	0x00000001},
			{ SurfaceFormat::SURFACE_R3G3B2    ,  8, 0x000000e0, 0x0000001c, 0x00000003, 0x00000000},
			{ SurfaceFormat::SURFACE_R3G2B3    ,  8, 0x000000e0, 0x00000018, 0x00000007, 0x00000000},
			{ SurfaceFormat::SURFACE_A8        ,  8, 0x00000000, 0x00000000, 0x00000000, 0x000000ff},
			{ SurfaceFormat::SURFACE_A8R3G3B2  , 16, 0x000000e0, 0x0000001c, 0x00000003, 0x0000ff00},
			{ SurfaceFormat::SURFACE_A8R3G2B3  , 16, 0x000000e0, 0x00000018, 0x00000007, 0x0000ff00},
			{ SurfaceFormat::SURFACE_DXT1      , 0,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_DXT3      , 0,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_DXT5      , 0,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_R16F,			16, 0,0,0,0},
			{ SurfaceFormat::SURFACE_G16R16F,		32, 0,0,0,0},
			{ SurfaceFormat::SURFACE_A16B16G16R16F,	64, 0,0,0,0},
			{ SurfaceFormat::SURFACE_R32F,			32, 0,0,0,0},
			{ SurfaceFormat::SURFACE_G32R32F,		64, 0,0,0,0},
			{ SurfaceFormat::SURFACE_A32B32G32R32F, 128, 0,0,0,0},
			{ SurfaceFormat::SURFACE_D32       , 32, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_D24       , 24, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_D16       , 16, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
			{ SurfaceFormat::SURFACE_D24S8     , 32, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
		};
		*/
    }


	public class Image
    {
    }
}
