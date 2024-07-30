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
        private const string _hgrDll = "HGR_RW.dll";

        [DllImport(_hgrDll, CharSet = CharSet.Ansi)]
        public static extern void ReadHGR();

        public void StoreHGR() {
            ReadHGR();
        }
    }
}
