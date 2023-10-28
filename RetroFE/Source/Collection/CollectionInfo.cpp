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
#include "CollectionInfo.h"
#include "Item.h"
#include "../Database/Configuration.h"
#include "../Utility/Utils.h"
#include "../Utility/Log.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <exception>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#if defined(__linux) || defined(__APPLE__)
#include <errno.h>
#include <cstring>
#endif

CollectionInfo::CollectionInfo(
    Configuration& c,
    std::string name,
    std::string listPath,
    std::string extensions,
    std::string metadataType,
    std::string metadataPath)
    : name(name)
    , listpath(listPath)
    , saveRequest(false)
    , metadataType(metadataType)
    , menusort(true)
    , subsSplit(false)
    , hasSubs(false)
    , sortDesc(false)
    , conf_(c)  // Initialize conf_ last
    , metadataPath_(metadataPath)
    , extensions_(extensions)
{
}

CollectionInfo::~CollectionInfo()
{
    Playlists_T::iterator pit = playlists.begin();

    while(pit != playlists.end())
    {
        if(pit->second != &items)
        {
            delete pit->second;
        }
        playlists.erase(pit);
        pit = playlists.begin();
    }

	std::vector<Item *>::iterator it = items.begin();
    while(it != items.end())
    {
        delete *it;
        items.erase(it);
        it = items.begin();
    }
}

bool CollectionInfo::saveFavorites(Item* removed)
{
    bool retval = true;
    if(saveRequest && name != "")
    {
        std::string playlistCollectionName = name;
        bool globalFavLast = false;
        (void)conf_.getProperty("globalFavLast", globalFavLast);
        if (globalFavLast) {
            playlistCollectionName = "Favorites";
        }
        std::string dir  = Utils::combinePath(Configuration::absolutePath, "collections", playlistCollectionName, "playlists");
        std::string file = Utils::combinePath(Configuration::absolutePath, "collections", playlistCollectionName, "playlists", "favorites.txt");
        Logger::write(Logger::ZONE_INFO, "Collection", "Saving favorites " + file);

        std::ofstream filestream;
        try
        {
            // Create playlists directory if it does not exist yet.
            struct stat info;
            if ( stat( dir.c_str(), &info ) != 0 )
            {
#if defined(_WIN32) && !defined(__GNUC__)
                if(!CreateDirectory(dir.c_str(), NULL))
                {
                    if(ERROR_ALREADY_EXISTS != GetLastError())
                    {
                        Logger::write(Logger::ZONE_WARNING, "Collection", "Could not create directory " + dir);
                        return false;
                    }
                }
#else 
#if defined(__MINGW32__)
                if(mkdir(dir.c_str()) == -1)
#else
                if(mkdir(dir.c_str(), 0755) == -1)
#endif        
                {
                    Logger::write(Logger::ZONE_WARNING, "Collection", "Could not create directory " + dir);
                    return false;
                }
#endif
            }
            else if ( !(info.st_mode & S_IFDIR) )
            {
                Logger::write(Logger::ZONE_WARNING, "Collection", dir + " exists, but is not a directory.");
                return false;
            }
            std::vector<Item*>* saveitems = playlists["favorites"];
            if (globalFavLast && name != "Favorites") {
                std::map <std::string, std::map <std::string, std::string>> existing;

                // remove from favorites file
                if (removed != NULL) {
                    std::vector <std::string> remaining;
                    std::ifstream includeStream(file.c_str());
                    std::string line;
                    std::string collectionName;
                    std::string itemName;
                    while (std::getline(includeStream, line)) {
                        line = Utils::filterComments(line);
                        if (!line.empty() && line != "_" + removed->collectionInfo->name + ":" + removed->name) {
                            remaining.push_back(line);
                        }
                    }
                    filestream.open(file.c_str());
                    std::vector<std::string>::iterator it = remaining.begin();
                    while (it != remaining.end())
                    {
                        filestream << (*it) << std::endl;
                        it++;
                    }
                    filestream.close();

                    return true;
                }

                // add to favorites file
                // make lookup of existing by collection to only add unique entries
                // and check if a game has been removed for that collection
                std::ifstream includeStream(file.c_str());
                std::string line;
                std::string collectionName;
                std::string itemName;
                while (std::getline(includeStream, line)) {
                    line = Utils::filterComments(line);
                    if (!line.empty()){
                        if (line.at(0) == '_') { // name consists of _<collectionName>:<itemName>
                            line.erase(0, 1); // Remove _
                            size_t position = line.find(":");
                            if (position != std::string::npos) {
                                collectionName = line.substr(0, position);
                                itemName = line.erase(0, position + 1);
                            }
                        }
                        existing[collectionName][itemName] = itemName;
                    }
                }
                std::vector<Item*>::iterator it = saveitems->begin();
                while (it != saveitems->end())
                {
                    if (existing.find((*it)->collectionInfo->name) != existing.end() &&
                        existing[(*it)->collectionInfo->name].find((*it)->name) != existing[(*it)->collectionInfo->name].end()) {
                        // exists so don't add to file
                        it = saveitems->erase(it);
                    }
                    else {
                        it++;
                    }                    
                }
            
                filestream.open(file.c_str(), std::ios_base::app);
            }
            else {
                filestream.open(file.c_str());
            }
            
            for(std::vector<Item *>::iterator it = saveitems->begin(); it != saveitems->end(); it++)
            {
                if ((*it)->collectionInfo->name == (*it)->name)
                {
                    filestream << (*it)->name << std::endl;
                }
                else
                {
                    filestream << "_" << (*it)->collectionInfo->name << ":" << (*it)->name << std::endl;
                }
            }

            filestream.close();
        }
        catch(std::exception &)
        {
            Logger::write(Logger::ZONE_ERROR, "Collection", "Save favorites failed: " + file);
            retval = false;
        }
    }
    
    return retval;
}


