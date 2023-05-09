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
#include "CollectionInfoBuilder.h"
#include "CollectionInfo.h"
#include "Item.h"
#include "../Database/Configuration.h"
#include "../Database/MetadataDatabase.h"
#include "../Database/DB.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include <dirent.h>

#if defined(__linux) || defined(__APPLE__)
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>
#endif

#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>

CollectionInfoBuilder::CollectionInfoBuilder(Configuration &c, MetadataDatabase &mdb)
    : conf_(c)
    , metaDB_(mdb)
{
}

CollectionInfoBuilder::~CollectionInfoBuilder()
{
}

bool CollectionInfoBuilder::createCollectionDirectory(std::string name)
{
    std::string collectionPath = Utils::combinePath(Configuration::absolutePath, "collections", name);

    std::vector<std::string> paths;
    paths.push_back(collectionPath);
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "artwork_back"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "artwork_front"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "bezel"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "logo"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "medium_back"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "medium_front"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "screenshot"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "screentitle"));
    paths.push_back(Utils::combinePath(collectionPath, "medium_artwork", "video"));
    paths.push_back(Utils::combinePath(collectionPath, "roms"));
    paths.push_back(Utils::combinePath(collectionPath, "system_artwork"));

    for(std::vector<std::string>::iterator it = paths.begin(); it != paths.end(); it++)
    {
        std::cout << "Creating folder \"" << *it << "\"" << std::endl;

#if defined(_WIN32) && !defined(__GNUC__)
        if (!CreateDirectory(it->c_str(), NULL))
        {
            if (ERROR_ALREADY_EXISTS != GetLastError())
            {
                std::cout << "Could not create folder \"" << *it << "\"" << std::endl;
                return false;
            }
        }
#else 
#if defined(__MINGW32__)
        if (mkdir(it->c_str()) == -1)
#else
        if (mkdir(it->c_str(), 0755) == -1)
#endif        
        {
           std::cout << "Could not create folder \"" << *it << "\":" << errno << std::endl;
        }
    #endif
    }

    std::string filename = Utils::combinePath(collectionPath, "include.txt");
    std::cout << "Creating file \"" << filename << "\"" << std::endl;

    std::ofstream includeFile;
    includeFile.open(filename.c_str());
    includeFile << "# Add a list of files to show on the menu (one filename per line, without the extension)." << std::endl;
    includeFile << "# If no items are in this list then all files in the folder specified" << std::endl;
    includeFile << "# by settings.conf will be used" << std::endl;
    includeFile.close();

    filename = Utils::combinePath(collectionPath, "exclude.txt");
    std::cout << "Creating file \"" << filename << "\"" << std::endl;
    std::ofstream excludeFile;
    excludeFile.open(filename.c_str());

    includeFile << "# Add a list of files to hide on the menu (one filename per line, without the extension)." << std::endl;
    excludeFile.close();

    filename = Utils::combinePath(collectionPath, "settings.conf");
    std::cout << "Creating file \"" << filename << "\"" << std::endl;
    std::ofstream settingsFile;
    settingsFile.open(filename.c_str());

    settingsFile << "# Uncomment and edit the following line to use a different ROM path." << std::endl;
    settingsFile << "#list.path = " << Utils::combinePath("%BASE_ITEM_PATH%", "%ITEM_COLLECTION_NAME%", "roms") << std::endl;
    settingsFile << "list.includeMissingItems = false" << std::endl;
    settingsFile << "list.extensions = zip" << std::endl;
    settingsFile << "list.menuSort = yes" << std::endl;
    settingsFile << std::endl;
    settingsFile << "launcher = mame" << std::endl;
    settingsFile << "#metadata.type = MAME" << std::endl;
    settingsFile << std::endl;
    settingsFile << std::endl;
    settingsFile << "#media.screenshot      = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "screenshot") << std::endl;
    settingsFile << "#media.screentitle     = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "screentitle") << std::endl;
    settingsFile << "#media.artwork_back    = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "artwork_back") << std::endl;
    settingsFile << "#media.artwork_front   = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "artwork_front") << std::endl;
    settingsFile << "#media.logo            = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "logo") << std::endl;
    settingsFile << "#media.medium_back     = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "medium_back") << std::endl;
    settingsFile << "#media.medium_front    = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "medium_front") << std::endl;
    settingsFile << "#media.screenshot      = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "screenshot") << std::endl;
    settingsFile << "#media.screentitle     = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "screentitle") << std::endl;
    settingsFile << "#media.video           = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "medium_artwork", "video") << std::endl;
    settingsFile << "#media.system_artwork  = " << Utils::combinePath("%BASE_MEDIA_PATH%", "%ITEM_COLLECTION_NAME%", "system_artwork") << std::endl;
    settingsFile.close();

    filename = Utils::combinePath(collectionPath, "menu.txt");
    std::cout << "Creating file \"" << filename << "\"" << std::endl;
    std::ofstream menuFile;
    menuFile.open(filename.c_str());
    menuFile.close();

    return true;
}
CollectionInfo *CollectionInfoBuilder::buildCollection(std::string name)
{
   return buildCollection(name, "");
}

