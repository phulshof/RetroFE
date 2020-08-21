using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Configuration
{
    class RetroFE
    {
        public static string GetAbsolutePath()
        {
            string path = Environment.GetEnvironmentVariable("RETROFE_PATH");

            if (path == null)
            {
                path = new FileInfo(System.Reflection.Assembly.GetEntryAssembly().Location).Directory.FullName;    
            }

            return path;
        }
    }
}
