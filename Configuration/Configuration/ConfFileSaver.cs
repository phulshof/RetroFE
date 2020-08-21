using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Configuration
{
    class ConfFileSaver
    {
        public void AddOption(string key, string value)
        {
            Options.Add(key, value);
        }

        public void AddOption(string key, bool value)
        {
            string strValue = (value) ? "yes" : "no";
            Options.Add(key, strValue);
        }


        public void AddOption(string key, int value)
        {
            string strValue = Convert.ToString(value);

            Options.Add(key, strValue);
        }

        public void Save(string filePath)
        {
            TextWriter iniFile = null;

            try
            {
                iniFile = new StreamWriter(filePath);

                foreach (KeyValuePair<string, string> option in Options)
                {
                    iniFile.Write(option.Key + " = " + option.Value + Environment.NewLine);
                }

            }
            catch (Exception ex)
            {
                throw ex;
            }
            finally
            {
                if (iniFile != null)
                    iniFile.Close();
            }
        }

        private Dictionary<string, string> Options = new Dictionary<string,string>();
    }
}
