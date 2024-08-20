using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    class Exporter
    {
        public string inputPath;
        private const string _contentTool = "ContentTool.dll";

        [DllImport(_contentTool, CharSet = CharSet.Ansi)]
        public static extern bool ReadData();

        public void StoreHGR() {
            ReadData();
        }
    }
}
