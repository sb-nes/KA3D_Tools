using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using Assimp;

namespace KA3D_Tools
{
    /// <summary>
    /// Final Conversion to FBX:
    /// HGR interpreted data is converted to IntPtr (Native Pointer) using MemoryHelper Structure.
    /// Assimp then converts it to FBX!
    /// </summary>
    public class Exporter
    {
        private AssimpC.Scene baseHGR;
        public ExportFormatDescription[] formatIds;
        public string inputPath;

        private Matrix4x4 copyTransform(Utilities.float3x4 modelTm)

        {
            Matrix4x4 transform = new Matrix4x4(modelTm.a.x, modelTm.a.y, modelTm.a.z, modelTm.a.w, 
                                                modelTm.b.x, modelTm.b.y, modelTm.b.z, modelTm.b.w, 
                                                modelTm.c.x, modelTm.c.y, modelTm.c.z, modelTm.c.w, 
                                                0, 0, 0, 1);
            return transform;
        }

        public List<AssimpC.Mesh> StoreMesh(HGR_Data hgrData)
        {
            List<AssimpC.Mesh> meshes = new List<AssimpC.Mesh>();

            for(int i = 0; i < hgrData.meshes.Length; ++i)
            {
                AssimpC.Mesh mesh = new AssimpC.Mesh();
                AssimpC.Node item = new AssimpC.Node();

                item.Name = hgrData.meshes[i].node.name;
                //item.Children = new AssimpC.NodeCollection(item);
                item.Transform = copyTransform(hgrData.meshes[i].node.modeltm);

                foreach (var index in hgrData.meshes[i].primitiveIndices) {
                    item.MeshIndices.Add(Convert.ToInt32(index));
                }

                var parIndx = hgrData.meshes[i].node.parentIndex;

                item.Parent = baseHGR.RootNode;
                baseHGR.RootNode.Children.Add(item);

                /*
                if (parIndx == -1) {
                    item.Parent = baseHGR.RootNode;
                    baseHGR.RootNode.Children.Add(item);
                } else {
                    item.Parent = parIndx;
                }
                */

                foreach (var primIndex in hgrData.meshes[i].primitiveIndices)
                {
                    mesh.Name = hgrData.meshes[i].node.name;

                    switch (hgrData.primitives[primIndex].primitiveType)
                    {
                        case 0:
                        mesh.PrimitiveType = PrimitiveType.Point;
                        break;

                        case 1:
                        mesh.PrimitiveType = PrimitiveType.Line;
                        break;

                        case 3:
                        mesh.PrimitiveType = PrimitiveType.Triangle;
                        break;

                        case 6:
                        mesh.PrimitiveType = PrimitiveType.Polygon;
                        break;

                        default:
                        mesh.PrimitiveType = PrimitiveType.Polygon;
                        break;
                    }

                    mesh.MaterialIndex = hgrData.primitives[primIndex].materialIndex;
                    uint verts = hgrData.primitives[primIndex].vertices;

                    var temp = hgrData.primitives[primIndex].vertexArray;
                    mesh.Vertices = new List<Vector3D>(); 
                    foreach (var vertData in temp[0].vert0) // Vertices
                    {
                        var vert = new Vector3D((vertData.x * temp[0].scale) + temp[0].bias.x, 
                                                (vertData.y * temp[0].scale) + temp[0].bias.y, 
                                                (vertData.z * temp[0].scale) + +temp[0].bias.z);
                        mesh.Vertices.Add(vert);
                    }
                    Debug.Assert(mesh.VertexCount == verts);

                    mesh.TextureCoordinateChannels = new List<Vector3D>[1]; //Channel 0 is default for UV
                    mesh.TextureCoordinateChannels[0] = new List<Vector3D>();
                    
                    foreach (var vertData in temp[1].vert0) // Tex_Coords
                    {
                        var vert = new Vector3D((vertData.x * temp[1].scale) + temp[1].bias.x, 
                                                (vertData.y * temp[1].scale) + temp[1].bias.y, 
                                                0);
                        mesh.TextureCoordinateChannels[0].Add(vert);
                    }
                    
                    mesh.Faces = new List<AssimpC.Face>(); // Faces / Indices
                    switch (mesh.PrimitiveType)
                    {
                        case PrimitiveType.Triangle:
                        for (int j = 0; j < hgrData.primitives[primIndex].indices; ++j)
                            {
                                var face = new AssimpC.Face();

                                face.Indices.Add(hgrData.primitives[primIndex].indexData[j++]);
                                face.Indices.Add(hgrData.primitives[primIndex].indexData[j++]);
                                face.Indices.Add(hgrData.primitives[primIndex].indexData[j]);

                                mesh.Faces.Add(face);
                            }
                        break;

                        default:
                        Debug.WriteLine("Unimplemented Type");
                        break;
                    }

                    meshes.Add(mesh);
                }
                mesh.UVComponentCount[0] = 2;
            }

            return meshes;
        }
        public List<AssimpC.Material> StoreMaterial(Material[] hgrMaterials)
        {
            List<AssimpC.Material> materials = new List<AssimpC.Material>();

            for(int i = 0; i < hgrMaterials.Length; ++i)
            {
                AssimpC.Material mat = new AssimpC.Material(); // i'm thinking to keep the rest default
                mat.Name = hgrMaterials[i].name;
                mat.IsTwoSided = false;
                mat.ShadingMode = ShadingMode.NoShading;

                float[] vec4;
                vec4 = hgrMaterials[i].vector4Parameters[0].value; // Ambient Colour
                mat.ColorAmbient = new Color4D(vec4[0], vec4[1], vec4[2], vec4[3]);
                vec4 = hgrMaterials[i].vector4Parameters[0].value; // Diffuse Colour
                mat.ColorDiffuse = new Color4D(vec4[0], vec4[1], vec4[2], vec4[3]);
                vec4 = hgrMaterials[i].vector4Parameters[0].value; // Specular Colour
                mat.ColorSpecular = new Color4D(vec4[0], vec4[1], vec4[2], vec4[3]);

                mat.Shininess = hgrMaterials[i].floatParameters[0].value;

                materials.Add(mat);
            }

            return materials;
        }
        public AssimpC.Node CreateRootNode()
        {
            AssimpC.Node root = new AssimpC.Node();

            root.Name = "RootNode";
            root.Parent = null;
            root.Transform = new Matrix4x4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1); // Identity Matrix
            root.Children = new AssimpC.NodeCollection(root);

