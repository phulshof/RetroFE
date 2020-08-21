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
    /// Interaction logic for AddRemoveList.xaml
    /// </summary>
    public partial class AddRemoveList : UserControl
    {

        public ICommand AddListItemCommand
        {
            get { return (ICommand)GetValue(AddListItemCommandProperty); }
            set { SetValue(ListItemsSourceProperty, value); }
        }

        public static DependencyProperty AddListItemCommandProperty = DependencyProperty.Register("AddListItemCommand", typeof(ICommand), typeof(AddRemoveList));

        public ICommand RemoveListItemCommand
        {
            get { return (ICommand)GetValue(RemoveListItemCommandProperty); }
            set { SetValue(ListItemsSourceProperty, value); }
        }

        public static DependencyProperty RemoveListItemCommandProperty = DependencyProperty.Register("RemoveListItemCommand", typeof(ICommand), typeof(AddRemoveList));

        public System.Collections.IEnumerable ListItemsSource
        {
            get { return (ObservableCollection<System.Collections.IEnumerable>)GetValue(ListItemsSourceProperty); }
            set { SetValue(ListItemsSourceProperty, value); }
        }

        public static DependencyProperty ListItemsSourceProperty = DependencyProperty.Register("ListItemsSource", typeof(System.Collections.IEnumerable), typeof(AddRemoveList));


        public object SelectedItem
        {
            get { return (object)GetValue(SelectedItemProperty); }
            set { SetValue(SelectedItemProperty, value); }
        }

        public static DependencyProperty SelectedItemProperty = DependencyProperty.Register("SelectedItem", typeof(object), typeof(AddRemoveList));


        public String ListDisplayMemberPath
        {
            get { return (String)GetValue(ListDisplayMemberPathProperty); }
            set { SetValue(ListDisplayMemberPathProperty, value); }
        }

        public static DependencyProperty ListDisplayMemberPathProperty = DependencyProperty.Register("ListDisplayMemberPath", typeof(String), typeof(AddRemoveList));


        public AddRemoveList()
        {
            InitializeComponent();
        }

        private void HideAddPopup(object sender, RoutedEventArgs e)
        {
            AddPopup.IsOpen = false;
         //   AddName.Text = "";
        }
    }
}