CollectionInfo *CollectionInfoBuilder::buildCollection(std::string name, std::string mergedCollectionName)
{
    std::string listItemsPathKey = "collections." + name + ".list.path";
    std::string listFilterKey = "collections." + name + ".list.filter";
    std::string extensionsKey = "collections." + name + ".list.extensions";
    std::string launcherKey = "collections." + name + ".launcher";

    //todo: metadata is not fully not implemented
    std::string metadataTypeKey = "collections." + name + ".metadata.type";
    std::string metadataPathKey = "collections." + name + ".metadata.path";

    std::string listItemsPath;
    std::string launcherName;
    std::string extensions;
    std::string metadataType = name;
    std::string metadataPath;
    
    conf_.getCollectionAbsolutePath(name, listItemsPath);

    (void)conf_.getProperty(extensionsKey, extensions);
    (void)conf_.getProperty(metadataTypeKey, metadataType);
    (void)conf_.getProperty(metadataPathKey, metadataPath);

    if (!conf_.getProperty(launcherKey, launcherName))
    {
        std::stringstream ss;
        ss        << "\""
                  << launcherKey
                  << "\" points to a launcher that is not configured (launchers."
                  << launcherName
                  << "). Your collection will be viewable, however you will not be able to "
                  << "launch any of the items in your collection.";

        Logger::write(Logger::ZONE_NOTICE, "Collections", ss.str());
    }

    CollectionInfo *collection = new CollectionInfo(name, listItemsPath, extensions, metadataType, metadataPath);

    (void)conf_.getProperty("collections." + collection->name + ".launcher", collection->launcher);

    ImportDirectory(collection, mergedCollectionName);

    return collection;
}


bool CollectionInfoBuilder::ImportBasicList(CollectionInfo *info, std::string file, std::map<std::string, Item *> &list)
{
    std::ifstream includeStream(file.c_str());

    if (!includeStream.good())
    {
        return false;
    }

    std::string line; 

    while(std::getline(includeStream, line))
    {
        line = Utils::filterComments(line);
        
        if (!line.empty() && list.find(line) == list.end())
        {
            Item *i = new Item();

            line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );

            i->fullTitle = line;
            i->name = line;
            i->title = line;
            i->collectionInfo = info;

            list[line] = i;
        }
    }

    return true;
}

bool CollectionInfoBuilder::ImportBasicList(CollectionInfo *info, std::string file, std::vector<Item *> &list)
{
    std::ifstream includeStream(file.c_str());

    if (!includeStream.good())
    {
        return false;
    }

    std::string line; 

    while(std::getline(includeStream, line))
    {
        line = Utils::filterComments(line);
        
        if (!line.empty())
        {

            bool found = false;
            for (std::vector<Item *>::iterator it = list.begin(); it != list.end(); ++it)
            {
                if (line == (*it)->name)
                {
                    found = true;
                }
            }

            if (!found)
            {
                Item *i = new Item();

                line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );

                i->fullTitle = line;
                i->name = line;
                i->title = line;
                i->collectionInfo = info;

                list.push_back(i);
            }

        }
    }

    return true;
}

