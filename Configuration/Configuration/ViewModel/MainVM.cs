using System;
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
    public class MainVM : INotifyPropertyChanged
    {
        bool _IsFullscreen;
        public bool IsFullscreen
        {
            get { return _IsFullscreen; }
            set { _IsFullscreen = value; NotifyPropertyChanged("IsFullscreen"); }
        }

        int _HorizontalResolution;
        public int HorizontalResolution
        {
            get { return _HorizontalResolution; }
            set { _HorizontalResolution = value; NotifyPropertyChanged("HorizontalResolution"); }
        }

        bool _IsHorizontalStretch;
        public bool IsHorizontalStretch
        {
            get { return _IsHorizontalStretch; }
            set { _IsHorizontalStretch = value; NotifyPropertyChanged("IsHorizontalStretch"); }
        }

        bool _IsVerticalStretch;
        public bool IsVerticalStretch
        {
            get { return _IsVerticalStretch; }
            set { _IsVerticalStretch = value; NotifyPropertyChanged("IsVerticalStretch"); }
        }

        int _VerticalResolution;
        public int VerticalResolution
        {
            get { return _VerticalResolution; }
            set { _VerticalResolution = value; NotifyPropertyChanged("VerticalResolution"); }
        }

        String _Layout;
        public String Layout
        {
            get { return _Layout; }
            set { _Layout = value; NotifyPropertyChanged("Layout"); }
        }

        ObservableCollection<string> _Layouts;
        public ObservableCollection<string> Layouts
        {
            get { return _Layouts; }
            set { _Layouts = value; NotifyPropertyChanged("Layouts"); }
        }


        bool _IsMouseHidden;
        public bool IsMouseHidden
        {
            get { return _IsMouseHidden; }
            set { _IsMouseHidden = value; NotifyPropertyChanged("IsMouseHidden"); }
        }

        bool _IsParenthesisVisible;
        public bool IsParenthesisVisible
        {
            get { return _IsParenthesisVisible; }
            set { _IsParenthesisVisible = value; NotifyPropertyChanged("IsParenthesisVisible"); }
        }

        bool _IsBracesVisible;
        public bool IsBracesVisible
        {
            get { return _IsBracesVisible; }
            set { _IsBracesVisible = value; NotifyPropertyChanged("IsBracesVisible"); }
        }

        CollectionVM _FirstCollection;
        public CollectionVM FirstCollection
        {
            get { return _FirstCollection; }
            set { _FirstCollection = value; NotifyPropertyChanged("FirstCollection"); }
        }

        bool _IsVideoEnabled;
        public bool IsVideoEnabled
        {
            get { return _IsVideoEnabled; }
            set { _IsVideoEnabled = value; NotifyPropertyChanged("IsVideoEnabled"); }
        }

        int _VideoLoop;
        public int VideoLoop
        {
            get { return _VideoLoop; }
            set { _VideoLoop = value; NotifyPropertyChanged("VideoLoop"); }
        }

        bool _IsInfiniteLoop;
        public bool IsInfiniteLoop
        {
            get { return _IsInfiniteLoop; }
            set { _IsInfiniteLoop = value; NotifyPropertyChanged("IsInfiniteLoop"); }
        }

        bool _IsExitOnFirstBack;
        public bool IsExitOnFirstBack
        {
            get { return _IsExitOnFirstBack; }
            set { _IsExitOnFirstBack = value; NotifyPropertyChanged("IsExitOnFirstBack"); }
        }

        int _AttractModeTime;
        public int AttractModeTime
        {
            get { return _AttractModeTime; }
            set { _AttractModeTime = value; NotifyPropertyChanged("AttractModeTime"); }
        }

        bool _IsAttractModeEnabled;
        public bool IsAttractModeEnabled
        {
            get { return _IsAttractModeEnabled; }
            set { _IsAttractModeEnabled = value; NotifyPropertyChanged("IsAttractModeEnabled"); }
        }


        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged(String propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public void Save()
        {
            ConfFileSaver s = new ConfFileSaver();

            if(IsVerticalStretch)
            {
                s.AddOption("vertical", "stretch");
            }
            else
            {
                s.AddOption("vertical", VerticalResolution);
            }

            if(IsHorizontalStretch)
            {
                s.AddOption("horizontal", "stretch");
            }
            else
            {
                s.AddOption("horizontal", HorizontalResolution);
            }

            s.AddOption("fullscreen", IsFullscreen);
            s.AddOption("layout", (Layout == null) ? "Default" : Layout);
            s.AddOption("hideMouse", IsMouseHidden);
            s.AddOption("showParenthesis", IsParenthesisVisible);
            s.AddOption("showSquareBrackets", IsBracesVisible);
            s.AddOption("firstCollection", (FirstCollection == null) ? "Main" : FirstCollection.Name);
            
            s.AddOption("videoEnable", IsVideoEnabled);
            s.AddOption("videoLoop", VideoLoop);
            s.AddOption("exitOnFirstPageBack", IsExitOnFirstBack);
            s.AddOption("attractModeTime", (IsAttractModeEnabled) ? AttractModeTime : 0);

    
            //todo: change location
            string path = RetroFE.GetAbsolutePath() + "/Settings.conf";
            s.Save(path);
        }

    }
}
