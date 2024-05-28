using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
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
        public int                  format;
        public int                  palettesize;
    }


    public class NTX : ViewModelBase
    {
        [DllImport("gdi32.dll")]
        public static extern bool DeleteObject(IntPtr hObject);


        private string _ntxPath;
        public string NTXPath
        {
            get => _ntxPath;
            set { if (_ntxPath != value)
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

        public int bitsperpixel = 0;
        public int[] pal; // Table of Common Colours in the image -> Lossless Compression Technique
        public int[] img;

        private Bitmap _img;
        public Bitmap IMG
        {
            get => _img;
            set
            {
                if (_img != value)
                {
                    _img = value;
                    OnPropertyChanged(nameof(IMG));
                }
            }
        }

        // Properties -> Build -> Allow Unsafe Code : For using Pointers

        // uint32_t setBytesLE(uint8_t* dst, uint32_t c, int bytesperpixel)
        public byte[] setBytesLE(byte c, int bytesperpixel)
        {
            byte[] dst = new byte[4]; 
            dst[0] = (byte) c;

            switch (bytesperpixel)
            {
                case 4:
                    dst[3] = (byte)(c >> 24);
                    dst[2] = (byte)(c >> 16);
                    dst[1] = (byte)(c >> 8);
                    break;
                case 3:
                    dst[2] = (byte)(c >> 16);
                    dst[1] = (byte)(c >> 8);
                    break;
                case 2:
                    dst[1] = (byte)(c >> 8);
                    break;
            }

            return dst;
        }

        // uint32_t getBytesLE(uint8_t* src, int bytesperpixel)
        public int getBytesLE(byte[] src, int bytesperpixel) // this should return a uint32_t
        {
            int d = src[0];
            switch (bytesperpixel)
            {
                case 4:
                    d += src[3] << 24;
                    d += src[2] << 16;
                    d += src[1] << 8;
                    break;
                case 3:
                    d += src[2] << 16; 
                    d += src[1] << 8;
                    break;
                case 2:
                    d += src[1] << 8;
                    break;
            }
            return d; //returns an integer of all bytes
        }


        private void createBMP(NTX_Header head)
        {
            Bitmap bmp = new Bitmap(head.width, head.height);
            for (int y = 0; y < head.height; y++)
            {
                for (int x = 0; x < head.width; x++)
                {
                    int i = (y * head.width) + x;
                    int pixelData = img[i];

                    switch (head.format)
                    {
                        case (int)SurfaceFormat.SURFACE_A4R4G4B4:
                            int a = ((pixelData & 0xF000) >> 8);
                            int r = ((pixelData & 0x0F00) >> 4);
                            int g = ((pixelData & 0x00F0));
                            int b = ((pixelData & 0x000F) << 4);
                            Color color = Color.FromArgb(a, r, g, b);
                            bmp.SetPixel(x, y, color);
                            break;
                    }
                }
            }
            bmp.Save($@"{Environment.GetFolderPath(Environment.SpecialFolder.Desktop)}/home.png", ImageFormat.Png);
            IMG = bmp;
        }

        private void readHeader(BinaryReader bw, NTX_Header head) // Read 14 bytes
        {
            head.version = bw.ReadUInt16(); // Version
            head.height = bw.ReadUInt16(); // Height
            head.width = bw.ReadUInt16(); // Width
            head.format = bw.ReadUInt16(); // Format
            head.palettesize = bw.ReadUInt16(); // Palette Size
            // I don't think we need the flags rn
            bw.ReadUInt16(); // Flags = 0
            bw.ReadUInt16(); // User Flags = 0
        }

        private void readPalette(BinaryReader bw, NTX_Header head) // What is this palette used for? -> Compression
        {
            int bytesperpixel = bitsperpixel/8;
            int pixels = head.width * head.height;
            Debug.Assert(head.palettesize <= pixels);
            pal = new int[head.palettesize];
            if (head.palettesize > 0)
            {
                for (int i = 0; i < head.palettesize; ++i)
                {
                    byte[] bytes;
                    bytes = bw.ReadBytes(bytesperpixel);
                    pal[i] = getBytesLE(bytes, bytesperpixel);
                }
            }
        }

        private void readPixelData(BinaryReader bw, NTX_Header head)
        {
            int bytesperpixel = bitsperpixel/8;
            img = new int[head.width * head.height * bytesperpixel];
            int index;
            for(int j = 0; j < head.height; ++j)
            {
                for(int i = 0; i<head.width; ++i)
                {
                    if (head.palettesize > 0)
                    {
                        index = bw.ReadByte();
                        Debug.Assert(index != -1);
                        Debug.Assert(index >= 0 && index < 256);
                        img[i + (j * head.width)] = pal[index];
                    } else { // if no palette exists
                        img[i + (j * head.width)] = getBytesLE(bw.ReadBytes(bytesperpixel), bytesperpixel);
                    }
                }
            }
        }

        public void readNTX()
        {
            // pitch : int -> width?
            NTX_Header header = new NTX_Header();
            var file = File.Open(_ntxPath, FileMode.Open, FileAccess.Read);
            using (var bw = new BinaryReader(file))
            {
                readHeader(bw, header);
                // This needs to be in an async func.
                // Data has 2 parts: Palette + Pixel Data
                {
                    var vm = new KA3D_Image();
                    bitsperpixel = (int)vm.FORMAT_DESC[header.format, 0];
                }
                readPalette(bw, header);
                readPixelData(bw, header);
                Debug.WriteLine(file.Length - file.Position <= 0);

                // var data = bw.ReadBytes((int)(file.Length - file.Position)); // Byte Array -> uint8_t
                var x = (SurfaceFormat)header.format;
                Data += x.ToString();
                Data += "\n";
                createBMP(header);
            }
            
        }
        public NTX()
        {
        }
    }
}