bool CollectionInfoBuilder::ImportDirectory(CollectionInfo *info, std::string mergedCollectionName)
{
    std::string path = info->listpath;
    std::vector<Item *> includeFilterUnsorted;
    std::map<std::string, Item *> includeFilter;
    std::map<std::string, Item *> excludeFilter;
    std::string includeFile    = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "include.txt");
    std::string excludeFile    = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "exclude.txt");

    std::string launcher;
    bool showMissing  = false; 
    bool romHierarchy = false;
    bool emuarc       = false;
 
    if (mergedCollectionName != "")
    {
        
        std::string mergedFile = Utils::combinePath(Configuration::absolutePath, "collections", mergedCollectionName, info->name + ".sub");
        Logger::write(Logger::ZONE_INFO, "CollectionInfoBuilder", "Checking for \"" + mergedFile + "\"");
        (void)conf_.getProperty("collections." + mergedCollectionName + ".list.includeMissingItems", showMissing);
        ImportBasicList(info, mergedFile, includeFilterUnsorted);
        ImportBasicList(info, mergedFile, includeFilter);

    }
    (void)conf_.getProperty("collections." + info->name + ".list.includeMissingItems", showMissing);
    (void)conf_.getProperty("collections." + info->name + ".list.romHierarchy", romHierarchy);
    (void)conf_.getProperty("collections." + info->name + ".list.emuarc", emuarc);
    if (emuarc)
        romHierarchy = true;

    Logger::write(Logger::ZONE_INFO, "CollectionInfoBuilder", "Checking for \"" + includeFile + "\"");
    ImportBasicList(info, includeFile, includeFilterUnsorted);
    ImportBasicList(info, includeFile, includeFilter);
    ImportBasicList(info, excludeFile, excludeFilter);

    for(std::vector<Item *>::iterator it = includeFilterUnsorted.begin(); it != includeFilterUnsorted.end(); ++it)
    {
        if (showMissing && excludeFilter.find((*it)->name) == excludeFilter.end())
        {
            info->items.push_back(*it);
        }
        else
        {
            delete *it;
        }
    }
    includeFilterUnsorted.clear( );

    // Read ROM directory if showMissing is false
    if (!showMissing || includeFilter.size() == 0)
    {
        do
        {
             std::string rompath;
             size_t position = path.find( ";" );
             if(position != std::string::npos)
             {
                 rompath = path.substr(0, position);
                 path    = path.substr(position+1);
             }
             else
             {
                 rompath = path;
                 path    = "";
             }
             ImportRomDirectory(rompath, info, includeFilter, excludeFilter, romHierarchy, emuarc);
        } while (path != "");
    }

    while(includeFilter.size() > 0)
    {
        std::map<std::string, Item *>::iterator it = includeFilter.begin();
        // delete the unused items if they were never pushed to the main collection
        if (!showMissing)
        {
            delete it->second;
        }
        includeFilter.erase(it);
    }
    while(excludeFilter.size() > 0)
    {
        std::map<std::string, Item *>::iterator it = excludeFilter.begin();
        delete it->second;
        excludeFilter.erase(it);
    }

    return true;
}


