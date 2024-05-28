using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    [StructLayout(LayoutKind.Sequential)]
    class NTX_Header
    {
        /*
        UInt16 
        version
        width
        height
        format
        palettesize
        flags
        userflags
        */
        public int                  version;
        public int                  width;
        public int                  height;
        public int                  palettesize;
    }

    public class NTX : ViewModelBase
    {
        private string _ntxPath;
        public string NTXPath
        {
            get => _ntxPath;
            set { if(_ntxPath != value)
                {
                    _ntxPath = value;
                    OnPropertyChanged(nameof(NTXPath));
                } }
        }

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

        private void readHeader(BinaryReader bw, NTX_Header head)
        {
            head.version = bw.ReadUInt16(); // Version
            head.height = bw.ReadUInt16(); // Height
            head.width = bw.ReadUInt16(); // Width
            bw.ReadUInt16(); // Format
            head.palettesize = bw.ReadUInt16(); // Palette Size
            bw.ReadUInt16(); // Flags = 0
            bw.ReadUInt16(); // User Flags = 0
        }

        public void setBytesLE(int[] x, int pal, int size)
        {

        }
        public void getBytesLE(int[] x, int size) // this should return a uint32_t
        {

        }

        private void readPalette(BinaryReader bw, NTX_Header head)
        {
            int bytesperpixel = 0;
            int[] pal = new int[head.palettesize];
            if (head.palettesize > 0)
            {
                for (int i = 0; i < head.palettesize; ++i)
                {
                    int[] bytes = new int[4];
                    bw.ReadBytes(bytesperpixel);
                    setBytesLE(bytes, pal[i], bytesperpixel); // am i sending an intptr as bytes???
                }
            }
        }

        private void readPixelData(BinaryReader bw, NTX_Header head)
        {
            int bytesperpixel = 0;
            for(int j = 0; j < head.height; ++j)
            {
                for(int i = 0; i<head.width; ++i)
                {
                    if (head.palettesize > 0)
                    {

                    } else { // if no palette exists
                        bw.ReadBytes(bytesperpixel);
                    }
                }
            }
        }

        public void readNTX()
        {
            // format : SurfaceFormat -> namespace gr
            // pitch : int -> width?
            NTX_Header header = new NTX_Header();
            var file = File.Open(_ntxPath, FileMode.Open, FileAccess.Read);
            using (var bw = new BinaryReader(file))
            {
                readHeader(bw, header);
                // This needs to be in an async func.
                // Data has 2 parts: Palette + Pixel Data
                readPalette(bw, header);
                var data = bw.ReadBytes((int)(file.Length - file.Position)); // Byte Array -> uint8_t
                foreach(var d in data)
                {
                    Data += d;
                }
            }
            
        }
        public NTX()
        {
        }
    }
}
