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
    public class ControllerVM : INotifyPropertyChanged
    {
        String _ScrollPrevious;
        public String ScrollPrevious
        {
            get { return _ScrollPrevious; }
            set { _ScrollPrevious = value; NotifyPropertyChanged("ScrollPrevious"); }
        }

        String _ScrollNext;
        public String ScrollNext
        {
            get { return _ScrollNext; }
            set { _ScrollNext = value; NotifyPropertyChanged("ScrollNext"); }
        }

        String _PageUp;
        public String PageUp
        {
            get { return _PageUp; }
            set { _PageUp = value; NotifyPropertyChanged("PageUp"); }
        }

        String _PageDown;
        public String PageDown
        {
            get { return _PageDown; }
            set { _PageDown = value; NotifyPropertyChanged("PageDown"); }
        }

        String _SelectItem;
        public String SelectItem
        {
            get { return _SelectItem; }
            set { _SelectItem = value; NotifyPropertyChanged("SelectItem"); }
        }

        String _Back;
        public String Back
        {
            get { return _Back; }
            set { _Back = value; NotifyPropertyChanged("Back"); }
        }

        String _Quit;
        public String Quit
        {
            get { return _Quit; }
            set { _Quit = value; NotifyPropertyChanged("Quit"); }
        }

        public void Save()
        {
            ConfFileSaver s = new ConfFileSaver();

            s.AddOption("previousItem", ScrollPrevious);
            s.AddOption("nextItem", ScrollNext);
            s.AddOption("pageUp", PageUp);
            s.AddOption("pageDown", PageDown);

            s.AddOption("select", SelectItem);
            s.AddOption("back", Back);
            s.AddOption("quit", Quit);

            //todo: change location
            string path = RetroFE.GetAbsolutePath() + "/Controls.conf";
            s.Save(path);
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