void CollectionInfoBuilder::addPlaylists(CollectionInfo *info)
{
    std::map<std::string, Item *> excludeAllFilter;
    std::string excludeAllFile = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "exclude_all.txt");

    ImportBasicList(info, excludeAllFile, excludeAllFilter);

    if ( excludeAllFilter.size() > 0)
    {
        info->playlists["all"] = new std::vector<Item *>();
        for(std::vector<Item *>::iterator it = info->items.begin(); it != info->items.end(); it++)
        {
            bool found = false;
            for(std::map<std::string, Item *>::iterator itex = excludeAllFilter.begin(); itex != excludeAllFilter.end(); itex++)
            {
                std::string collectionName = info->name;
                std::string itemName       = itex->first;
                if (itemName.at(0) == '_') // name consists of _<collectionName>:<itemName>
                {
                     itemName.erase(0, 1); // Remove _
                     size_t position = itemName.find(":");
                     if (position != std::string::npos )
                     {
                         collectionName = itemName.substr(0, position);
                         itemName       = itemName.erase(0, position+1);
                     }
                }
                if ( ((*it)->name == itemName || itemName == "*") && (*it)->collectionInfo->name == collectionName )
                {
                    found = true;
                }
            }
            if ( !found )
            {
                info->playlists["all"]->push_back((*it));
            }
        }
        while(excludeAllFilter.size() > 0)
        {
            std::map<std::string, Item *>::iterator it = excludeAllFilter.begin();
            excludeAllFilter.erase(it);
        }
    }
    else
    {
        info->playlists["all"] = &info->items;
    }

    DIR *dp;
    struct dirent *dirp;
    std::string path = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "playlists");
    dp = opendir(path.c_str());

    if(dp == NULL)
    {
        info->playlists["favorites"] = new std::vector<Item *>();
        return;
    }
    std::map<std::string, Item*> playlistItems;

    while((dirp = readdir(dp)) != NULL)
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
                Logger::write(Logger::ZONE_INFO, "RetroFE", "Loading playlist: " + basename);

                std::map<std::string, Item *> playlistFilter;
                std::string playlistFile = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "playlists", file);
                ImportBasicList(info, playlistFile, playlistFilter);

                info->playlists[basename] = new std::vector<Item *>();

                Item* playlistItem = new Item();
                playlistItem->name = basename;
                playlistItem->title = basename;
                playlistItem->fullTitle = basename;
                playlistItem->leaf = false;
                playlistItem->collectionInfo = info;
                playlistItems[basename] = playlistItem;

                // add the playlist list 
                for(std::map<std::string, Item *>::iterator it = playlistFilter.begin(); it != playlistFilter.end(); it++)
                {
                    std::string collectionName = info->name;
                    std::string itemName       = it->first;
                    if (itemName.at(0) == '_') // name consists of _<collectionName>:<itemName>
                    {
                         itemName.erase(0, 1); // Remove _
                         size_t position = itemName.find(":");
                         if (position != std::string::npos )
                         {
                             collectionName = itemName.substr(0, position);
                             itemName       = itemName.erase(0, position+1);
                         }
                    }

                    for(std::vector<Item *>::iterator it = info->items.begin(); it != info->items.end(); it++)
                    {
                        if ( ((*it)->name == itemName || itemName == "*") && (*it)->collectionInfo->name == collectionName )
                        {
                            info->playlists[basename]->push_back((*it));
                            if ( basename == "favorites" )
                                (*it)->isFavorite = true;
                        }
                    }
                }
                while ( playlistFilter.size( ) > 0 )
                {
                    std::map<std::string, Item *>::iterator it = playlistFilter.begin();
                    delete it->second;
                    playlistFilter.erase( it->first );
                }
            }
        }
    }
    // if cyclePlaylist then order playlist menu items by that
    std::string cycleString;
    conf_.getProperty("cyclePlaylist", cycleString);
    std::vector<std::string> cycleVector;
    Utils::listToVector(cycleString, cycleVector, ',');
    if (cycleVector.size())
    {
        // add in order according to cycle list
        for (std::vector<std::string>::iterator itP = cycleVector.begin(); itP != cycleVector.end(); itP++)
        {
            if (playlistItems[*itP]) {
                info->playlistItems.push_back(playlistItems[*itP]);
            }
        }
    }
    else {
        // convert lookup playlist map to vector
        for (std::map<std::string, Item*>::iterator itP = playlistItems.begin(); itP != playlistItems.end(); itP++)
        {
            info->playlistItems.push_back(itP->second);
        }
    }

    if (dp) closedir(dp);

    if(info->playlists["favorites"] == NULL)
    {
        info->playlists["favorites"] = new std::vector<Item *>();
    }

    if(info->playlists["lastplayed"] == NULL)
    {
        info->playlists["lastplayed"] = new std::vector<Item *>();
    }

    return;
}


