using System.Runtime.InteropServices;

namespace KA3D_Tools
{
    static class ContentToolAPI
    {
        private const string _contentTool = "ContentTool.dll";

        [DllImport(_contentTool, CharSet = CharSet.Ansi)]
        public static extern bool StoreData(string path, string texpath, string outpath);
        public static bool StoreHGR(string inputPath, string texturePath, string outputPath) {
            return StoreData(inputPath, texturePath, outputPath);
        }
    }
}
