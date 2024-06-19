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

    [StructLayout(LayoutKind.Sequential)]
    class HGR_Header
    {
        public int              m_ver;
        public int              m_exportedVer;
        public int              m_dataFlags;
        public int              m_platformID;
    }

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
        public void readSceneParameters(BinaryReader bw)
        {
            Data += bw.ReadByte() == 1 ? "fogtype = linear | " : "fogtype = none | ";

            Data += "Fog Start = " + Convert.ToString((bw.ReadByte() << 24) + (bw.ReadByte() << 16) + 
                                                      (bw.ReadByte() << 8) + bw.ReadByte(), 16);

            Data += " | Fog End = " + Convert.ToString((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                      (bw.ReadByte() << 8) + bw.ReadByte(), 16) + " | ";

            for (int i = 0; i < 3; ++i)
            {
                //fogColor[i] = (bw.ReadByte() << 8) + (bw.ReadByte() << 8) + (bw.ReadByte() << 8) + bw.ReadByte();
                Data += " " + (i+1) + ". " + Convert.ToString((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                                              (bw.ReadByte() << 8) + bw.ReadByte(), 16);
            }
            Data += "\n";
        }

        private void readTextureData(BinaryReader bw)
        {
            UInt32 texCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) + 
                                               (bw.ReadByte() << 8) + bw.ReadByte());
            Data += "Texture Count = " + texCount  + "\n";

            for (UInt32 i = 0; i < texCount; i++)
            {
                int size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach(char ch in bw.ReadChars(size)) {
                    Data += ch;
                }
                int type = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();
                Data += size > 15 ? "" : "\t";
                Data += " \t\t\t" + (type == 1 ? "cubemap" : "texture") + "\n";
            }
        }

        private void readTexParameter(BinaryReader bw)
        {
            // Parameter Type -> String
            int size = (bw.ReadByte() << 8) + bw.ReadByte();
            foreach (char ch in bw.ReadChars(size))
            {
                Data += ch;
            }
            UInt16 texIndex = Convert.ToUInt16((bw.ReadByte() << 8) + bw.ReadByte());
        }
        private void readVec4Parameter(BinaryReader bw)
        {
            int size = (bw.ReadByte() << 8) + bw.ReadByte();
            foreach (char ch in bw.ReadChars(size))
            {
                Data += ch;
            }
            float[] value = new float[4];
            for (int i = 0; i < 4; i++)
            {
                value[i] = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();
            }
        }
        private void readFloatParameter(BinaryReader bw)
        {
            int size = (bw.ReadByte() << 8) + bw.ReadByte();
            foreach (char ch in bw.ReadChars(size))
            {
                Data += ch;
            }
            float value = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();
        }

        private void readMaterialData(BinaryReader bw)
        {
            UInt32 matCount = Convert.ToUInt32((bw.ReadByte() << 24) + (bw.ReadByte() << 16) +
                                               (bw.ReadByte() << 8) + bw.ReadByte());
            Data += "Material Count = " + matCount + "\n";

            for (UInt32 i = 0; i < matCount; i++)
            {
                int size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach (char ch in bw.ReadChars(size)) {
                    Data += ch;
                }
                size = (bw.ReadByte() << 8) + bw.ReadByte();
                foreach (char ch in bw.ReadChars(size)) {
                    Data += ch;
                }

                // lightmap should be used by Shader -> bit 0 set
                int lightmap = (bw.ReadByte() << 24) + (bw.ReadByte() << 16) + (bw.ReadByte() << 8) + bw.ReadByte();

                UInt16 textureparamcount = bw.ReadByte();
                for (UInt16 j = 0; j < textureparamcount; ++j) {
                    readTexParameter(bw);
                }
                UInt16 vec4paramcount = bw.ReadByte();
                for (UInt16 j = 0; j < vec4paramcount; ++j)
                {
                    readVec4Parameter(bw);
                }
                UInt16 floatparamcount = bw.ReadByte();
                for (UInt16 j = 0; j < floatparamcount; ++j)
                {
                    readFloatParameter(bw);
                }
            }
        }

        public void readHGR()
        {
            var file = File.Open(filePath, FileMode.Open, FileAccess.Read);
            HGR_Header header = new HGR_Header();
            using (var bw = new BinaryReader(file))
            {
                Debug.Assert(string.Concat(bw.ReadChars(4)) == "hgrf");
                readHeader(bw, header);
                readSceneParameters(bw);

                setCheckID(bw);

                readTextureData(bw);
                readMaterialData(bw);
                checkUpdatedID(bw);

                //primitives
                //checkUpdatedID(bw);

                //meshes
                //checkUpdatedID(bw);

                //cameras
                //checkUpdatedID(bw);

                //lights
                //checkUpdatedID(bw);

                //dummies
                //checkUpdatedID(bw);

                //shapes
                //checkUpdatedID(bw);

                //other nodes
                //checkUpdatedID(bw);

                //transform animations
                //checkUpdatedID(bw);

                //user properties
            }
        }
    }
}