            return root;
        }
        public void StoreHGR(HGR_Data hgrData)
        {
            baseHGR = new AssimpC.Scene();
            baseHGR.Materials = StoreMaterial(hgrData.materials);
            baseHGR.RootNode = CreateRootNode();
            baseHGR.Meshes = StoreMesh(hgrData);

            IntPtr unData = AssimpC.Scene.ToUnmanagedScene(baseHGR);
            baseHGR = null;
            Scene y = unmanagedToManaged(unData);
            ExportFBXScene(y);
        }

        public Scene unmanagedToManaged(IntPtr unmanagedData)
        {
            return Scene.FromUnmanagedScene(unmanagedData);
            // FromUnmanagedScene (IntPtr) to ManagedScene
        }

        private void ExportFBXScene(Scene scene)
        {
            AssimpContext assimpExporter = new AssimpContext();
            formatIds = assimpExporter.GetSupportedExportFormats();

            FileIOSystem ioSystem = new FileIOSystem(Environment.GetFolderPath(Environment.SpecialFolder.Desktop));
            assimpExporter.SetIOSystem(ioSystem);

            var SelectedFormat = formatIds[3];

            String outputPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), (Path.GetFileName(inputPath)+"."+SelectedFormat.FileExtension));
            
            assimpExporter.ExportFile(scene, outputPath, SelectedFormat.FormatId);
        }
    }
}
