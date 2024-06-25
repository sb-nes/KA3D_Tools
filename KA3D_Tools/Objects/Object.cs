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
        public float scale;
        public Utilities.float4 bias;

        public Utilities.float4[] vert0; // size of dataFormat
    }

    [StructLayout(LayoutKind.Sequential)]
    public class VertexComponent
    {
        public string vertexDataType; // Check if the prefix is 'DT_'
        public string vertexDataFormat; // Add the prefix 'DF_'
    }

    [StructLayout(LayoutKind.Sequential)]
    public class VertexFormat
    {
        public UInt16 vertexComponenetCount;
        public VertexComponent[] vertexComponents; 
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Primitive
    {
        public UInt32 vertices;
        public UInt32 indices;
        public VertexFormat vFormat;
        public UInt16 materialIndex;
        public UInt16 primitiveType;
        public VertexArray[] vertexArray;
        public UInt16[] indexData;
        public UInt16 usedBoneCount;
        public UInt16[] usedBoneArray;
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
        public Mesh[] meshes;

        public UInt32 cameraCount;
        public Camera[] cameras;

        public UInt32 lightCount;
        public Light[] lights;

        public UInt32 dummyCount;
        public Dummy[] dummies;

        public UInt32 shapeCount;
        public HGRShape[] shapes;

        public UInt32 otherNodeCount;

        public UInt32 transformAnimationCount;

        public UInt32 userPropertyCount;

    }

    class Object
    {
    }
}
