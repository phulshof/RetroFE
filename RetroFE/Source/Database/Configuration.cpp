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
#include "Configuration.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include <algorithm>
#include <locale>
#include <fstream>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#elif __APPLE__
#include <libproc.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

std::string Configuration::absolutePath;
bool Configuration::HardwareVideoAccel = false;
int Configuration::AvdecMaxThreads = 2;
bool Configuration::MuteVideo = false;

Configuration::Configuration()
{
}

Configuration::~Configuration()
{
}

void Configuration::initialize()
{
    const char *environment = std::getenv("RETROFE_PATH");
    std::string sPath; // Declare sPath here

#if defined(__linux) || defined(__APPLE__)
    std::string home_load = std::getenv("HOME") + std::string("/.retrofe");
    std::ifstream retrofe_path(home_load.c_str());
#endif

    // Check Environment for path
    if (environment != NULL)
    {
        absolutePath = environment;
    }
#if defined(__linux) || defined(__APPLE__)
    // Or check for home based flat file works on linux/mac
    else if (retrofe_path && std::getline( retrofe_path, absolutePath ))
    {
    	retrofe_path.close();
    }
#endif
    // Or check executable for path
    else
    {
#ifdef WIN32
        HMODULE hModule = GetModuleHandle(NULL);
        CHAR exe[MAX_PATH];
        GetModuleFileName(hModule, exe, MAX_PATH);
        sPath = Utils::getDirectory(exe);
        sPath = Utils::getParentDirectory(sPath);
#elif __APPLE__
    	char exepath[PROC_PIDPATHINFO_MAXSIZE];
    	if( proc_pidpath (getpid(), exepath, sizeof(exepath)) <= 0 ) // Error to console if no path to write logs…
            fprintf(stderr, "Cannot set absolutePath: %s\nOverride with RETROFE_PATH env var\n", strerror(errno));
        sPath = Utils::getDirectory(exepath);

        // RetroFE can be started from the command line: 'retrofe' or as an app, by clicking RetroFE.app.
        // If this was started as an app bundle, relocate it's current working directory to the root folder.
        // as an example /usr/local/opt/retro/RetroFE.app/Contents/MacOS becomes /usr/local/opt/retrofe
        // Note: executing 'brew applinks retrofe' - should create symlink to /Applications/RetroFE.app
        size_t rootPos = sPath.find("/RetroFE.app/Contents/MacOS");
	if(rootPos!=std::string::npos) 
		sPath = sPath.erase(rootPos);
#else
        char exepath[1024] = {};
        char buffer[1024] = {}; // Separate buffer for readlink result
        sprintf(exepath, "/proc/%d/exe", getpid());
        ssize_t len = readlink(exepath, buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0'; // Null-terminate the result
            sPath = Utils::getDirectory(buffer);
        }
#endif

        absolutePath = sPath;
    }
}




void Configuration::clearProperties( )
{
    properties_.clear( );
}


bool Configuration::import(std::string keyPrefix, std::string file, bool mustExist)
{
    return import("", keyPrefix, file, mustExist);
}

bool Configuration::import(std::string collection, std::string keyPrefix, std::string file, bool mustExist)
{
    bool retVal = true;
    int lineCount = 0;
    std::string line;

    Logger::write(Logger::ZONE_INFO, "Configuration", "Importing \"" + file + "\"");

    std::ifstream ifs(file.c_str());

    if (!ifs.is_open())
    {
        if (mustExist)
        {
            Logger::write(Logger::ZONE_ERROR, "Configuration", "Could not open " + file + "\"");
        }
        else
        {
            Logger::write(Logger::ZONE_WARNING, "Configuration", "Could not open " + file + "\"");
        }

        return false;
    }

    while (std::getline (ifs, line))
    {
        lineCount++;
        retVal = retVal && parseLine(collection, keyPrefix, line, lineCount);
    }

    ifs.close();

    return retVal;
}


bool Configuration::parseLine(std::string collection, std::string keyPrefix, std::string line, int lineCount)
{
    bool retVal = false;
    std::string key;
    std::string value;
    size_t position;
    std::string delimiter = "=";

    // strip out any comments
    line = Utils::filterComments(line);
    
    if(line.empty() || (line.find_first_not_of(" \t\r") == std::string::npos))
    {
        retVal = true;
    }
    // all configuration fields must have an assignment operator
    else if ((position = line.find(delimiter)) != std::string::npos)
    {
        if (keyPrefix.size() != 0)
        {
            keyPrefix += ".";
        }

        key = keyPrefix + line.substr(0, position);

        key = trimEnds(key);


        value = line.substr(position + delimiter.length(), line.length());
        value = trimEnds(value);

        if (collection != "")
        {
            value = Utils::replace(value, "%ITEM_COLLECTION_NAME%", collection);
        }

        properties_[key] = value;

        std::stringstream ss;
        ss << "Dump: "  << "\"" << key << "\" = \"" << value << "\"";


        Logger::write(Logger::ZONE_INFO, "Configuration", ss.str());
        retVal = true;
    }
    else
    {
        std::stringstream ss;
        ss << "Missing an assignment operator (=) on line " << lineCount;
        Logger::write(Logger::ZONE_ERROR, "Configuration", ss.str());
    }

    return retVal;
}

