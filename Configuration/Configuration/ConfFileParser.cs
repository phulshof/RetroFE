using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
 
public class ConfFileParser
{
    private Dictionary<string, string> keyPairs = new Dictionary<string,string>();
    private String FilePath;
 
    /// <summary>
    /// Opens the INI file at the given path and enumerates the values in the IniParser.
    /// </summary>
    /// <param name="iniPath">Full path to INI file.</param>
    public ConfFileParser(String filePath)
    {
        TextReader iniFile = null;
        String strLine = null;
 
        FilePath = filePath;
 
        if (File.Exists(filePath))
        {
            try
            {
                iniFile = new StreamReader(filePath);
 
                strLine = iniFile.ReadLine();
 
                while (strLine != null)
                {
                    strLine = strLine.Trim();
 
                    if (strLine != "")
                    {
                        int commentStart = strLine.IndexOf("#");
                        if(commentStart > 0)
                        {
                            strLine = strLine.Substring(0, commentStart-1);
                        }

                        string[] propertyPair = strLine.Split(new char[] { '=' }, 2);
 
                        if (propertyPair.Length > 1)
                        {
                            string key = propertyPair[0].Trim();
                            string value = propertyPair[1].Trim();
                            keyPairs.Add(key, value);
                        }
                    }
 
                    strLine = iniFile.ReadLine();
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
        else
            throw new FileNotFoundException("Unable to locate " + filePath);
 
    }
 
    /// <summary>
    /// Returns the value for the given section, key pair.
    /// </summary>
    /// <param name="sectionName">Section name.</param>
    /// <param name="settingName">Key name.</param>
    public String GetSetting(String settingName)
    {
        if(keyPairs.ContainsKey(settingName))
            return (String)keyPairs[settingName];

        return "";
    }
 
    /// <summary>
    /// Adds or replaces a setting to the table to be saved.
    /// </summary>
    /// <param name="sectionName">Section to add under.</param>
    /// <param name="settingName">Key name to add.</param>
    /// <param name="settingValue">Value of key.</param>
    public void AddSetting(String settingName, String settingValue)
    {
        keyPairs[settingName] = settingValue;
    }
 
    /// <summary>
    /// Remove a setting.
    /// </summary>
    /// <param name="sectionName">Section to add under.</param>
    /// <param name="settingName">Key name to add.</param>
    public void DeleteSetting(String settingName)
    {
        if (keyPairs.ContainsKey(settingName))
            keyPairs.Remove(settingName);
    }
 
    /// <summary>
    /// Save settings to new file.
    /// </summary>
    /// <param name="newFilePath">New file path.</param>
    public void SaveSettings(String newFilePath)
    {
        String tmpValue = "";
        String strToSave = "";
 
        foreach (string property in keyPairs.Keys)
        {
            tmpValue = (String)keyPairs[property];
 
            if (tmpValue != null)
                tmpValue = "=" + tmpValue;
 
            strToSave += (property + tmpValue + "\r\n");
        }
 
        strToSave += "\r\n";
 
        try
        {
            TextWriter tw = new StreamWriter(newFilePath);
            tw.Write(strToSave);
            tw.Close();
        }
        catch (Exception ex)
        {
            throw ex;
        }
    }
 
    /// <summary>
    /// Save settings back to ini file.
    /// </summary>
    public void SaveSettings()
    {
        SaveSettings(FilePath);
    }
}