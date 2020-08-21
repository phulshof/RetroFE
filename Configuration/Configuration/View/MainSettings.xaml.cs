using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
    /// Interaction logic for MainSettings.xaml
    /// </summary>
    public partial class MainSettings : UserControl
    {
        public MainVM Data
        {
            get { return (MainVM)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        public static DependencyProperty DataProperty = DependencyProperty.Register("Data", typeof(MainVM), typeof(MainSettings));

        public System.Collections.IEnumerable Launchers
        {
            get { return (System.Collections.IEnumerable)GetValue(LaunchersProperty); }
            set { SetValue(LaunchersProperty, value); }
        }
        public static DependencyProperty LaunchersProperty = DependencyProperty.Register("Launchers", typeof(System.Collections.IEnumerable), typeof(MainSettings));

        public System.Collections.IEnumerable Collections
        {
            get { return (System.Collections.IEnumerable)GetValue(CollectionsProperty); }
            set { SetValue(CollectionsProperty, value); }
        }
        public static DependencyProperty CollectionsProperty = DependencyProperty.Register("Collections", typeof(System.Collections.IEnumerable), typeof(MainSettings));


        public MainSettings()
        {
            InitializeComponent();
        }
    }
}
