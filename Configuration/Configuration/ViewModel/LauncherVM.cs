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
    public class LauncherVM : INotifyPropertyChanged
    {
        String _Name;
        public String Name
        {
            get { return _Name; }
            set { _Name = value; NotifyPropertyChanged("Name"); }
        }

        String _ExecutablePath;
        public String ExecutablePath
        {
            get { return _ExecutablePath; }
            set { _ExecutablePath = value; NotifyPropertyChanged("ExecutablePath"); }
        }

        String _Arguments;
        public String Arguments
        {
            get { return _Arguments; }
            set { _Arguments = value; NotifyPropertyChanged("Arguments"); }
        }
        
        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged(String propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        ICommand _BrowseFileCommand;
        public ICommand BrowseFileCommand
        {
            get
            {
                if (_BrowseFileCommand == null)
                {
                    _BrowseFileCommand = new RelayCommand(param => this.BrowseFile(), param => true);
                }

                return _BrowseFileCommand;
            }
        }


        public void BrowseFile()
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Multiselect = false;

            if (dialog.ShowDialog() == DialogResult.OK)
            {
                ExecutablePath = dialog.FileName;
            }
        }

    }
}
