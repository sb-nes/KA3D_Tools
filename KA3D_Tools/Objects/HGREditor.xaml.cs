using System;
using System.Collections.Generic;
using System.Diagnostics;
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
                ContentToolAPI.StoreHGR(dlg.FileName);
            }
        }
    }
}
