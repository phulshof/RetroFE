using System;
using System.IO;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using Configuration;

namespace Configuration.ViewModel
{
    class CollectionListVM : INotifyPropertyChanged
    {
        ObservableCollection<CollectionVM> _CollectionList = new ObservableCollection<CollectionVM>();
        public ObservableCollection<CollectionVM> CollectionList
        {
            get { return _CollectionList; }
            set { _CollectionList = value; NotifyPropertyChanged("CollectionList"); }
        }

        CollectionVM _SelectedCollection = null;
        public CollectionVM SelectedCollection
        {
            get { return _SelectedCollection; }
            set {
                if (_SelectedCollection != null)
                {
                    Save(_SelectedCollection);
                }
                _SelectedCollection = value; 
                NotifyPropertyChanged("SelectedCollection"); 
            }
        }

        ICommand _AddListItemCommand;
        public ICommand AddListItemCommand
        {
            get
            {
                if (_AddListItemCommand == null)
                {
                    _AddListItemCommand = new RelayCommand(param => this.AddCollection(param), param => this.CanAdd());
                }

                return _AddListItemCommand;
            }
        }
        ICommand _RemoveListItemCommand;
        public ICommand RemoveListItemCommand
        {
            get
            {
                if (_RemoveListItemCommand == null)
                {
                    _RemoveListItemCommand = new RelayCommand(param => this.RemoveCollection(), param => this.CanDelete());
                }

                return _RemoveListItemCommand;
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged(String propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }


        private bool CanAdd()
        {
            return true;
        }

        private bool CanDelete()
        {
            return (SelectedCollection != null);
        }

        private void AddCollection(object param)
        {

            CollectionVM cvm = new CollectionVM();
            cvm.Name = param as String;
            NotifyPropertyChanged("CollectionList");

            ConfFileSaver settings = new ConfFileSaver();
            ConfFileSaver include = new ConfFileSaver();
            ConfFileSaver exclude = new ConfFileSaver();
            MenuParser menu = new MenuParser();

            //todo change path
            string path = RetroFE.GetAbsolutePath() + "/Collections/" + cvm.Name;
            if (!Directory.Exists(path))
            {
                Directory.CreateDirectory(path);
            }

            if (!File.Exists(path + "/Settings.conf"))
            {
                CollectionList.Add(cvm);
                settings.Save(path + "/Settings.conf");

                if (!File.Exists(path + "/Include.txt"))
                {
                    include.Save(path + "/Include.txt");
                }
                if (!File.Exists(path + "/Exclude.txt"))
                {
                    exclude.Save(path + "/Exclude.txt");
                }

                //menu.Save(path + "/Menu.xml");
            }
        }

        public void Save(CollectionVM cvm)
        {
            if (cvm == null) return;

            ConfFileSaver s = new ConfFileSaver();

            if (!cvm.IsDefaultLayout)
            {
                s.AddOption("layout", cvm.Layout);
            }
            s.AddOption("launcher", (cvm.Launcher == null) ? "" : cvm.Launcher.Name);
            s.AddOption("list.path", cvm.ListPath);
            s.AddOption("list.extensions", cvm.FileExtensions);
            s.AddOption("media.box", cvm.MediaPathBox);
            s.AddOption("media.cart", cvm.MediaPathCart);
            s.AddOption("media.logo", cvm.MediaPathLogo);
            s.AddOption("media.snap", cvm.MediaPathSnap);
            s.AddOption("media.title", cvm.MediaPathTitle);
            s.AddOption("media.video", cvm.MediaPathVideo);

            //todo: change serverPath
            string path = RetroFE.GetAbsolutePath() + "/Collections/" + cvm.Name + "/Settings.conf";
            s.Save(path);
        }

        private bool RemoveCollection()
        {
            //todo: change location
            string path = RetroFE.GetAbsolutePath() + "/Launchers/" + SelectedCollection.Name + ".conf";
            if (File.Exists(path))
            {
                File.Delete(path);
            }

            CollectionList.Remove(SelectedCollection);

            return true;
        }
    }
}
