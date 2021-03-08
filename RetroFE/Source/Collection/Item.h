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
#pragma once

#include <string>
#include <map>
#include "CollectionInfo.h"

class Item
{
public:
    Item();
    virtual ~Item();
    std::string filename();
    std::string lowercaseTitle() ;
    std::string lowercaseFullTitle();
    std::string name;
    std::string filepath;
    std::string file;
    std::string title;
    std::string fullTitle;
    std::string year;
    std::string manufacturer;
    std::string developer;
    std::string genre;
    std::string cloneof;
    std::string numberPlayers;
    std::string numberButtons;
    std::string ctrlType;
    std::string joyWays;
    std::string rating;
    std::string score;
    bool        isFavorite;
    CollectionInfo *collectionInfo;
    bool leaf;

    typedef std::map<std::string, std::string> InfoType;
    typedef std::pair<std::string, std::string> InfoPair;
    InfoType info_;
    void setInfo( std::string key, std::string value );
    bool getInfo( std::string key, std::string &value );
    void loadInfo( std::string path );
};
