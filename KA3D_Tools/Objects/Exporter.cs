using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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

        public ExportFormatDescription[] formatIds;

        public List<AssimpC.Mesh> StoreMesh(HGR_Data hgrData)
        {
            List<AssimpC.Mesh> meshes = new List<AssimpC.Mesh>();

            for(int i = 0; i < hgrData.meshes.Length; ++i)
            {
                AssimpC.Mesh mesh = new AssimpC.Mesh();

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
                        var vert = new Vector3D(vertData.x, vertData.y, vertData.z);
                        mesh.Vertices.Add(vert);
                    }

                    mesh.TextureCoordinateChannels = new List<Vector3D>[1]; //Channel 0 is default for UV
                    mesh.TextureCoordinateChannels[0] = new List<Vector3D>();

                    foreach (var vertData in temp[1].vert0) // Tex_Coords
                    {
                        var vert = new Vector3D(vertData.x, vertData.y, 0);
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
        public void StoreHGR(HGR_Data hgrData)
        {
            AssimpC.Scene baseHGR = new AssimpC.Scene();
            baseHGR.Materials = StoreMaterial(hgrData.materials);
            baseHGR.Meshes = StoreMesh(hgrData);

            IntPtr unData = AssimpC.Scene.ToUnmanagedScene(baseHGR);
            ExportFBXScene(unData);
        }

        private void ExportFBXScene(IntPtr unmanagedData)
        {
            AssimpContext assimpExporter = new AssimpContext();
            string[] formatsIn = assimpExporter.GetSupportedImportFormats();
            formatIds = assimpExporter.GetSupportedExportFormats();
            Scene scene = Scene.FromUnmanagedScene(unmanagedData); // FromUnmanagedScene (IntPtr) to ManagedScene

            FileIOSystem ioSystem = new FileIOSystem(Environment.GetFolderPath(Environment.SpecialFolder.Desktop));
            assimpExporter.SetIOSystem(ioSystem);

            // This part doesn't work rn, or maybe at all
            String outputPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), "bounce.dae");
            assimpExporter.ExportFile(scene, outputPath, formatIds[0].FormatId, PostProcessSteps.None);

            String inputPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), "jackFrost.fbx");
            scene = assimpExporter.ImportFile(inputPath, PostProcessSteps.None);
            outputPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), "jack.fbx");

            assimpExporter.ExportFile(scene, outputPath, formatIds[0].FormatId, PostProcessSteps.None);
        }
    }
}
