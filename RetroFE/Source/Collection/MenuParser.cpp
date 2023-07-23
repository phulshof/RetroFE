/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MenuParser.h"
#include "CollectionInfo.h"
#include "CollectionInfoBuilder.h"
#include "Item.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include "../Database/Configuration.h"
#include "../Database/DB.h"
#include <algorithm>
#include <rapidxml.hpp>
#include <fstream>
#include <sstream>
#include <dirent.h>


bool VectorSort(Item *d1, Item *d2)
{
    return d1->lowercaseTitle() < d2->lowercaseTitle();
}

MenuParser::MenuParser()
{
}

MenuParser::~MenuParser()
{
}

bool MenuParser::buildMenuItems(CollectionInfo *collection, bool sort)
{

    if(!buildTextMenu(collection, sort))
    {
        return buildLegacyXmlMenu(collection, sort);
    }

    return true;
} 

bool MenuParser::buildTextMenu(CollectionInfo *collection, bool sort)
{
    std::string file = Utils::combinePath(Configuration::absolutePath, "collections", collection->name, "menu.txt");
    std::ifstream includeStream(file.c_str());
    std::vector<Item *> menuItems;

    if (!includeStream.good())
    {

        Logger::write(Logger::ZONE_INFO, "Menu", "File does not exist: \"" + file + "\"; trying menu directory.");

        DIR *dp;
        struct dirent *dirp;
        std::string path = Utils::combinePath(Configuration::absolutePath, "collections", collection->name, "menu");
        dp = opendir(path.c_str());

        while(dp && (dirp = readdir(dp)) != NULL)
        {
            std::string file = dirp->d_name;

            size_t position = file.find_last_of(".");
            std::string basename = (std::string::npos == position)? file : file.substr(0, position);

            std::string comparator = ".txt";
            int start = file.length() - comparator.length();

            if(start >= 0)
            {
                if(file.compare(start, comparator.length(), comparator) == 0)
                {
                    std::string title = basename;
                    Item *item = new Item();
                    item->title = title;
                    item->fullTitle = title;
                    item->name = title;
                    item->leaf = false;
                    item->collectionInfo = collection;

                    menuItems.push_back(item);
                }
            }
        }

        if (dp) closedir(dp);
        std::sort(menuItems.begin(), menuItems.end(), [](Item *a, Item *b) {return Utils::toLower(a->fullTitle) <= Utils::toLower(b->fullTitle);});

    }
    else
    {

        Logger::write(Logger::ZONE_INFO, "Menu", "Found: \"" + file + "\"");

        std::string line;

        while(std::getline(includeStream, line))
        {
            line = Utils::filterComments(line);

            if(!line.empty())
            {
                std::string title = line;
                Item *item = new Item();
                item->title = title;
                item->fullTitle = title;
                item->name = title;
                item->leaf = false;
                item->collectionInfo = collection;

                menuItems.push_back(item);
            }
        }
    }

    collection->menusort = sort;
    collection->items.insert(collection->items.begin(), menuItems.begin(), menuItems.end());

    return true;
}

bool MenuParser::buildLegacyXmlMenu(CollectionInfo *collection, bool sort)
{
    bool retVal = false;
    //todo: magic string
    std::string menuFilename = Utils::combinePath(Configuration::absolutePath, "collections", collection->name, "menu.xml");
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> * rootNode;
    std::vector<Item *> menuItems;

    try
    {
        std::ifstream file(menuFilename.c_str());

        // gracefully exit if there is no menu file for the pa
        if(file.good())
        {
            Logger::write(Logger::ZONE_INFO, "Menu", "Found: \"" + menuFilename + "\"");
            Logger::write(Logger::ZONE_INFO, "Menu", "Using legacy menu.xml file. Consider using the new menu.txt format");
            std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            buffer.push_back('\0');

            doc.parse<0>(&buffer[0]);

            rootNode = doc.first_node("menu");

            for (rapidxml::xml_node<> * itemNode = rootNode->first_node("item"); itemNode; itemNode = itemNode->next_sibling())
            {
                rapidxml::xml_attribute<> *collectionAttribute = itemNode->first_attribute("collection");

                if(!collectionAttribute)
                {
                    retVal = false;
                    Logger::write(Logger::ZONE_ERROR, "Menu", "Menu item tag is missing collection attribute");
                    break;
                }
                //todo, check for empty string
                std::string title = collectionAttribute->value();
                Item *item = new Item();
                item->title = title;
                item->fullTitle = title;
                item->name = collectionAttribute->value();
                item->leaf = false;
                item->collectionInfo = collection;

                menuItems.push_back(item);
            }
        

            collection->menusort = sort;
            collection->items.insert(collection->items.begin(), menuItems.begin(), menuItems.end());

            retVal = true;
        }
    }
    catch(std::ifstream::failure &e)
    {
        std::stringstream ss;
        ss << "Unable to open menu file \"" << menuFilename << "\": " << e.what();
        Logger::write(Logger::ZONE_ERROR, "Menu", ss.str());
    }

    return retVal;
}
