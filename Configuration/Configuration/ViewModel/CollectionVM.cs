using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Forms;
using Configuration;

namespace Configuration.ViewModel
{
    public class CollectionVM : INotifyPropertyChanged
    {
        String _Name;
        public String Name
        {
            get { return _Name; }
            set { _Name = value; NotifyPropertyChanged("Name"); }
        }

        String _ListPath;
        public String ListPath
        {
            get { return _ListPath; }
            set { _ListPath = value; NotifyPropertyChanged("ListPath"); }
        }

        String _FileExtensions = null;
        public String FileExtensions
        {
            get { return _FileExtensions; }
            set { _FileExtensions = value; NotifyPropertyChanged("FileExtensions"); }
        }

        LauncherVM _Launcher = null;
        public LauncherVM Launcher
        {
            get { return _Launcher; }
            set { _Launcher = value; NotifyPropertyChanged("Launcher"); }
        }

        String _Layout = null;
        public String Layout
        {
            get { return _Layout; }
            set { _Layout = value; NotifyPropertyChanged("Layout"); }
        }

        bool _IsDefaultLayout;
        public bool IsDefaultLayout
        {
            get { return _IsDefaultLayout; }
            set { _IsDefaultLayout = value; NotifyPropertyChanged("IsDefaultLayout"); }
        }


        String _MediaPathVideo;
        public String MediaPathVideo
        {
            get { return _MediaPathVideo; }
            set { _MediaPathVideo = value; NotifyPropertyChanged("MediaPathVideo"); }
        }

        String _MediaPathTitle;
        public String MediaPathTitle
        {
            get { return _MediaPathTitle; }
            set { _MediaPathTitle = value; NotifyPropertyChanged("MediaPathTitle"); }
        }
        String _MediaPathLogo;
        public String MediaPathLogo
        {
            get { return _MediaPathLogo; }
            set { _MediaPathLogo = value; NotifyPropertyChanged("MediaPathLogo"); }
        }
        String _MediaPathBox;
        public String MediaPathBox
        {
            get { return _MediaPathBox; }
            set { _MediaPathBox = value; NotifyPropertyChanged("MediaPathBox"); }
        }
        String _MediaPathCart;
        public String MediaPathCart
        {
            get { return _MediaPathCart; }
            set { _MediaPathCart = value; NotifyPropertyChanged("MediaPathCart"); }
        }
        String _MediaPathSnap;
        public String MediaPathSnap
        {
            get { return _MediaPathSnap; }
            set { _MediaPathSnap = value; NotifyPropertyChanged("MediaPathSnap"); }
        }

        ObservableCollection<string> _Submenus;
        public ObservableCollection<string> Submenus
        {
            get { return _Submenus; }
            set { _Submenus = value; NotifyPropertyChanged("Submenus"); }
        }

        ICommand _DeleteSubmenuCommand;
        public ICommand DeleteSubmenuCommand
        {
            get
            {
                if (_DeleteSubmenuCommand == null)
                {
                    _DeleteSubmenuCommand = new RelayCommand(param => this.DeleteSubmenu((string)param), param => true);
                }

                return _DeleteSubmenuCommand;
            }
        }



        ICommand _BrowseFolderCommand;
        public ICommand BrowseFolderCommand
        {
            get
            {
                if (_BrowseFolderCommand == null)
                {
                    _BrowseFolderCommand = new RelayCommand(param => this.BrowseFolder((string)param), param => true);
                }

                return _BrowseFolderCommand;
            }
        }

        public void DeleteSubmenu(string reserved)
        {
        }

        public void BrowseFolder(string type)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();

            if(dialog.ShowDialog() == DialogResult.OK)
            {
                string selectedPath = dialog.SelectedPath;

                if (type == "Video")
                {
                    MediaPathVideo = selectedPath;
                }
                else if (type == "Title")
                {
                    MediaPathTitle = selectedPath;
                }
                else if (type == "Snap")
                {
                    MediaPathSnap = selectedPath;
                }
                else if (type == "Logo")
                {
                    MediaPathLogo = selectedPath;
                }
                else if (type == "Cart")
                {
                    MediaPathCart = selectedPath;
                }
                else if (type == "Box")
                {
                    MediaPathBox = selectedPath;
                }
                else if (type == "Item")
                {
                    ListPath = selectedPath;
                }
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

    }
}
