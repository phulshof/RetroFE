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
    /// Interaction logic for Collection.xaml
    /// </summary>
    public partial class Collection : UserControl
    {

        public CollectionVM Data
        {
            get { return (CollectionVM)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        public static DependencyProperty DataProperty = DependencyProperty.Register("Data", typeof(CollectionVM), typeof(Collection));

        public System.Collections.IEnumerable LauncherCollection
        {
            get { return (System.Collections.IEnumerable)GetValue(LauncherCollectionProperty); }
            set { SetValue(LauncherCollectionProperty, value); }
        }

        public static DependencyProperty LauncherCollectionProperty = DependencyProperty.Register("LauncherCollection", typeof(System.Collections.IEnumerable), typeof(Collection));

        public System.Collections.IEnumerable Layouts
        {
            get { return (System.Collections.IEnumerable)GetValue(LayoutsProperty); }
            set { SetValue(LayoutsProperty, value); }
        }

        public static DependencyProperty LayoutsProperty = DependencyProperty.Register("Layouts", typeof(System.Collections.IEnumerable), typeof(Collection));


        public System.Collections.IEnumerable Collections
        {
            get { return (System.Collections.IEnumerable)GetValue(CollectionsProperty); }
            set { SetValue(CollectionsProperty, value); }
        }

        public static DependencyProperty CollectionsProperty = DependencyProperty.Register("Collections", typeof(System.Collections.IEnumerable), typeof(Collection));

        public Collection()
        {
            InitializeComponent();
        }

    }
}
