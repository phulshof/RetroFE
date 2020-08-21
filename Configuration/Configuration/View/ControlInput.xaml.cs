using System;
using System.Collections.Generic;
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
    /// Interaction logic for ControlInput.xaml
    /// </summary>
    public partial class ControlInput : UserControl
    {
        Dictionary<int, string> SDLAsciiDescMap = new Dictionary<int, string>();
        public ControlInput()
        {
            InitializeComponent();
            SDLAsciiDescMap[0x8] = "Backspace";
            SDLAsciiDescMap[0x9] = "Tab";
            SDLAsciiDescMap[0x0C] = "Clear";
            SDLAsciiDescMap[0x0D] = "Return";
            SDLAsciiDescMap[0x13] = "Pause";
            SDLAsciiDescMap[0x14] = "CapsLock";
            SDLAsciiDescMap[0x1B] = "Escape";
            SDLAsciiDescMap[0x20] = "Space";
            SDLAsciiDescMap[0x21] = "PageUp";
            SDLAsciiDescMap[0x22] = "PageDown";
            SDLAsciiDescMap[0x23] = "End";
            SDLAsciiDescMap[0x24] = "Home";
            SDLAsciiDescMap[0x25] = "Left";
            SDLAsciiDescMap[0x26] = "Up";
            SDLAsciiDescMap[0x27] = "Right";
            SDLAsciiDescMap[0x28] = "Down";
            SDLAsciiDescMap[0x29] = "Select";
            SDLAsciiDescMap[0x2B] = "Execute";
            SDLAsciiDescMap[0x2C] = "PrintScreen";
            SDLAsciiDescMap[0x2D] = "Insert";
            SDLAsciiDescMap[0x2E] = "Delete";
            SDLAsciiDescMap[0x2F] = "Help";
            SDLAsciiDescMap[0x30] = "0";
            SDLAsciiDescMap[0x31] = "1";
            SDLAsciiDescMap[0x32] = "2";
            SDLAsciiDescMap[0x33] = "3";
            SDLAsciiDescMap[0x34] = "4";
            SDLAsciiDescMap[0x35] = "5";
            SDLAsciiDescMap[0x36] = "6";
            SDLAsciiDescMap[0x37] = "7";
            SDLAsciiDescMap[0x38] = "8";
            SDLAsciiDescMap[0x39] = "9";
            SDLAsciiDescMap[0x41] = "A";
            SDLAsciiDescMap[0x42] = "B";
            SDLAsciiDescMap[0x43] = "C";
            SDLAsciiDescMap[0x44] = "D";
            SDLAsciiDescMap[0x45] = "E";
            SDLAsciiDescMap[0x46] = "F";
            SDLAsciiDescMap[0x47] = "G";
            SDLAsciiDescMap[0x48] = "H";
            SDLAsciiDescMap[0x49] = "I";
            SDLAsciiDescMap[0x4A] = "J";
            SDLAsciiDescMap[0x4B] = "K";
            SDLAsciiDescMap[0x4C] = "L";
            SDLAsciiDescMap[0x4D] = "M";
            SDLAsciiDescMap[0x4E] = "N";
            SDLAsciiDescMap[0x4F] = "O";
            SDLAsciiDescMap[0x50] = "P";
            SDLAsciiDescMap[0x51] = "Q";
            SDLAsciiDescMap[0x52] = "R";
            SDLAsciiDescMap[0x53] = "S";
            SDLAsciiDescMap[0x54] = "T";
            SDLAsciiDescMap[0x55] = "U";
            SDLAsciiDescMap[0x56] = "V";
            SDLAsciiDescMap[0x57] = "W";
            SDLAsciiDescMap[0x58] = "X";
            SDLAsciiDescMap[0x59] = "Y";
            SDLAsciiDescMap[0x5A] = "Z";
            SDLAsciiDescMap[0x5B] = "Left GUI";
            SDLAsciiDescMap[0x5C] = "Right GUI";
            SDLAsciiDescMap[0x5D] = "Application";
            SDLAsciiDescMap[0x5F] = "Sleep";
            SDLAsciiDescMap[0x60] = "Keypad 0";
            SDLAsciiDescMap[0x61] = "Keypad 1";
            SDLAsciiDescMap[0x62] = "Keypad 2";
            SDLAsciiDescMap[0x63] = "Keypad 3";
            SDLAsciiDescMap[0x64] = "Keypad 4";
            SDLAsciiDescMap[0x65] = "Keypad 5";
            SDLAsciiDescMap[0x66] = "Keypad 6";
            SDLAsciiDescMap[0x67] = "Keypad 7";
            SDLAsciiDescMap[0x68] = "Keypad 8";
            SDLAsciiDescMap[0x69] = "Keypad 9";
            SDLAsciiDescMap[0x6A] = "*";
            SDLAsciiDescMap[0x6B] = "+";
            SDLAsciiDescMap[0x6C] = "Separator";
            SDLAsciiDescMap[0x6D] = "-";
            SDLAsciiDescMap[0x6E] = ".";
            SDLAsciiDescMap[0x6F] = "/";
            SDLAsciiDescMap[0x70] = "F1";
            SDLAsciiDescMap[0x71] = "F2";
            SDLAsciiDescMap[0x72] = "F3";
            SDLAsciiDescMap[0x73] = "F4";
            SDLAsciiDescMap[0x74] = "F5";
            SDLAsciiDescMap[0x75] = "F6";
            SDLAsciiDescMap[0x76] = "F7";
            SDLAsciiDescMap[0x77] = "F8";
            SDLAsciiDescMap[0x78] = "F9";
            SDLAsciiDescMap[0x79] = "F10";
            SDLAsciiDescMap[0x7A] = "F11";
            SDLAsciiDescMap[0x7B] = "F12";
            SDLAsciiDescMap[0x7C] = "F13";
            SDLAsciiDescMap[0x7D] = "F14";
            SDLAsciiDescMap[0x7E] = "F15";
            SDLAsciiDescMap[0x7F] = "F16";
            SDLAsciiDescMap[0x80] = "F17";
            SDLAsciiDescMap[0x81] = "F18";
            SDLAsciiDescMap[0x82] = "F19";
            SDLAsciiDescMap[0x83] = "F20";
            SDLAsciiDescMap[0x84] = "F21";
            SDLAsciiDescMap[0x85] = "F22";
            SDLAsciiDescMap[0x86] = "F23";
            SDLAsciiDescMap[0x87] = "F24";
            SDLAsciiDescMap[0x90] = "Numlock";
            SDLAsciiDescMap[0x91] = "ScrollLock";
            SDLAsciiDescMap[0xA0] = "Left Shift";
            SDLAsciiDescMap[0xA1] = "Right Shift";
            SDLAsciiDescMap[0xA2] = "Left Ctrl";
            SDLAsciiDescMap[0xA3] = "Right Ctrl";
            SDLAsciiDescMap[0xA4] = "Left Menu";
            SDLAsciiDescMap[0xA5] = "Right Menu";
            SDLAsciiDescMap[0xC0] = "`";

        }

        public ControllerVM Data
        {
            get { return (ControllerVM)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        public static DependencyProperty DataProperty = DependencyProperty.Register("Data", typeof(ControllerVM), typeof(ControlInput));


        private void TextBlock_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            TextBox tb = sender as TextBox;
            System.Windows.Forms.KeysConverter kc = new System.Windows.Forms.KeysConverter();
            int key = System.Windows.Input.KeyInterop.VirtualKeyFromKey(e.Key);
            int systemKey = System.Windows.Input.KeyInterop.VirtualKeyFromKey(e.SystemKey);

            if (SDLAsciiDescMap.ContainsKey(key))
            {
                tb.Text = SDLAsciiDescMap[key];
            }
            else 
                if (SDLAsciiDescMap.ContainsKey(systemKey))
            {
                tb.Text = SDLAsciiDescMap[systemKey];
            }
            else
            {
                tb.Text = key + " " + e.SystemKey + " (none)";
            }
            //todo, implement a map
            
            e.Handled = true;
        }
    }

}
