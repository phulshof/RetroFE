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

#include "../Database/MetadataDatabase.h"
#include <string>
#include <map>
#include <vector>

class Configuration;
class CollectionInfo;


class CollectionInfoBuilder
{
public:
    CollectionInfoBuilder(Configuration &c, MetadataDatabase &mdb);
    virtual ~CollectionInfoBuilder();
    CollectionInfo *buildCollection(std::string collectionName);
    CollectionInfo *buildCollection(std::string collectionName, std::string mergedCollectionName);
    void addPlaylists(CollectionInfo *info);
    void updateLastPlayedPlaylist(CollectionInfo *info, Item *item, int size);
    void injectMetadata(CollectionInfo *info);
    static bool createCollectionDirectory(std::string collectionName);
    bool ImportBasicList(CollectionInfo *info, std::string file, std::vector<Item *> &list);

private:
    Configuration &conf_;
    MetadataDatabase &metaDB_;
    bool ImportBasicList(CollectionInfo *info, std::string file, std::map<std::string, Item *> &list);
    bool ImportDirectory(CollectionInfo *info, std::string mergedCollectionName);
    void ImportRomDirectory(std::string path, CollectionInfo *info, std::map<std::string, Item *> includeFilter, std::map<std::string, Item *> excludeFilter, bool romHierarchy, bool emuarc);
};