std::string Configuration::trimEnds(std::string str)
{
    // strip off any initial tabs or spaces
    size_t trimStart = str.find_first_not_of(" \t");

    if(trimStart != std::string::npos)
    {
        size_t trimEnd = str.find_last_not_of(" \t");

        str = str.substr(trimStart, trimEnd - trimStart + 1);
    }

    return str;
}

bool Configuration::getRawProperty(std::string key, std::string &value)
{
    bool retVal = false;

    if(properties_.find(key) != properties_.end())
    {
        value = properties_[key];

        retVal = true;
    }

    return retVal;
}
bool Configuration::getProperty(std::string key, std::string &value)
{
    bool retVal = getRawProperty(key, value);

    std::string baseMediaPath = absolutePath;
    std::string baseItemPath  = absolutePath;

    baseMediaPath = Utils::combinePath(absolutePath, "collections");
    baseItemPath  = Utils::combinePath(absolutePath, "collections");

    getRawProperty("baseMediaPath", baseMediaPath);
    getRawProperty("baseItemPath", baseItemPath);

    value = Utils::replace(value, "%BASE_MEDIA_PATH%", baseMediaPath);
    value = Utils::replace(value, "%BASE_ITEM_PATH%", baseItemPath);
    return retVal;
}

bool Configuration::getProperty(std::string key, int &value)
{
    std::string strValue;

    bool retVal = getProperty(key, strValue);

    if(retVal)
    {
        std::stringstream ss;
        ss << strValue;
        ss >> value;
    }

    return retVal;
}

bool Configuration::getProperty(std::string key, bool &value)
{
    std::string strValue;

    bool retVal = getProperty(key, strValue);

    if(retVal)
    {
        if(!strValue.compare("yes") || !strValue.compare("true") || !strValue.compare("on"))
        {
            value = true;
        }
        else
        {
            value = false;
        }
    }

    return retVal;
}

void Configuration::setProperty(std::string key, std::string value)
{
    properties_[key] = value;
}

bool Configuration::propertiesEmpty()
{
    return properties_.empty();
}

bool Configuration::propertyExists(std::string key)
{
    return (properties_.find(key) != properties_.end());
}

bool Configuration::propertyPrefixExists(std::string key)
{
    PropertiesType::iterator it;

    for(it = properties_.begin(); it != properties_.end(); ++it)
    {
        std::string search = key + ".";
        if(it->first.compare(0, search.length(), search) == 0)
        {
            return true;
        }
    }

    return false;
}

void Configuration::childKeyCrumbs(std::string parent, std::vector<std::string> &children)
{
    PropertiesType::iterator it;

    for(it = properties_.begin(); it != properties_.end(); ++it)
    {
        std::string search = parent + ".";
        if(it->first.compare(0, search.length(), search) == 0)
        {
            std::string crumb = Utils::replace(it->first, search, "");

            std::size_t end = crumb.find_first_of(".");

            if(end != std::string::npos)
            {
                crumb = crumb.substr(0, end);
            }

            if(std::find(children.begin(), children.end(), crumb) == children.end())
            {
                children.push_back(crumb);
            }
        }
    }
}

std::string Configuration::convertToAbsolutePath(const std::string& prefix, const std::string& path)
{
    char first = (path.size() > 0) ? path[0] : ' ';
    char second = (path.size() > 1) ? path[1] : ' ';

    // check to see if it is already an absolute path
    if((first != Utils::pathSeparator) && (second != ':'))
    {
        return Utils::combinePath(prefix, path);
    }
    return path;
}


bool Configuration::getPropertyAbsolutePath(std::string key, std::string &value)
{
    bool retVal = getProperty(key, value);

    if(retVal)
    {
        value = convertToAbsolutePath(absolutePath, value);
    }

    return retVal;
}

void Configuration::getMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, std::string &value)
{
    return getMediaPropertyAbsolutePath(collectionName, mediaType, false, value);
}


void Configuration::getMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, bool system, std::string &value)
{
    std::string key = "collections." + collectionName + ".media." + mediaType;
    if (system) 
    {
        key = "collections." + collectionName + ".media.system_artwork";
    }

    // use user-overridden setting if it exists
    if(getPropertyAbsolutePath(key, value))
    {
        return;
    }

    // use user-overridden base media path if it was specified
    std::string baseMediaPath;
    if(!getPropertyAbsolutePath("baseMediaPath", baseMediaPath))
    {
        // base media path was not specified, assume media files are in the collection
        baseMediaPath = Utils::combinePath(absolutePath, "collections");
    }

    if(system)
    {
        value = Utils::combinePath(baseMediaPath, collectionName, "system_artwork");
    }
    else
    {
        value = Utils::combinePath(baseMediaPath, collectionName, "medium_artwork", mediaType);
    }
}

void Configuration::getCollectionAbsolutePath(std::string collectionName, std::string &value)
{
    std::string key = "collections." + collectionName + ".list.path";

    if(getPropertyAbsolutePath(key, value))
    {
        return;
    }

    std::string baseItemPath;
    if(getPropertyAbsolutePath("baseItemPath", baseItemPath))
    {
        value = Utils::combinePath(baseItemPath, collectionName);
        return;
    }

    value = Utils::combinePath(absolutePath, "collections", collectionName, "roms");
}

