using System;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using Microsoft.Win32;
using Microsoft.WindowsAPICodePack.Dialogs; // use NuGet for CommonOpenFileDialog

namespace KA3D_Tools
{
    /// <summary>
    /// Interaction logic for HGREditor.xaml
    /// </summary>
    public partial class HGREditor : UserControl
    {
        public HGREditor()
        {
            InitializeComponent();
            Loaded += OnHGREditor_Loaded;
        }

        private void OnHGREditor_Loaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnHGREditor_Loaded;
            var vm = DataContext as HGR;
            vm.InputPath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            vm.TexturePath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            vm.OutputPath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            vm.PathValid = true;
        }

        private void On_ReadFileButton_Clicked(object sender, RoutedEventArgs e)
        {
            var dlg = new OpenFileDialog()
            {
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                Filter = "KA3D Object/Scene File (*.hgr)|*.hgr"
            };

            if (dlg.ShowDialog() == true)
            {
                // Code runs after selecting the file
                Debug.Assert(!string.IsNullOrEmpty(dlg.FileName));
                // read the file
                var vm = DataContext as HGR;
                if (ContentToolAPI.StoreHGR(dlg.FileName, vm.TexturePath, vm.OutputPath)) {
                    vm.Data += dlg.FileName + "\n";
                }

                MessageBox.Show("Conversion Completed : " + dlg.FileName.Substring(dlg.FileName.LastIndexOf("\\") + 1, dlg.FileName.Length - dlg.FileName.LastIndexOf("\\") - 1));
            }
        }

        private void On_ReadMultipleButton_Clicked(object sender, RoutedEventArgs e)
        {
            var vm = DataContext as HGR;
            string[] files = Directory.GetFiles(vm.InputPath, "*.hgr", SearchOption.TopDirectoryOnly);
            Directory.CreateDirectory(vm.OutputPath);

            foreach (var hgr in files) {
                if (ContentToolAPI.StoreHGR(hgr, vm.TexturePath, vm.OutputPath)) {
                    vm.Data += hgr + "\n";
                }
            }
        }

        private void On_SelectPathButton_Clicked(object sender, RoutedEventArgs e)
        {
            var vm = DataContext as HGR;
            var dlg = new CommonOpenFileDialog() {
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                IsFolderPicker = true
            };
            if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
            {
                vm.InputPath = dlg.FileName;
            }
        }

        private void On_SelectTexturePathButton_Clicked(object sender, RoutedEventArgs e) {
            var vm = DataContext as HGR;
            var dlg = new CommonOpenFileDialog()
            {
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                IsFolderPicker = true
            };
            if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
            {
                vm.TexturePath = dlg.FileName;
            }
        }

        private void On_SelectOutputPathButton_Clicked(object sender, RoutedEventArgs e) {
            var vm = DataContext as HGR;
            var dlg = new CommonOpenFileDialog()
            {
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                IsFolderPicker = true
            };
            if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
            {
                vm.OutputPath = dlg.FileName;
            }
        }
    }
}
