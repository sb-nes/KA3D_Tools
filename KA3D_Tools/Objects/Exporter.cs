using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Assimp;

namespace KA3D_Tools
{
    public class Exporter
    {
        /// <summary>
        /// Final Conversion to FBX:
        /// HGR interpreted data is converted to IntPtr (Native Pointer) using MemoryHelper Structure.
        /// Assimp then converts it to FBX!
        /// </summary>
        

        public List<AssimpC.Material> StoreMaterial(Material[] hgrMaterials)
        {
            List<AssimpC.Material> materials = new List<AssimpC.Material>();

            for(int i = 0; i < hgrMaterials.Length; ++i)
            {
                AssimpC.Material mat = new AssimpC.Material(); // i'm thinking to keep the rest default
                mat.Name = hgrMaterials[i].name;
                mat.IsTwoSided = false;

                float[] vec4;
                vec4 = hgrMaterials[i].vector4Parameters[0].value; // Ambient Colour
                mat.ColorAmbient = new Color4D(vec4[0], vec4[1], vec4[2], vec4[3]);
                vec4 = hgrMaterials[i].vector4Parameters[0].value; // Diffuse Colour
                mat.ColorDiffuse = new Color4D(vec4[0], vec4[1], vec4[2], vec4[3]);
                vec4 = hgrMaterials[i].vector4Parameters[0].value; // Specular Colour
                mat.ColorSpecular = new Color4D(vec4[0], vec4[1], vec4[2], vec4[3]);

                mat.Shininess = hgrMaterials[i].floatParameters[0].value;
            }

            return materials;
        }
        public void StoreHGR(HGR_Data hgrData)
        {
            StoreMaterial(hgrData.materials);
        }

        private void ExportFBXScene(IntPtr unmanagedData)
        {
            AssimpContext assimpExporter = new AssimpContext();
            Scene scene = Scene.FromUnmanagedScene(unmanagedData); // FromUnmanagedScene (IntPtr) to ManagedScene

            FileIOSystem ioSystem = new FileIOSystem(Environment.GetFolderPath(Environment.SpecialFolder.Desktop));
            assimpExporter.SetIOSystem(ioSystem);


            String outputPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), "Output/bounce.fbx");
            assimpExporter.ExportFile(scene, outputPath, "fbx", PostProcessSteps.None);
        }
    }
}
