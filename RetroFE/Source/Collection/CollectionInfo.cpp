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

CollectionInfo::CollectionInfo(std::string name,
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

bool CollectionInfo::Save() 
{
    bool retval = true;
    if(saveRequest)
    {
        std::string dir  = Utils::combinePath(Configuration::absolutePath, "collections", name, "playlists");
        std::string file = Utils::combinePath(Configuration::absolutePath, "collections", name, "playlists/favorites.txt");
        Logger::write(Logger::ZONE_INFO, "Collection", "Saving " + file);

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

            filestream.open(file.c_str());
            std::vector<Item *> *saveitems = playlists["favorites"];
            for(std::vector<Item *>::iterator it = saveitems->begin(); it != saveitems->end(); it++)
            {
                if ((*it)->collectionInfo->name == name)
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
            Logger::write(Logger::ZONE_ERROR, "Collection", "Save failed: " + file);
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

bool CollectionInfo::itemIsLess(Item *lhs, Item *rhs)
{
    if(lhs->leaf && !rhs->leaf) return true;
    if(!lhs->leaf && rhs->leaf) return false;
    if(lhs->collectionInfo->subsSplit && lhs->collectionInfo != rhs->collectionInfo)
        return lhs->collectionInfo->lowercaseName() < rhs->collectionInfo->lowercaseName();
    if(!lhs->collectionInfo->menusort && !lhs->leaf && !rhs->leaf)
        return false;
    return lhs->lowercaseFullTitle() < rhs->lowercaseFullTitle();
}


void CollectionInfo::sortItems()
{
    std::sort( items.begin(), items.end(), itemIsLess );
}


void CollectionInfo::sortPlaylists()
{
    std::vector<Item *> *allItems = &items;
    std::vector<Item *> toSortItems;

    for ( Playlists_T::iterator itP = playlists.begin( ); itP != playlists.end( ); itP++ )
    {
        if ( itP->second != allItems )
        {
            toSortItems.clear();
            for(std::vector <Item *>::iterator itSort = itP->second->begin(); itSort != itP->second->end(); itSort++)
            {
                toSortItems.push_back((*itSort));
            }
            itP->second->clear();
            for(std::vector <Item *>::iterator itAll = allItems->begin(); itAll != allItems->end(); itAll++)
            {
                for(std::vector <Item *>::iterator itSort = toSortItems.begin(); itSort != toSortItems.end(); itSort++)
                {
                    if ((*itAll) == (*itSort))
                    {
                        itP->second->push_back((*itAll));
                    }
                }
            }
        }
    }
}
