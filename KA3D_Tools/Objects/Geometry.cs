using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Media3D;

namespace KA3D_Tools {

    enum ElementsType {
        Position = 0x00,
        Normals = 0x01,
        TSpace = 0x03,
        Joints = 0x04,
        Colours = 0x08,
    }
    enum PrimitiveTopology {
        Point_List = 1,
        Line_List,
        Line_Strip,
        Triangle_List,
        Triangle_Strip,
    };
    /*
    class MeshRendererVertexData : ViewModelBase {
        private bool _isHighlighted;
        public bool IsHighlighted {
            get => _isHighlighted;
            set { if (_isHighlighted != value) {
                    _isHighlighted = value;
                    OnPropertyChanged(nameof(IsHighlighted));
                    OnPropertyChanged(nameof(Diffuse));
                }
            }
        }

        private bool _isIsolated;
        public bool IsIsolated {
            get => _isIsolated;
            set { if (_isIsolated != value) {
                    _isIsolated = value;
                    OnPropertyChanged(nameof(IsIsolated));
                }
            }
        }

        // Materials in WPF uses Brushes

        //Normally we don't want to put a brush in a view model as it is a dependency object; It belongs to the views.
        //It's a good habit to not mix controls of views with view model, but this is an exception;
        private Brush _specular = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#ff111111")); // Dark Grey - Not Shiny
        public Brush Specular {
            get => _specular;
            set { if (_specular != value) { _specular = value; OnPropertyChanged(nameof(Specular)); } }
        }

        private Brush _diffuse = Brushes.White; // Default Diffuse Color = White
        public Brush Diffuse {
            get => _isHighlighted ? Brushes.Orange : _diffuse;
            set { if (_diffuse != value) { _diffuse = value; OnPropertyChanged(nameof(Diffuse)); } }
        }

        public string Name { get; set; }

        public Point3DCollection Positions { get; } = new Point3DCollection();
        public Vector3DCollection Normals { get; } = new Vector3DCollection();
        public PointCollection UVs { get; } = new PointCollection();
        public Int32Collection Indices { get; } = new Int32Collection();
    }

    class MeshRenderer : ViewModelBase {
        // Camera Properties
        private Vector3D _cameraDirection = new Vector3D(0, 0, -10);
        public Vector3D CameraDirection {
            get => _cameraDirection;
            set { if (_cameraDirection != value) { _cameraDirection = value; OnPropertyChanged(nameof(CameraDirection)); } }
        }

        private Point3D _cameraPosition = new Point3D(0, 0, 10); // Looking Into the scene from +z towards -z
        public Point3D CameraPosition {
            get => _cameraPosition;
            set { if (_cameraPosition != value) { _cameraPosition = value; OnPropertyChanged(nameof(CameraPosition)); OnPropertyChanged(nameof(OffsetCameraPosition)); CameraDirection = new Vector3D(-value.X, -value.Y, -value.Z); } }
        }

        private Point3D _cameraTarget = new Point3D(0, 0, 0); // Initial Target = Origin
        public Point3D CameraTarget {
            get => _cameraTarget;
            set { if (_cameraTarget != value) { _cameraTarget = value; OnPropertyChanged(nameof(CameraTarget)); OnPropertyChanged(nameof(OffsetCameraPosition)); } }
        }

        // Offset Camera wrt Camera's Target
        public Point3D OffsetCameraPosition => new Point3D(CameraPosition.X + CameraTarget.X, CameraPosition.Y + CameraTarget.Y, CameraPosition.Z + CameraTarget.Z);

        // Light Properties for Global Illumination
        private Color _keyLight = (Color)ColorConverter.ConvertFromString("#ffaeaeae"); // Grey-ish color for main light
        public Color KeyLight {
            get => _keyLight;
            set { if (_keyLight != value) { _keyLight = value; OnPropertyChanged(nameof(KeyLight)); } }
        }

        private Color _skyLight = (Color)ColorConverter.ConvertFromString("#ff111b30"); // Blue-ish color for sky light
        public Color SkyLight {
            get => _skyLight;
            set { if (_skyLight != value) { _skyLight = value; OnPropertyChanged(nameof(SkyLight)); } }
        }

        private Color _groundLight = (Color)ColorConverter.ConvertFromString("#ff3f2f1e"); // warmer red-ish color for ground light
        public Color GroundLight {
            get => _groundLight;
            set { if (_groundLight != value) { _groundLight = value; OnPropertyChanged(nameof(GroundLight)); } }
        }

        private Color _ambientLight = (Color)ColorConverter.ConvertFromString("#ff3b3b3b"); // Neutral Grey color for ambient light
        public Color AmbientLight {
            get => _ambientLight;
            set { if (_ambientLight != value) { _ambientLight = value; OnPropertyChanged(nameof(AmbientLight)); } }
        }

        // Constructor that accepts mesh LOD and unpacks Data
        public MeshRenderer(Mesh mesh, MeshRenderer old) {
            Debug.Assert(mesh.VertexCount >= 0);

            // In order to setup the camera pos and target properly, we need to figure out how big the object
            // which we are rendering is. Hence, we need to know it's bounding box.
            // Calculate Bounding Box = Max position and Min Postion
            double minX, minY, minZ; minX = minY = minZ = double.MaxValue;
            double maxX, maxY, maxZ; maxX = maxY = maxZ = double.MinValue;
            Vector3D avgNormal = new Vector3D(); // to look at the average dir. of all normals

            // Unpack Packed Normals => (i/intervals)*2 - 1
            var interval = 2.0f / ((1 << 16) - 1);

            var vertexData = new MeshRendererVertexData() { Name = mesh.Name };
            // unpack all vertices
            using (var reader = new BinaryReader(new MemoryStream(mesh.Positions)))
                for (int i = 0; i < mesh.VertexCount; ++i)
                {
                    // Read Positions
                    var posX = reader.ReadSingle();
                    var posY = reader.ReadSingle();
                    var posZ = reader.ReadSingle();

                    vertexData.Positions.Add(new Point3D(posX, posY, posZ));

                    //Adjust Bounding Box
                    minX = Math.Min(minX, posX); maxX = Math.Max(maxX, posX);
                    minY = Math.Min(minY, posY); maxY = Math.Max(maxY, posY);
                    minZ = Math.Min(minZ, posZ); maxZ = Math.Max(maxZ, posZ);
                }
            if (mesh.ElementsType.HasFlag(ElementsType.Normals))
            {
                var tSpaceOffset = 0;
                if (mesh.ElementsType.HasFlag(ElementsType.Joints)) tSpaceOffset = sizeof(short) * 4; // skip joint indices

                using (var reader = new BinaryReader(new MemoryStream(mesh.Elements)))
                    for (int i = 0; i < mesh.VertexCount; ++i)
                    {
                        // we need to get the normal Z from the int32 reserved and t_sign for displaying Plane
                        //last byte of integer = t_sign
                        //& it so that everything except this last byte is zeroed [optional]
                        var signs = (reader.ReadUInt32() >> 24) & 0x000000ff;
                        reader.BaseStream.Position += tSpaceOffset;

                        // Read Normals X and Y
                        var nrmX = reader.ReadUInt16() * interval - 1.0f;
                        var nrmY = reader.ReadUInt16() * interval - 1.0f;
                        //Calculate Normal Z
                        var nrmZ = Math.Sqrt(Math.Clamp(1f - (nrmX * nrmX + nrmY * nrmY), 0f, 1f)) * ((signs & 0x2) - 1f); //second bit of signs is Normal Z sign

                        var normal = new Vector3D(nrmX, nrmY, nrmZ);
                        normal.Normalize();
                        vertexData.Normals.Add(normal);
                        avgNormal += normal;

                        //Read UV
                        if (mesh.ElementsType.HasFlag(ElementsType.TSpace))
                        {
                            reader.BaseStream.Position += sizeof(short) * 2; //Advance Read Head to skip Tangents
                            var u = reader.ReadSingle();
                            var v = reader.ReadSingle();
                            vertexData.UVs.Add(new Point(u, v));
                        }

                        if (mesh.ElementsType.HasFlag(ElementsType.Joints) && mesh.ElementsType.HasFlag(ElementsType.Colours)) // WHY am i checking joints/skeletal again?
                        {
                            reader.BaseStream.Position += 4; // skip colours
                        }
                    }
            }
            using (var reader = new BinaryReader(new MemoryStream(mesh.Indices)))
                if (mesh.IndexSize == sizeof(short))
                    for (int i = 0; i < mesh.IndexCount; ++i) vertexData.Indices.Add(reader.ReadUInt16()); // read 2 bytes
                else
                    for (int i = 0; i < mesh.IndexCount; ++i) vertexData.Indices.Add(reader.ReadInt32()); // read 4 bytes
            // Important Note: If there are a lot of vertices, index value would become larger than it would fit in 16 bit integer
            //                 Therefore, we need to use signed 32 bit integers, otherwise the Int32 collection would interpret it
            //                 as negative values.

            // Freeze function makes the collection unmodifiable -> framework will perform optimizations to make it faster
            vertexData.Positions.Freeze();
            vertexData.Normals.Freeze();
            vertexData.UVs.Freeze();
            vertexData.Indices.Freeze();

            Meshes.Add(vertexData);

            // Set camera position and direction depending on how big the object was and the average of the normals
            if (old != null)
            {
                CameraTarget = old.CameraTarget;
                CameraPosition = old.CameraPosition;
                CameraDirection = old.CameraDirection;

                // NOTE: this is only for primitive meshes with multiple LODs,
                //       because they're displayed with textures
                foreach (var mesh in old.Meshes)
                {
                    mesh.IsHighlighted = false;
                }
                foreach (var mesh in Meshes)
                {
                    mesh.Diffuse = old.Meshes.First().Diffuse;
                }
            } else {
                // Compute bounding box dimensions
                var width = maxX - minX;
                var height = maxY - minY;
                var depth = maxZ - minZ;
                var radius = new Vector3D(height, width, depth).Length * 1.2; //radius of the sphere enclosing the bounding box = diameter bigger than the bounding box [2x or 1.2x]
                if (avgNormal.Length > 0.8)
                {
                    avgNormal.Normalize();
                    avgNormal *= radius;
                    CameraPosition = new Point3D(avgNormal.X, avgNormal.Y, avgNormal.Z);
                }
                else
                {
                    CameraPosition = new Point3D(width, height * 0.5, radius);
                }
                CameraTarget = new Point3D(minX + width * 0.5, minY + height * 0.5, minZ + depth * 0.5);
            }
        }
    }

    class Mesh : ViewModelBase {
        public static int PositionSize = sizeof(float) * 3;
        private int _elementSize;
        public int ElementSize　{
            get => _elementSize;
            set{　if (_elementSize != value)　{
                    _elementSize = value;
                    OnPropertyChanged(nameof(ElementSize));
                }
            }
        }
        private int _vertexCount;
        public int VertexCount {
            get => _vertexCount;
            set { if (_vertexCount != value) { _vertexCount = value; OnPropertyChanged(nameof(VertexCount)); } }
        }

        private int _indexSize;
        public int IndexSize {
            get => _indexSize;
            set { if (_indexSize != value) { _indexSize = value; OnPropertyChanged(nameof(IndexSize)); } }
        }

        private int _indexCount;
        public int IndexCount {
            get => _indexCount;
            set { if (_indexCount != value) { _indexCount = value; OnPropertyChanged(nameof(IndexCount)); } }
        }

        private string _name;
        public string Name
        {
            get => _name;
            set { if (_name != value) { _name = value; OnPropertyChanged(nameof(Name)); } }
        }

        public ElementsType ElementsType { get; set; }
        public PrimitiveTopology PrimitiveTopology { get; set; }
        // Byte Arrays
        public byte[] Positions { get; set; }
        public byte[] Elements { get; set; }
        public byte[] Indices { get; set; }
    }
    */

    class Geometry : ViewModelBase {

    }
}
