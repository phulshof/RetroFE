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
using System.Windows.Navigation;
using System.Windows.Shapes;
using Configuration.ViewModel;

namespace Configuration.View
{
    /// <summary>
    /// Interaction logic for Launcher.xaml
    /// </summary>
    public partial class Launcher : UserControl
    {
        public Launcher()
        {
            InitializeComponent();
        }


        public LauncherVM Data
        {
            get { return (LauncherVM)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        public static DependencyProperty DataProperty = DependencyProperty.Register("Data", typeof(LauncherVM), typeof(Launcher));

    }
}
