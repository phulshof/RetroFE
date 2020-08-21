using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.IO;
using Configuration;

namespace Configuration.ViewModel
{
    class LauncherListVM : INotifyPropertyChanged
    {
        ObservableCollection<LauncherVM> _LauncherCollection = new ObservableCollection<LauncherVM>();
        public ObservableCollection<LauncherVM> LauncherCollection
        {
            get { return _LauncherCollection; }
            set { _LauncherCollection = value; NotifyPropertyChanged("LauncherCollection"); }
        }

        LauncherVM _SelectedLauncher = null;
        public LauncherVM SelectedLauncher
        {
            get { return _SelectedLauncher; }
            set 
            {
                if (_SelectedLauncher != null)
                {
                    Save(_SelectedLauncher);
                }
                _SelectedLauncher = value; 
                NotifyPropertyChanged("SelectedLauncher"); 
            }
        }

        ICommand _AddListItemCommand;
        public ICommand AddListItemCommand
        {
            get
            {
                if (_AddListItemCommand == null)
                {
                    _AddListItemCommand = new RelayCommand(param => this.AddLauncher(param), param => this.CanAdd());
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
                    _RemoveListItemCommand = new RelayCommand(param => this.RemoveLauncher(), param => this.CanDelete());
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
            return (SelectedLauncher != null);
        }

        private void AddLauncher(object param)
        {

            LauncherVM l = new LauncherVM();
            l.Name = param as String;
            NotifyPropertyChanged("LauncherCollection");
            ConfFileSaver saver = new ConfFileSaver();

            //todo change path
            if (!File.Exists(RetroFE.GetAbsolutePath() + "/Launchers/" + l.Name + ".conf"))
            {
                LauncherCollection.Add(l);
                saver.Save(RetroFE.GetAbsolutePath() + "/Launchers/" + l.Name + ".conf");
            }
        }

        public void Save(LauncherVM launcher)
        {
            if (launcher == null) return;

            ConfFileSaver s = new ConfFileSaver();
            s.AddOption("executable", launcher.ExecutablePath);
            s.AddOption("arguments", launcher.Arguments);

            //todo: change location
            string path = RetroFE.GetAbsolutePath() + "/Launchers/" + SelectedLauncher.Name + ".conf";
            s.Save(path);
        }
        
        private bool RemoveLauncher()
        {
            //todo: change location
            string path = RetroFE.GetAbsolutePath() + "/Launchers/" + SelectedLauncher.Name + ".conf";
            if (File.Exists(path))
            {
                File.Delete(path);
            }

            LauncherCollection.Remove(SelectedLauncher);

            return true;
        }
    }
}
