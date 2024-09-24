using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace KA3D_Tools
{
    /// <summary>
    /// HGR -> Hierarichal Graphics Library
    /// </summary>

    public class HGR : ViewModelBase
    {
        private string _data;
        public string Data {
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

        private string _path;
        public string InputPath {
            get => _path;
            set {
                if (_path != value) {
                    _path = value;
                    PathValid = validatePath(_path);
                    OnPropertyChanged(nameof(InputPath));
                }
            }
        }

        private string _texturePath;
        public string TexturePath
        {
            get => _texturePath;
            set
            {
                if (_texturePath != value)
                {
                    _texturePath = value;
                    PathValid = validatePath(_texturePath);
                    OnPropertyChanged(nameof(TexturePath));
                }
            }
        }

        private string _outputPath;
        public string OutputPath {
            get => _outputPath;
            set {
                if (_outputPath != value) {
                    _outputPath = value;
                    PathValid = validatePath(_outputPath);
                    OnPropertyChanged(nameof(OutputPath));
                }
            }
        }

        private bool _pathValid;
        public bool PathValid
        {
            get => _pathValid;
            set
            {
                if (_pathValid != value)
                {
                    _pathValid = value;
                    OnPropertyChanged(nameof(PathValid));
                }
            }
        }

        private bool validatePath(string path) {
            bool chk = true;

            // use 'regexr.com' to create REGEX functions
            // also: https://fireship.io/lessons/regex-cheat-sheet-js/
            // var pathRegex = new Regex(@"^[A-Za-z_][A-Za-z0-9_]*$"); // Valid Name Characters

            if (string.IsNullOrEmpty(path)) chk = false;
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1) chk = false;
            else if (!Directory.Exists(path)) chk = false;

            return chk;
        }

    }
}