std::string CollectionInfo::settingsPath() const
{
    return Utils::combinePath(Configuration::absolutePath, "collections", name);
}


void CollectionInfo::extensionList(std::vector<std::string> &extensionlist)
{
    std::istringstream ss(extensions_);
    std::string token;

    while(std::getline(ss, token, ','))
    {
        token = Utils::trimEnds(token);
    	extensionlist.push_back(token);
    }
}

std::string CollectionInfo::lowercaseName()
{
    std::string lcstr = name;
    std::transform(lcstr.begin(), lcstr.end(), lcstr.begin(), ::tolower);
    return lcstr;
}

void CollectionInfo::addSubcollection(CollectionInfo *newinfo)
{
    items.insert(items.begin(), newinfo->items.begin(), newinfo->items.end());
}

auto CollectionInfo::itemIsLess(std::string sortType, bool currentCollectionMenusort)
{
    return [sortType, currentCollectionMenusort](Item* lhs, Item* rhs) {

        if (lhs->leaf && !rhs->leaf) return true;
        if (!lhs->leaf && rhs->leaf) return false;

        // sort by collections first
        if (lhs->collectionInfo->subsSplit && lhs->collectionInfo != rhs->collectionInfo)
            return lhs->collectionInfo->lowercaseName() < rhs->collectionInfo->lowercaseName();
        
        // no elements left
        if (!lhs->leaf && !rhs->leaf)
            return false;

        // sort by another attribute
        if (sortType != "") {
            std::string lhsValue = lhs->getMetaAttribute(sortType);
            std::string rhsValue = rhs->getMetaAttribute(sortType);
            bool desc = Item::isSortDesc(sortType);

            if (lhsValue != rhsValue) {
                return desc ? lhsValue > rhsValue : lhsValue < rhsValue;
            }
        }

        // menu sort is false then use playlist's order
        if (!currentCollectionMenusort)
            return false;

        // default sort by name
        return lhs->lowercaseFullTitle() < rhs->lowercaseFullTitle();
    };
}


void CollectionInfo::sortItems()
{
    std::sort(items.begin(), items.end(), itemIsLess("", menusort));
}


void CollectionInfo::sortPlaylists()
{
    std::vector<Item *> *allItems = &items;
    std::vector<Item *> toSortItems;

    for ( Playlists_T::iterator itP = playlists.begin( ); itP != playlists.end( ); itP++ )
    {
        if ( itP->second != allItems )
        {
            // temporarily set collection info's sortType so search has access to it
            sortType = Item::validSortType(itP->first) ? itP->first : "";
            std::sort(itP->second->begin(), itP->second->end(), itemIsLess(sortType, menusort));
        }
    }
    sortType = "";
}
