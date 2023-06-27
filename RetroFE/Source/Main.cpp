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

#include "Database/Configuration.h"
#include "Collection/CollectionInfoBuilder.h"
#include "Execute/Launcher.h"
#include "Utility/Log.h"
#include "Utility/Utils.h"
#include "RetroFE.h"
#include "Version.h"
#include "SDL.h"
#include <cstdlib>
#include <fstream>
#include <dirent.h>
#include <time.h>
#include <locale>

static bool ImportConfiguration(Configuration* c);
static bool StartLogging(Configuration* c);

int main(int argc, char** argv)
{
    // check to see if version or help was requested
    if (argc > 1)
    {
        std::string program = argv[0];
        std::string param = argv[1];

        if (argc == 3 && param == "-createcollection")
        {
            // Do nothing; we handle that later
        }
        else if (param == "-version" ||
            param == "--version" ||
            param == "-v")
        {
            std::cout << "RetroFE version " << Version::getString() << std::endl;
            return 0;
        }
        else
        {
            std::cout << "Usage:" << std::endl;
            std::cout << program << "                                           Run RetroFE" << std::endl;
            std::cout << program << " --version                                 Print the version of RetroFE." << std::endl;
            std::cout << program << " -createcollection <collection name>       Create a collection directory structure." << std::endl;
            return 0;
        }
    }

    // Initialize locale language
    setlocale(LC_ALL, "");

    // Initialize random seed
    srand(static_cast<unsigned int>(time(0)));

    Configuration::initialize();

    Configuration config;

    if (!StartLogging(&config))
    {
        return -1;
    }

    // check to see if createcollection was requested
    if (argc == 3)
    {
        std::string param = argv[1];
        std::string value = argv[2];

        if (param == "-createcollection")
        {
            CollectionInfoBuilder::createCollectionDirectory(value);
        }

        return 0;
    }
    try {

        while (true)
        {
            if (!ImportConfiguration(&config))
            {
                // Exit with a heads up...
                std::string logFile = Utils::combinePath(Configuration::absolutePath, "log.txt");
                fprintf(stderr, "RetroFE has failed to start due to configuration error.\nCheck log for details: %s\n", logFile.c_str());
                return -1;
            }
            RetroFE p(config);
            if (p.run()) // Check if we need to reboot after running
                config.clearProperties();
            else
                break;
        }
    }
    catch (std::exception& e)
    {
        Logger::write(Logger::ZONE_ERROR, "EXCEPTION", e.what());
    }

    Logger::deInitialize();

    return 0;
}

bool ImportConfiguration(Configuration* c)
{
    std::string configPath = Configuration::absolutePath;
#ifdef WIN32
    std::string launchersPath = Utils::combinePath(Configuration::absolutePath, "launchers.windows");
#elif __APPLE__
    std::string launchersPath = Utils::combinePath(Configuration::absolutePath, "launchers.apple");
#else
    std::string launchersPath = Utils::combinePath(Configuration::absolutePath, "launchers.linux");
#endif
    std::string collectionsPath = Utils::combinePath(Configuration::absolutePath, "collections");
    DIR* dp;
    struct dirent* dirp;

    std::string settingsConfPath = Utils::combinePath(configPath, "settings");
    if (!c->import("", settingsConfPath + ".conf"))
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + settingsConfPath + ".conf\"");
        return false;
    }
    for (int i = 1; i < 15; i++)
        c->import("", "", settingsConfPath + std::to_string(i) + ".conf", false);
    c->import("", "", settingsConfPath + "_saved.conf", false);

    // log version
    Logger::write(Logger::ZONE_INFO, "RetroFE", "Version " + Version::getString() + " starting");

#ifdef WIN32
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Windows");
#elif __APPLE__
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Mac");
#else
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Linux");
#endif

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Absolute path: " + Configuration::absolutePath);

    dp = opendir(launchersPath.c_str());

    if (dp == NULL)
    {
        Logger::write(Logger::ZONE_INFO, "RetroFE", "Could not read directory \"" + launchersPath + "\"");
        launchersPath = Utils::combinePath(Configuration::absolutePath, "launchers");
        dp = opendir(launchersPath.c_str());
        if (dp == NULL)
        {
            Logger::write(Logger::ZONE_NOTICE, "RetroFE", "Could not read directory \"" + launchersPath + "\"");
            return false;
        }
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        if (dirp->d_type != DT_DIR && std::string(dirp->d_name) != "." && std::string(dirp->d_name) != "..")
        {
            std::string basename = dirp->d_name;
            std::string::size_type dot_position = basename.find_last_of(".");

            if (dot_position == std::string::npos)
            {
                Logger::write(Logger::ZONE_NOTICE, "RetroFE", "Extension missing on launcher file \"" + basename + "\"");
                continue;
            }

            std::string extension = Utils::toLower(basename.substr(dot_position, basename.size() - 1));
            basename = basename.substr(0, dot_position);

            if (extension == ".conf")
            {
                std::string prefix = "launchers." + Utils::toLower(basename);

                std::string importFile = Utils::combinePath(launchersPath, std::string(dirp->d_name));

                if (!c->import(prefix, importFile))
                {
                    Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + importFile + "\"");
                    if (dp) closedir(dp);
                    return false;
                }
            }
        }
    }

    if (dp) closedir(dp);

    dp = opendir(collectionsPath.c_str());

    if (dp == NULL)
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not read directory \"" + collectionsPath + "\"");
        return false;
    }

    bool settingsImported;
    while ((dirp = readdir(dp)) != NULL)
    {
        std::string collection = (dirp->d_name);
        if (dirp->d_type == DT_DIR && collection != "." && collection != ".." && collection.length() > 0 && collection[0] != '_')
        {
            std::string prefix = "collections." + collection;

            settingsImported = false;
            std::string settingsPath = Utils::combinePath(collectionsPath, collection, "settings");
            settingsImported |= c->import(collection, prefix, settingsPath + ".conf", false);
            for (int i = 1; i < 15; i++)
                settingsImported |= c->import(collection, prefix, settingsPath + std::to_string(i) + ".conf", false);

            std::string infoFile = Utils::combinePath(collectionsPath, collection, "info.conf");
            c->import(collection, prefix, infoFile, false);

            if (!settingsImported)
            {
                Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not import any collection settings for " + collection);
            }
        }
    }

    if (dp) closedir(dp);

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Imported configuration");

    return true;
}

bool StartLogging(Configuration* config)
{
    std::string logFile = Utils::combinePath(Configuration::absolutePath, "log.txt");

    if (!Logger::initialize(logFile, config))
    {
        // Can't write to logs give a heads up...
        fprintf(stderr, "Could not open log: %s for writing!\nRetroFE will now exit...\n", logFile.c_str());
        //Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not open \"" + logFile + "\" for writing");
        return false;
    }

    return true;
}
