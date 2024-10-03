using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
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
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using Microsoft.WindowsAPICodePack.Dialogs;

namespace KA3D_Tools
{
    /// <summary>
    /// Interaction logic for NTXEditor.xaml
    /// </summary>
    public partial class NTXEditor : UserControl
    {
        public ImageFileType outputType;
        public NTXEditor()
        {
            InitializeComponent();
        }

        private void OnFiletypeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var vm = sender as ComboBox;
            outputType = (ImageFileType)vm.SelectedIndex;
        }

        private void OnConvertBulkButton_Clicked(object sender, RoutedEventArgs e)
        {
            var vm = DataContext as NTX;
            var dlg = new CommonOpenFileDialog()
            {
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                IsFolderPicker = true
            };
            if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
            {
                string[] files = Directory.GetFiles(dlg.FileName, "*.ntx", SearchOption.TopDirectoryOnly);
                    vm.OutPath = $@"{dlg.FileName}/NTX Output";
                    Directory.CreateDirectory(vm.OutPath);

                    foreach (var ntx in files)
                    {
                        vm.NTXPath = ntx;
                        vm.outType = outputType;
                        vm.readNTX();
                    }
            }
        }

        private void OnConvertSingleFileButton_Clicked(object sender, RoutedEventArgs e)
        {
            var dlg = new OpenFileDialog()
            {
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                Filter = "KA3D Image File (*.ntx)|*.ntx"
            };

            if (dlg.ShowDialog() == true)
            {
                // Code runs after selecting the file
                Debug.Assert(!string.IsNullOrEmpty(dlg.FileName));
                // read the file
                var vm = DataContext as NTX;
                vm.NTXPath = dlg.FileName;
                vm.outType = outputType;
                vm.OutPath = System.IO.Path.GetDirectoryName(dlg.FileName);
                vm.readNTX();
            }
        }
    }
}
