using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    /// <summary>
    /// HGR -> Hierarichal Graphics Library
    /// </summary>

    public class HGR : ViewModelBase
    {
        const int min_version = 170;
        const int max_version = 193;

        private string _data;
        public string Data
        {
            get => _data;
            set
            {
                if (_data != value)
                {
                    _data = value;
                    OnPropertyChanged(nameof(Data));
                }
            }
        }

        public string filePath;
        public UInt32 CheckID;

        // Read Check ID
        private void setCheckID(BinaryReader bw)
        {
            CheckID = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte());
        }
        private void checkUpdatedID(BinaryReader bw)
        {
            UInt32 tmp = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte());
            Debug.Assert(++CheckID == tmp);
        }

        private void readHeader(BinaryReader bw, HGR_Header head)
        {
            bw.ReadByte();
            head.m_ver = bw.ReadByte();
            Debug.Assert(head.m_ver > min_version && head.m_ver <= max_version);

            if(head.m_ver > 190) {
                head.m_exportedVer = bw.ReadByte();
                head.m_exportedVer = (head.m_exportedVer << 8) + bw.ReadByte();
                head.m_exportedVer = (head.m_exportedVer << 8) + bw.ReadByte();
                head.m_exportedVer = (head.m_exportedVer << 8) + bw.ReadByte();
            } else {
                head.m_exportedVer = 0x020903;
            }

            // Data Descriptor
            head.m_dataFlags = bw.ReadByte();
            head.m_dataFlags = (head.m_dataFlags << 8) + bw.ReadByte();

            if (head.m_ver > 180) {
                head.m_platformID = bw.ReadByte();
                head.m_platformID = (head.m_platformID << 8) + bw.ReadByte();
            }

            Data = head.m_ver + " " + head.m_exportedVer + " " + 
                   Convert.ToString(head.m_dataFlags & 0b00111111, 2) + 
                   " " + head.m_platformID;

            Data += "\n";
        }
        public Scene_Parameters readSceneParameters(BinaryReader bw)
        {
            Scene_Parameters scene_param = new Scene_Parameters();
            scene_param.fogType = (bw.ReadByte() == 1 ? "linear" : "none");

            scene_param.fogStart = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + 
                                          (bw.ReadByte() << 8) + bw.ReadByte();

            scene_param.fogEnd = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                        (bw.ReadByte() << 8) + bw.ReadByte();

            scene_param.fogColour = new float[3];
            for (int i = 0; i < 3; ++i)
            {
                scene_param.fogColour[i] = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                           (bw.ReadByte() << 8) + bw.ReadByte(); // Google conversion reveals a minor error in conversion
            }
            return scene_param;
        }

        private void readTextureData(BinaryReader bw, HGR_Data hgrData)
        {
            hgrData.textureCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) + 
                                                    (bw.ReadByte() << 8) + bw.ReadByte());
            hgrData.textures = new Texture[hgrData.textureCount];

            for (UInt32 i = 0; i < hgrData.textureCount; i++)
            {
                Texture tex = new Texture();
                int size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach (char ch in bw.ReadChars(size))
                {
                    tex.fileName += ch;
                }
                int type = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();
                tex.fileType = (type == 1 ? "cubemap" : "texture");
                hgrData.textures[i] = tex;
            }
        }

        private Texture_Parameter readTexParameter(BinaryReader bw)
        {
            Texture_Parameter texParam = new Texture_Parameter();
            // Parameter Type -> String
            int size = (bw.ReadByte() << 8) + bw.ReadByte();
            foreach (char ch in bw.ReadChars(size))
            {
                texParam.parameterType += ch;
            }
            texParam.textureIndex = Convert.ToUInt16((bw.ReadByte() << 8) + bw.ReadByte());

            return texParam;
        }
        private Vector4_Parameter readVec4Parameter(BinaryReader bw)
        {
            Vector4_Parameter vec4Param = new Vector4_Parameter();
            int size = (bw.ReadByte() << 8) + bw.ReadByte();
            foreach (char ch in bw.ReadChars(size)) {
                vec4Param.parameterType += ch;
            }
            float[] value = new float[4];
            for (int i = 0; i < 4; i++) {
                value[i] = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();
            }
            vec4Param.value = value;

            return vec4Param;
        }
        private Float_Parameter readFloatParameter(BinaryReader bw)
        {
            Float_Parameter fparam = new Float_Parameter();
            int size = (bw.ReadByte() << 8) + bw.ReadByte();
            foreach (char ch in bw.ReadChars(size))
            {
                fparam.parameterType += ch;
            }
            fparam.value = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();

            return fparam;
        }

        private void readMaterialData(BinaryReader bw, HGR_Data hgrData)
        {
            hgrData.materialCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                               (bw.ReadByte() << 8) + bw.ReadByte());
            hgrData.materials = new Material[hgrData.materialCount];

            for (UInt32 i = 0; i < hgrData.materialCount; i++)
            {
                Material mat = new Material();
                int size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach (char ch in bw.ReadChars(size)) {
                    mat.name += ch;
                }
                size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach (char ch in bw.ReadChars(size)) {
                    mat.shaderName += ch;
                }

                // lightmap should be used by Shader -> bit 0 set
                mat.shaderLightMap = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();

                mat.textureParameterCount = bw.ReadByte();
                mat.textureParameters = new Texture_Parameter[mat.textureParameterCount];
                for (UInt16 j = 0; j < mat.textureParameterCount; ++j) {
                    mat.textureParameters[j] = readTexParameter(bw);
                }

                mat.Vector4ParameterCount = bw.ReadByte();
                mat.vector4Parameters = new Vector4_Parameter[mat.Vector4ParameterCount];
                for (UInt16 j = 0; j < mat.Vector4ParameterCount; ++j)
                {
                    mat.vector4Parameters[j] = readVec4Parameter(bw);
                }

                mat.FloatParameterCount = bw.ReadByte();
                mat.floatParameters = new Float_Parameter[mat.FloatParameterCount];
                for (UInt16 j = 0; j < mat.FloatParameterCount; ++j)
                {
                    mat.floatParameters[j] = readFloatParameter(bw);
                }

                hgrData.materials[i] = mat;
            }
        }

        // Contains Unimplemented Data-Type read methods
        private VertexArray readVertexArray(BinaryReader bw, string df, UInt32 vertices)
        {
            VertexArray vArray = new VertexArray();
            vArray.scale = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                           (bw.ReadByte() << 8) + bw.ReadByte();
            vArray.bias = new Utilities.float4();
            vArray.bias.x = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                            (bw.ReadByte() << 8) + bw.ReadByte();
            vArray.bias.y = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                            (bw.ReadByte() << 8) + bw.ReadByte();
            vArray.bias.z = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                            (bw.ReadByte() << 8) + bw.ReadByte();

            vArray.vert0 = new Utilities.float4[vertices];
            switch (df)
            {
                case "DF_S_16":
                    for (int i = 0; i < vertices; ++i)
                    {
                        vArray.vert0[i] = new Utilities.float4();
                        vArray.vert0[i].x = (bw.ReadByte() << 8) + bw.ReadByte();
                    }
                    break;

                case "DF_V2_16":
                    for (int i = 0; i < vertices; ++i)
                    {
                        vArray.vert0[i] = new Utilities.float4();
                        vArray.vert0[i].x = (bw.ReadByte() << 8) + bw.ReadByte();
                        vArray.vert0[i].y = (bw.ReadByte() << 8) + bw.ReadByte();
                    }
                    break;

                case "DF_V3_16":
                    for (int i = 0; i < vertices; ++i)
                    {
                        vArray.vert0[i] = new Utilities.float4();
                        vArray.vert0[i].x = (bw.ReadByte() << 8) + bw.ReadByte();
                        vArray.vert0[i].y = (bw.ReadByte() << 8) + bw.ReadByte();
                        vArray.vert0[i].z = (bw.ReadByte() << 8) + bw.ReadByte();
                    }
                    break;

                case "DF_V4_16":
                    vArray.bias.w = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                    (bw.ReadByte() << 8) + bw.ReadByte();
                    for (int i = 0; i < vertices; ++i)
                    {
                        vArray.vert0[i] = new Utilities.float4();
                        vArray.vert0[i].x = (bw.ReadByte() << 8) + bw.ReadByte();
                        vArray.vert0[i].y = (bw.ReadByte() << 8) + bw.ReadByte();
                        vArray.vert0[i].z = (bw.ReadByte() << 8) + bw.ReadByte();
                        vArray.vert0[i].w = (bw.ReadByte() << 8) + bw.ReadByte();
                    }
                    break;

                default:
                    Data = "Unimplemented type: " + df;
                    break;
            }

            // v = v0 * scale + bias

            return vArray;
        }

        private VertexFormat readVertexFormat(BinaryReader bw)
        {
            VertexFormat vForm = new VertexFormat();
            vForm.vertexComponenetCount = bw.ReadByte();
            vForm.vertexComponents = new VertexComponent[vForm.vertexComponenetCount];

            for (UInt16 i = 0; i < vForm.vertexComponenetCount; ++i)
            {
                VertexComponent vComp = new VertexComponent();
                int size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach (char ch in bw.ReadChars(size))
                {
                    vComp.vertexDataType += ch;
                }
                size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach (char ch in bw.ReadChars(size)) // This is without 'DF_'
                {
                    vComp.vertexDataFormat += ch;
                }
                vComp.vertexDataFormat = "DF_" + vComp.vertexDataFormat;

                vForm.vertexComponents[i] = vComp;
            }

            return vForm;
        }

        private void readPrimitiveData(BinaryReader bw, HGR_Data hgrData)
        {
            hgrData.primitiveCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                      (bw.ReadByte() << 8) + bw.ReadByte());
            hgrData.primitives = new Primitive[hgrData.primitiveCount];
            for (UInt32 i = 0; i < hgrData.primitiveCount; i++)
            {
                Primitive prim = new Primitive();
                prim.vertices = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                 (bw.ReadByte() << 8) + bw.ReadByte());
                prim.indices = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                 (bw.ReadByte() << 8) + bw.ReadByte());
                prim.vFormat = readVertexFormat(bw);
                prim.materialIndex = Convert.ToUInt16((bw.ReadByte() << 8) + bw.ReadByte());
                prim.primitiveType = Convert.ToUInt16((bw.ReadByte() << 8) + bw.ReadByte()); // Refer PrimType in Useless

                prim.vertexArray = new VertexArray[prim.vFormat.vertexComponenetCount];
                for (UInt32 j = 0; j<prim.vFormat.vertexComponenetCount; ++j)
                {
                    prim.vertexArray[j] = readVertexArray(bw, prim.vFormat.vertexComponents[j].vertexDataFormat, prim.vertices);
                }

                prim.indexData = new UInt16[prim.indices];
                for (UInt32 j = 0; j < prim.indices; ++j)
                {
                    prim.indexData[j] = Convert.ToUInt16((bw.ReadByte() << 8) + bw.ReadByte());
                }

                prim.usedBoneCount = bw.ReadByte();
                prim.usedBoneArray = new UInt16[prim.usedBoneCount];
                for (UInt16 j = 0; j < prim.usedBoneCount; j++)
                {
                    prim.usedBoneArray[j] = bw.ReadByte();
                }

                hgrData.primitives[i] = prim;
            }
        }

        private Node readNodeData(BinaryReader bw)
        {
            Node node = new Node();

            int size = (bw.ReadByte() << 8) + bw.ReadByte();
            foreach (char ch in bw.ReadChars(size))
            {
                node.name += ch;
            }

            node.modeltm = new Utilities.float3x4();

            node.modeltm.a = new Utilities.float4();
            node.modeltm.a.x = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.a.y = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.a.z = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.a.w = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();

            node.modeltm.b = new Utilities.float4();
            node.modeltm.b.x = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.b.y = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.b.z = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.b.w = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();

            node.modeltm.c = new Utilities.float4();
            node.modeltm.c.x = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.c.y = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.c.z = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
            node.modeltm.c.w = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();

            node.nodeFlags = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                              (bw.ReadByte() << 8) + bw.ReadByte());

            // Some unidentified data of size 4 bytes
            int uid = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                      (bw.ReadByte() << 8) + bw.ReadByte();

            node.parentIndex = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();

            return node;
        }

        private MeshBone readMeshBoneData(BinaryReader bw)
        {
            MeshBone meshBone = new MeshBone();

            meshBone.boneNodeIndex = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                      (bw.ReadByte() << 8) + bw.ReadByte());

            meshBone.invresttm = new Utilities.float3x4();

            meshBone.invresttm.a = new Utilities.float4();
            meshBone.invresttm.a.x = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.a.y = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.a.z = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.a.w = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();

            meshBone.invresttm.b = new Utilities.float4();
            meshBone.invresttm.b.x = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.b.y = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.b.z = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.b.w = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();

            meshBone.invresttm.c = new Utilities.float4();
            meshBone.invresttm.c.x = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.c.y = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.c.z = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();
            meshBone.invresttm.c.w = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                     (bw.ReadByte() << 8) + bw.ReadByte();

            return meshBone;
        }

        private void readMeshData(BinaryReader bw, HGR_Data hgrData)
        {
            hgrData.meshCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                 (bw.ReadByte() << 8) + bw.ReadByte());
            hgrData.meshes = new Mesh[hgrData.meshCount];
            for (UInt32 i = 0; i < hgrData.meshCount; ++i)
            {
                Mesh mesh = new Mesh();
                mesh.node = readNodeData(bw);

                mesh.primitiveCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                       (bw.ReadByte() << 8) + bw.ReadByte());
                mesh.primitiveIndices = new UInt32[mesh.primitiveCount];
                for (UInt32 j = 0; j < mesh.primitiveCount; ++j)
                {
                    mesh.primitiveIndices[j] = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                                (bw.ReadByte() << 8) + bw.ReadByte()); ;
                }

                mesh.boneCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                  (bw.ReadByte() << 8) + bw.ReadByte());
                mesh.bones = new MeshBone[mesh.boneCount];

                for (UInt32 j = 0; j< mesh.boneCount; ++j)
                {
                    mesh.bones[j] = readMeshBoneData(bw);
                }

                hgrData.meshes[i] = mesh;
            }
        }

        private void readCameraData(BinaryReader bw, HGR_Data hgrData)
        {
            hgrData.cameraCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                   (bw.ReadByte() << 8) + bw.ReadByte());
            hgrData.cameras = new Camera[hgrData.cameraCount];
            for (UInt32 i = 0; i < hgrData.cameraCount; ++i)
            {
                Camera camera = new Camera();

                camera.front = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                               (bw.ReadByte() << 8) + bw.ReadByte();
                camera.back = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                              (bw.ReadByte() << 8) + bw.ReadByte();
                camera.horzFOV = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                 (bw.ReadByte() << 8) + bw.ReadByte();

                hgrData.cameras[i] = camera;
            }
        }

        public void readHGR()
        {
            var file = File.Open(filePath, FileMode.Open, FileAccess.Read);
            HGR_Data hgrData = new HGR_Data();
            hgrData.header = new HGR_Header();
            using (var bw = new BinaryReader(file))
            {
                Debug.Assert(string.Concat(bw.ReadChars(4)) == "hgrf");
                readHeader(bw, hgrData.header);
                hgrData.sceneParam = readSceneParameters(bw);

                setCheckID(bw); //0x12345600

                readTextureData(bw, hgrData);
                readMaterialData(bw, hgrData);

                checkUpdatedID(bw); //0x12345601

                readPrimitiveData(bw, hgrData);
                checkUpdatedID(bw); //0x12345602

                readMeshData(bw, hgrData);
                checkUpdatedID(bw); //0x12345603

                readCameraData(bw, hgrData);
                checkUpdatedID(bw); //0x12345604

                //lights
                //checkUpdatedID(bw); //0x12345605

                //dummies
                //checkUpdatedID(bw); //0x12345606

                //shapes
                //checkUpdatedID(bw); //0x12345607

                //other nodes
                //checkUpdatedID(bw); //0x12345608

                //transform animations
                //checkUpdatedID(bw); //0x12345609

                //user properties
            }
        }
    }
}
