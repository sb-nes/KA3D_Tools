using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    [StructLayout(LayoutKind.Sequential)]
    public class HGR_Header
    {
        public int m_ver;
        public int m_exportedVer;
        public int m_dataFlags;
        public int m_platformID;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Scene_Parameters
    {
        public string fogType;
        public float fogStart;
        public float fogEnd;
        public float[] fogColour;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Texture
    {
        public string fileName;
        public string fileType;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Texture_Parameter
    {
        public string parameterType;
        public UInt16 textureIndex;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Vector4_Parameter
    {
        public string parameterType;
        public float[] value = new float[4];
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Float_Parameter
    {
        public string parameterType;
        public float value;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Material
    {
        public string name;
        public string shaderName;
        public int shaderLightMap;
        public UInt16 textureParameterCount;
        public Texture_Parameter[] textureParameters;
        public UInt16 Vector4ParameterCount;
        public Vector4_Parameter[] vector4Parameters;
        public UInt16 FloatParameterCount;
        public Float_Parameter[] floatParameters;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class VertexArray
    {

    }

    [StructLayout(LayoutKind.Sequential)]
    public class VertexFormat
    {

    }

    [StructLayout(LayoutKind.Sequential)]
    public class Primitive
    {

    }

    [StructLayout(LayoutKind.Sequential)]
    public class HGR_Data
    {
        public HGR_Header header;
        public Scene_Parameters sceneParam;

        public UInt32 textureCount;
        public Texture[] textures;

        public UInt32 materialCount;
        public Material[] materials;

        public UInt32 primitiveCount;
        public Primitive[] primitives;

        public UInt32 meshCount;
        public UInt32 cameraCount;
        public UInt32 lightCount;
        public UInt32 dummyCount;
        public UInt32 shapeCount;
        public UInt32 otherNodeCount;
        public UInt32 transformAnimationCount;
        public UInt32 userPropertyCount;
    }

    class Object
    {
    }
}
