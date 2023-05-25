using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.IO;
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
namespace Configuration
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 

    public partial class MainWindow : Window
    {
        private TabItem LastSelectedTabItem;
        public MainWindow()
        {

            InitializeComponent();
            if (!File.Exists(RetroFE.GetAbsolutePath() + "/retrofe/RetroFE.exe"))
            {
                MessageBox.Show("Could not find RetroFE executable. Exiting.");
                Close();
            }
            else
            {
                MessageBox.Show("This tool has not had a lot of testing. " + Environment.NewLine + Environment.NewLine + "Back up your files and use at your own risk before using this tool.");
                ObservableCollection<string> layouts = new ObservableCollection<string>();
                LauncherListVM launcher = this.TryFindResource("LauncherConfig") as LauncherListVM;
                CollectionListVM collection = this.TryFindResource("CollectionConfig") as CollectionListVM;
                ControllerVM controller = this.TryFindResource("ControllerConfig") as ControllerVM;
                MainVM main = this.TryFindResource("MainConfig") as MainVM;
                Builder b = new Builder();

                launcher.LauncherCollection = b.LoadLaunchers();
                collection.CollectionList = b.LoadCollections(launcher.LauncherCollection);
                main.Layouts = b.LoadLayouts();
                b.LoadMain(ref main, main.Layouts, collection.CollectionList);
                b.LoadController(ref controller);
            }
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            TabControl control = sender as TabControl;

            if (LastSelectedTabItem != null)
            {
                LastSelectedTabItem.Focus();
                Save((string)LastSelectedTabItem.Header);
            }

            if (control != null && control.SelectedValue != null)
            {
                LastSelectedTabItem = control.SelectedItem as TabItem;
            }
        }

        private void TabControl_FocusableChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            TabControl control = sender as TabControl;

            if(control.SelectedItem != null)
            {
                TabItem item = control.SelectedItem as TabItem;
                item.Focus();
                Save((string)item.Header);
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (ConfigurationTabControl.SelectedItem != null)
            {
                TabItem item = ConfigurationTabControl.SelectedItem as TabItem;
                item.Focus();
                Save((string)item.Header);
            }

        }

        private void Save(string tabItem)
        {
            if (tabItem == "General")
            {
                MainVM main = this.TryFindResource("MainConfig") as MainVM;
                main.Save();
            }
            else if (tabItem == "Controller")
            {
                ControllerVM vm = this.TryFindResource("ControllerConfig") as ControllerVM;
                vm.Save();
            }
            else if (tabItem == "Launchers")
            {
                LauncherListVM vm = this.TryFindResource("LauncherConfig") as LauncherListVM;
                vm.Save(vm.SelectedLauncher);
            }
            else if (tabItem == "Collections")
            {
                CollectionListVM vm = this.TryFindResource("CollectionConfig") as CollectionListVM;
                vm.Save(vm.SelectedCollection);
            }
        }

    }
}
