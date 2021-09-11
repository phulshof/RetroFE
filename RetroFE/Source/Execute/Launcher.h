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

class Configuration;
class Item;
class RetroFE;

class Launcher
{
public:
    Launcher(Configuration &c);
    bool run(std::string collection, Item *collectionItem);
	void LEDBlinky( int command, std::string collection = "", Item *collectionItem = NULL);

private:
    std::string replaceString(
        std::string subject,
        const std::string &search,
        const std::string &replace);

    bool launcherName(std::string &launcherName, std::string collection);
    bool launcherExecutable(std::string &executable, std::string launcherName);
    bool launcherArgs(std::string &args, std::string launcherName);
    bool extensions(std::string &extensions, std::string launcherName);
    bool collectionDirectory(std::string &directory, std::string collection);
    bool execute(std::string executable, std::string arguments, std::string currentDirectory, bool wait = true);
    bool findFile(std::string &foundFilePath, std::string &foundFilename, std::string directory, std::string filenameWithoutExtension, std::string extensions);
    std::string replaceVariables(std::string str,
                                 std::string itemFilePath,
                                 std::string itemName,
                                 std::string itemFilename,
                                 std::string itemDirectory,
                                 std::string itemCollectionName);

    Configuration &config_;
};
