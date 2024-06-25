using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using KA3D_Tools.Utilities;

namespace KA3D_Tools
{
    [StructLayout(LayoutKind.Sequential)]
    public class Node
    {
        public String name;
        public float3x4 modeltm;
        public UInt32 nodeFlags;
        public int parentIndex;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class MeshBone
    {
        public UInt32 boneNodeIndex;
        public float3x4 invresttm;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Mesh
    {
        public Node node;
        public UInt32 primitiveCount;
        public UInt32[] primitiveIndices;
        public UInt32 boneCount;
        public MeshBone[] bones; // Gimme x3 Some Time
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Camera
    {
        public Node node;
        public float front;
        public float back;
        public float horzFOV; // radians
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Light
    {
        public Node node;
        public float3x4 color;
        public float reserved1;
        public float reserved2;
        public float farAttenStart;
        public float farAttenEnd;
        public float inner;
        public float outer;
        public UInt16 type;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class Dummy
    {
        public Node node;
        public float3x4 boxMin;
        public float3x4 boxMax;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class ShapeLine
    {
        public float3x4 start;
        public float3x4 end;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class ShapePath
    {
        public Int32 beginLine;
        public Int32 endLine;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class HGRShape
    {
        public Node node;
        public Int32 lineCount;
        public Int32 pathCount;
        public ShapeLine[] lines;
        public ShapePath[] paths;
    }

    class Nodes
    {
    }
}
