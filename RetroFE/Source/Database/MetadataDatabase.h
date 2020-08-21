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
#include <vector>
#include <map>

class DB;
class Configuration;
class CollectionInfo;
class Item;

class MetadataDatabase
{
public:
    MetadataDatabase(DB &db, Configuration &c);
    virtual ~MetadataDatabase();
    bool initialize();
    bool resetDatabase();

    void injectMetadata(CollectionInfo *collection);
    bool importHyperlist(std::string hyperlistFile, std::string collectionName);
    bool importMamelist(std::string filename, std::string collectionName);
    bool importEmuArclist(std::string filename);

private:
    bool importDirectory();
    bool needsRefresh();
    time_t timeDir(std::string path);
    Configuration &config_;
    DB &db_;
};
