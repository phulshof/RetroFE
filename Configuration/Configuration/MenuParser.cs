using System;
using System.IO;
using System.Collections;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Xml;

public class MenuParser
{
    public ObservableCollection<string> ReadCollections(string filePath)
    {

        ObservableCollection<string> list = new ObservableCollection<string>();
        if (File.Exists(filePath))
        {
            try
            {
                XmlReader reader = XmlReader.Create(filePath);
                XmlDocument doc = new XmlDocument();
                reader.Read();
                doc.Load(reader);

                XmlNodeList items = doc.GetElementsByTagName("item");
                foreach (XmlNode item in items)
                {
                    XmlAttribute name = item.Attributes["collection"];

                    if(name != null)
                    {
                        list.Add(name.Value);
                    }
                }
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        return list;
    }

    /// <summary>
    /// Save settings back to ini file.
    /// </summary>
    public void Save(ObservableCollection<string> list, string filePath)
    {
        try
        {
            XmlDocument doc = new XmlDocument();
            XmlElement menu = doc.CreateElement("menu");

            doc.AppendChild(menu);

            foreach (string item in list)
            {
                XmlElement node = doc.CreateElement("item");
                XmlAttribute attrib = doc.CreateAttribute("collection");
                attrib.Value = item;
                menu.AppendChild(node);
                node.AppendChild(attrib);
            }

            doc.Save(filePath);
        }
        catch (Exception ex)
        {
            throw ex;
        }

        
//        SaveSettings(_FilePath);
    }
}