using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace KA3D_Tools {
    /// <summary>
    /// Interaction logic for GeometryView.xaml
    /// </summary>
    public partial class GeometryView : UserControl {
        // Note: WPF can't display wireframe

        private Point _clickPosition;
        private bool _capturedLeft;
        private bool _capturedRight;
        /*
        public void SetGeometry(int index = -1) { //index => to display a selected mesh out of a group of 'em, otherwise -1 shows all meshes
            if (!(DataContext is MeshRenderer vm)) return;

            //if (viewport.Children.Count == 2) { // There's a group of objects besides lighting
            //    viewport.Children.RemoveAt(1); // Remove it
            //}

            var meshIndex = 0;
            var modelGroup = new Model3DGroup();
            foreach (var mesh in vm.Meshes) {
                // Skip meshes we don't want to display
                if (index != -1 && meshIndex != index) {
                    ++meshIndex;
                    continue;
                }

                var mesh3D = new MeshGeometry3D()
                {
                    Positions = mesh.Positions,
                    Normals = mesh.Normals,
                    TriangleIndices = mesh.Indices,
                    TextureCoordinates = mesh.UVs
                };

                var Diffuse = new DiffuseMaterial(mesh.Diffuse);
                var Specular = new SpecularMaterial(mesh.Specular, 50); // Higher Power = Smaller Specular Highlight
                var matGroup = new MaterialGroup();
                matGroup.Children.Add(Diffuse);
                matGroup.Children.Add(Specular);

                var model = new GeometryModel3D(mesh3D, matGroup);
                modelGroup.Children.Add(model);

                var binding = new Binding(nameof(mesh.Diffuse)) { Source = mesh };
                BindingOperations.SetBinding(Diffuse, DiffuseMaterial.BrushProperty, binding);

                if (meshIndex == index) break; // If selected mesh was added; index != -1
            }
            var visual = new ModelVisual3D() { Content = modelGroup };
            viewport.Children.Add(visual);
        }

        // LMB = Camera Rotate About Mesh
        // RMB = Camera Move Up/Down
        // Mouse Wheel = Camera Zoom

        private void OnGrid_MouseLBD(object sender, MouseButtonEventArgs e) {
            _clickPosition = e.GetPosition(this);
            _capturedLeft = true;
            Mouse.Capture(sender as UIElement);
        }

        private void OnGrid_MouseLBU(object sender, MouseButtonEventArgs e) {
            _capturedLeft = false;
            if (!_capturedRight) Mouse.Capture(null); // if right button is still down, then don't release mouse capture
        }

        private void OnGrid_MouseRBD(object sender, MouseButtonEventArgs e) {
            _clickPosition = e.GetPosition(this);
            _capturedRight = true;
            Mouse.Capture(sender as UIElement);
        }

        private void OnGrid_MouseRBU(object sender, MouseButtonEventArgs e) {
            _capturedRight = false;
            if (!_capturedLeft) Mouse.Capture(null); // if right button is still down, then don't release mouse capture
        }

        private void OnGrid_MouseMove(object sender, MouseEventArgs e) {
            if (!_capturedLeft && !_capturedRight) return;

            // Distance Moved by the Mouse
            var pos = e.GetPosition(this);
            var d = pos - _clickPosition;

            if (_capturedLeft && !_capturedRight) {
                MoveCamera(d.X, d.Y, 0);
            } else if (_capturedRight && !_capturedLeft) {
                var vm = DataContext as MeshRenderer;
                var cameraPos = vm.CameraPosition;
                var yOffset = d.Y * 0.001 * Math.Sqrt(cameraPos.X * cameraPos.X + cameraPos.Z * cameraPos.Z);
                // d.Y * 0.001 = 1mm/pixel
                // Distance from object determines the scaling of the movement 
                vm.CameraTarget = new Point3D(vm.CameraTarget.X, vm.CameraTarget.Y + yOffset, vm.CameraTarget.Z);
            }

            _clickPosition = pos; // update in loop
        }

        private void OnGrid_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            MoveCamera(0, 0, Math.Sign(e.Delta)); // e.Delta = value of mouse wheel | Math.Sign = returns only the sign of input : +1 or -1
        }

        private void MoveCamera(double dx, double dy, int dz)
        {
            var vm = DataContext as MeshRenderer;
            var v = new Vector3D(vm.CameraPosition.X, vm.CameraPosition.Y, vm.CameraPosition.Z);

            // Spherical - from Cartesian
            var r = v.Length;
            var theta = Math.Acos(v.Y / r);
            var phi = Math.Atan2(-v.Z, v.X); // Learn this later

            // Update Camera 
            theta -= dy * 0.01;
            phi -= dx * 0.01;
            r *= 1.0 - 0.1 * dz; // dx is either +1 or -1

            theta = Math.Clamp(theta, 0.0001, Math.PI - 0.0001); // Camera's UP vector is undefined at exact values of 0 and Math.PI (Max Value)

            // Cartesian - from Spherical
            v.X = r * Math.Sin(theta) * Math.Cos(phi); // Is this wrong?
            v.Z = -r * Math.Sin(phi) * Math.Sin(theta);
            v.Y = r * Math.Cos(theta);

            vm.CameraPosition = new Point3D(v.X, v.Y, v.Z);
        }
        */
        public GeometryView() {
            InitializeComponent();
        }
    }
}