void CollectionInfoBuilder::updateLastPlayedPlaylist(CollectionInfo *info, Item *item, int size)
{
    std::string path = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "playlists");
    Logger::write(Logger::ZONE_INFO, "RetroFE", "Updating lastplayed playlist");

    std::vector<Item *> lastplayedList;
    std::string playlistFile = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "playlists", "lastplayed.txt");
    ImportBasicList(info, playlistFile, lastplayedList);

    if (info->playlists["lastplayed"] == NULL)
        info->playlists["lastplayed"] = new std::vector<Item *>();
    else
        info->playlists["lastplayed"]->clear();

    if (size == 0)
        return;

    // Put the new item at the front of the list.
    info->playlists["lastplayed"]->push_back(item);

    // Add the items already in the playlist up to the lastplayedSize.
    for(std::vector<Item *>::iterator it = lastplayedList.begin(); it != lastplayedList.end(); it++)
    {
        if (info->playlists["lastplayed"]->size() >= static_cast<unsigned int>( size ))
            break;

        std::string collectionName = info->name;
        std::string itemName       = (*it)->name;
        if (itemName.at(0) == '_') // name consists of _<collectionName>:<itemName>
        {
             itemName.erase(0, 1); // Remove _
             size_t position = itemName.find(":");
             if (position != std::string::npos )
             {
                 collectionName = itemName.substr(0, position);
                 itemName       = itemName.erase(0, position+1);
             }
        }

        for(std::vector<Item *>::iterator it = info->items.begin(); it != info->items.end(); it++)
        {
            if ( (*it)->name == itemName && (*it)->collectionInfo->name == collectionName && (*it) != item)
            {
                info->playlists["lastplayed"]->push_back((*it));
            }
        }
    }

    // Write new lastplayed playlist
    std::string dir  = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "playlists");
    std::string file = Utils::combinePath(Configuration::absolutePath, "collections", info->name, "playlists/lastplayed.txt");
    Logger::write(Logger::ZONE_INFO, "Collection", "Saving " + file);

    std::ofstream filestream;
    try
    {
        // Create playlists directory if it does not exist yet.
        struct stat infostat;
        if ( stat( dir.c_str(), &infostat ) != 0 )
        {
#if defined(_WIN32) && !defined(__GNUC__)
            if(!CreateDirectory(dir.c_str(), NULL))
            {
                if(ERROR_ALREADY_EXISTS != GetLastError())
                {
                    Logger::write(Logger::ZONE_WARNING, "Collection", "Could not create directory " + dir);
                    return;
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
                return;
            }
#endif
        }
        else if ( !(infostat.st_mode & S_IFDIR) )
        {
            Logger::write(Logger::ZONE_WARNING, "Collection", dir + " exists, but is not a directory.");
            return;
        }

        filestream.open(file.c_str());
        std::vector<Item *> *saveitems = info->playlists["lastplayed"];
        for(std::vector<Item *>::iterator it = saveitems->begin(); it != saveitems->end(); it++)
        {
            if ((*it)->collectionInfo->name == info->name)
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
    }

    // Sort the playlist(s)
    info->sortPlaylists( );

    return;

}


void CollectionInfoBuilder::ImportRomDirectory(std::string path, CollectionInfo *info, std::map<std::string, Item *> includeFilter, std::map<std::string, Item *> excludeFilter, bool romHierarchy, bool emuarc)
{

    DIR                               *dp;
    struct dirent                     *dirp;
    std::vector<std::string>           extensions;
    std::vector<std::string>::iterator extensionsIt;

    info->extensionList(extensions);

    dp = opendir(path.c_str());

    Logger::write(Logger::ZONE_INFO, "CollectionInfoBuilder", "Scanning directory \"" + path + "\"");
    if (dp == NULL)
    {
        Logger::write(Logger::ZONE_INFO, "CollectionInfoBuilder", "Could not read directory \"" + path + "\". Ignore if this is a menu.");
        return;
    }

    while(dp != NULL && (dirp = readdir(dp)) != NULL)
    {
        std::string file = dirp->d_name;

        // Check if the file is a directory or a file
        struct stat sb;
        if (romHierarchy && file != "." && file != ".." && stat( Utils::combinePath( path, file ).c_str(), &sb ) == 0 && S_ISDIR( sb.st_mode ))
        {
            ImportRomDirectory( Utils::combinePath( path, file ), info, includeFilter, excludeFilter, romHierarchy, emuarc );
        }
        else if (file != "." && file != "..")
        {
            size_t position = file.find_last_of(".");
            std::string basename = (std::string::npos == position)? file : file.substr(0, position);
        
            // if there is an include list, only include roms that are found and are in the include list
            // if there is an exclude list, exclude those roms
            if ((includeFilter.size() == 0 || (includeFilter.find(basename) != includeFilter.end())) &&
                    (excludeFilter.size() == 0 || excludeFilter.find(basename) == excludeFilter.end()))
            {
                // iterate through all known file extensions
                for(extensionsIt = extensions.begin(); extensionsIt != extensions.end(); ++extensionsIt)
                {
                    std::string comparator = "." + *extensionsIt;
                    int start = file.length() - comparator.length() + 1;

                    if (start >= 0)
                    {
                        if (file.compare(start, comparator.length(), *extensionsIt) == 0)
                        {
                            Item *i = new Item();

                            i->name           = basename;
                            i->fullTitle      = basename;
                            i->title          = basename;
                            i->collectionInfo = info;
                            i->filepath       = path + Utils::pathSeparator;

                            if ( emuarc )
                            {
                                i->file      = basename;
                                i->name      = Utils::getFileName( path );
                                i->fullTitle = i->name;
                                i->title     = i->name;
                            }

                            // Add item if it doesn't already exist
                            bool found = false;
                            for ( std::vector<Item*>::iterator it = info->items.begin(); it != info->items.end( ); it++ )
                               if ( (*it)->name == basename )
                                   found = true;
                            if ( !found )
                                info->items.push_back(i);
                        }
                    }
                }
            }
        }
    }

    if (dp) closedir(dp);

    return;

}


void CollectionInfoBuilder::injectMetadata(CollectionInfo *info)
{
    metaDB_.injectMetadata(info);
    return;
}
