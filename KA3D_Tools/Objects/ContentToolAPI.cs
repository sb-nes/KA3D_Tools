using System.Runtime.InteropServices;

namespace KA3D_Tools
{
    static class ContentToolAPI
    {
        private const string _contentTool = "ContentTool.dll";

        [DllImport(_contentTool, CharSet = CharSet.Ansi)]
        public static extern bool StoreData(string path);

        public static void StoreHGR(string inputPath) {
            StoreData(inputPath);
        }
    }
}
