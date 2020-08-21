using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Configuration.ViewModel;
using System.IO;
namespace Configuration
{
    public class Builder
    {
        public void LoadMain(ref MainVM main, ObservableCollection<string>layouts, ObservableCollection<CollectionVM> collections)
        {
            //todo :make paths relative
            ConfFileParser ini = new ConfFileParser(RetroFE.GetAbsolutePath() + "/Settings.conf");

            main.IsFullscreen = ToBool(ini.GetSetting("fullscreen"));
            main.IsHorizontalStretch = ToBool(ini.GetSetting("horizontal"));
            main.IsVerticalStretch = ToBool(ini.GetSetting("vertical"));
            if (!main.IsHorizontalStretch)
            {
                main.HorizontalResolution = Convert.ToInt32(ini.GetSetting("horizontal"));
            }
            if (!main.IsVerticalStretch)
            {
                main.VerticalResolution = Convert.ToInt32(ini.GetSetting("vertical"));
            }

            main.Layout = layouts.FirstOrDefault(row => row == ini.GetSetting("layout"));

            main.IsMouseHidden = ToBool(ini.GetSetting("hideMouse"));
            main.IsParenthesisVisible = !ToBool(ini.GetSetting("showParenthesis"));
            main.IsBracesVisible = !ToBool(ini.GetSetting("showSquareBrackets"));
            string firstCollection = ini.GetSetting("firstCollection");
            if(firstCollection == "")
            {
                firstCollection = "Main";
            }
            main.FirstCollection = collections.FirstOrDefault(row => row.Name == firstCollection);
            main.IsVideoEnabled = ToBool(ini.GetSetting("videoEnable"));
            main.VideoLoop = Convert.ToInt32(ini.GetSetting("videoLoop"));
            main.IsInfiniteLoop = (main.VideoLoop == 0);
            main.IsExitOnFirstBack = ToBool(ini.GetSetting("exitOnFirstPageBack"));
            main.AttractModeTime = Convert.ToInt32(ini.GetSetting("attractModeTime"));
            main.IsAttractModeEnabled = (main.AttractModeTime != 0);
        }

        public void LoadController(ref ControllerVM vm)
        {
            //todo :make paths relative
            ConfFileParser ini = new ConfFileParser(RetroFE.GetAbsolutePath() + "/Controls.conf");
            vm.ScrollNext = ini.GetSetting("nextItem");
            vm.ScrollPrevious = ini.GetSetting("previousItem");
            vm.PageUp = ini.GetSetting("pageUp");
            vm.PageDown = ini.GetSetting("pageDown");
            vm.SelectItem = ini.GetSetting("select");
            vm.Back = ini.GetSetting("back");
            vm.Quit = ini.GetSetting("quit");
        }

        public ObservableCollection<LauncherVM> LoadLaunchers()
        {
            //todo :make paths relative
            ObservableCollection<LauncherVM> launchers = new ObservableCollection<LauncherVM>();

            string[] files = Directory.GetFiles(RetroFE.GetAbsolutePath() + "/Launchers", "*.conf");

            foreach (string file in files)
            {
                LauncherVM vm = new LauncherVM();
                ConfFileParser ini = new ConfFileParser(file);

                vm.Name = System.IO.Path.GetFileNameWithoutExtension(file);
                vm.ExecutablePath = ini.GetSetting("executable");
                vm.Arguments = ini.GetSetting("arguments");
                launchers.Add(vm);
            }

            return launchers;
        }

        public ObservableCollection<CollectionVM> LoadCollections(ObservableCollection<LauncherVM> launchers)
        {
            //todo :make paths relative
            ObservableCollection<CollectionVM> collections = new ObservableCollection<CollectionVM>();

            string[] dirs = Directory.GetDirectories(RetroFE.GetAbsolutePath() + "/Collections");

            foreach (string dir in dirs)
            {
                string settingsFile = Path.Combine(dir, "Settings.conf");
                string menuFile = Path.Combine(dir, "Menu.xml");
                CollectionVM vm = new CollectionVM();
                ConfFileParser ini = new ConfFileParser(settingsFile);
                MenuParser mp = new MenuParser();
                string launcher = ini.GetSetting("launcher");
                vm.Name = System.IO.Path.GetFileNameWithoutExtension(dir);
                vm.Launcher = launchers.FirstOrDefault(row => row.Name == launcher);
                vm.ListPath = ini.GetSetting("list.path");
                vm.Layout = ini.GetSetting("layout");

                if (vm.Layout == "")
                {
                    vm.IsDefaultLayout = true;
                }
                vm.FileExtensions = ini.GetSetting("list.extensions");
                vm.MediaPathVideo = ini.GetSetting("media.video");
                vm.MediaPathTitle = ini.GetSetting("media.title");
                vm.MediaPathLogo = ini.GetSetting("media.logo");
                vm.MediaPathTitle = ini.GetSetting("media.title");
                vm.MediaPathSnap = ini.GetSetting("media.snap");
                vm.MediaPathBox = ini.GetSetting("media.box");
                vm.MediaPathCart = ini.GetSetting("media.cart");

                //todo: read submenus

                vm.Submenus = mp.ReadCollections(menuFile);
                collections.Add(vm);
            }

            return collections;
        }
        public ObservableCollection<string> LoadLayouts()
        {
            //todo :make paths relative
            ObservableCollection<string> layouts = new ObservableCollection<string>();

            string[] dirs = Directory.GetDirectories(RetroFE.GetAbsolutePath() + "/Layouts");

            foreach (string dir in dirs)
            {
                string layout = System.IO.Path.GetFileNameWithoutExtension(dir);
                layouts.Add(layout);
            }

            return layouts;
        }

        private bool ToBool(string value)
        {
            value = value.Trim().ToLower();

            return (value == "yes" || value == "true" || value == "stretch");
        }
    }
}
