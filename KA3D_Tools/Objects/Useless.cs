using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    public class Useless
    {
        public string[] PlatformName =
        {
            "dx",
            "egl",
            "psp",
            "sw",
            "n3d"
        };

        public string[] PlatformDesc =
        {
            "DirectX 9.x",
            "OpenGL ES",
            "PlayStation Portable",
            "Software Renderer",
            "Nokia N3D"
        };

        public enum DataFlags
        {
            DATA_MATERIALS                  = 1,
            DATA_PRIMITIVES                 = 2,
            DATA_NODES                      = 4,
            DATA_ANIMATIONS                 = 8,
            DATA_USERPROPERTIES             = 16,

        }
    }
}
