using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    public class Useless
    {
        public const int MAX_BONES = 255;
        public const int MAX_TEXCOORDS = 4;

        public string[] PlatformName =
        {
            "dx",
            "egl",
            "psp",
            "sw",
            "n3d"
        };

        public string[] PlatformDesc =
        {
            "DirectX 9.x",
            "OpenGL ES",
            "PlayStation Portable",
            "Software Renderer",
            "Nokia N3D"
        };

        public enum DataFlags
        {
            DATA_MATERIALS                  = 1,
            DATA_PRIMITIVES                 = 2,
            DATA_NODES                      = 4,
            DATA_ANIMATIONS                 = 8,
            DATA_USERPROPERTIES             = 16,

        }
        public enum PrimType
        {
            PRIM_POINT = 0,
            PRIM_LINE = 1,
            PRIM_LINESTRIP = 2,
            PRIM_TRI = 3,
            PRIM_TRISTRIP = 4,
            PRIM_TRIFAN = 5,
            PRIM_SPRITE = 6,
            PRIM_INVALID = 7
        };
        public enum DataType
        {
            /** Vertex has model space position data. */
            DT_POSITION,
            /** Vertex has screen space position data. */
            DT_POSITIONT,
            /** Vertex has bone weights used in skinning. */
            DT_BONEWEIGHTS,
            /** Vertex has bone indices used in skinning. */
            DT_BONEINDICES,
            /** Vertex has normal pointing away from surface. */
            DT_NORMAL,
            /** Vertex has diffuse color data. */
            DT_DIFFUSE,
            /** Vertex has specular color data. */
            DT_SPECULAR,
            /** Vertex has texture layer 0. */
            DT_TEX0,
            /** Vertex has texture layer 1. */
            DT_TEX1,
            /** Vertex has texture layer 2. */
            DT_TEX2,
            /** Vertex has texture layer 3. */
            DT_TEX3,
            /** Vertex has tangent data. */
            DT_TANGENT,
            /** Number of different vertex component types. */
            DT_SIZE
        };

        public int getDataSizeFromVertexDataFormat(string df)
        {
            switch (df)
            {
                case "DF_S_32": return 4;
                case "DF_S_16": return 2;
                case "DF_S_8": return 1;
                case "DF_V2_32": return 8;
                case "DF_V2_16": return 4;
                case "DF_V2_8": return 2;
                case "DF_V3_32": return 12;
                case "DF_V3_16": return 6;
                case "DF_V3_8": return 3;
                case "DF_V4_32": return 16;
                case "DF_V4_16": return 8;
                case "DF_V4_8": return 4;
                case "DF_V4_5": return 2;
                case "DF_NONE": return 0;
                case "DF_SIZE": return 0;
            }
            return 0;
        }

        public int getDataDimFromVertexDataFormat(string df)
        {
            switch (df)
            {
                case "DF_S_32": return 1;
                case "DF_S_16": return 1;
                case "DF_S_8": return 1;
                case "DF_V2_32": return 2;
                case "DF_V2_16": return 2;
                case "DF_V2_8": return 2;
                case "DF_V3_32": return 3;
                case "DF_V3_16": return 3;
                case "DF_V3_8": return 3;
                case "DF_V4_32": return 4;
                case "DF_V4_16": return 4;
                case "DF_V4_8": return 4;
                case "DF_V4_5": return 4;
                case "DF_NONE": return 0;
                case "DF_SIZE": return 0;
            }
            return 0;
        }
    }
}
