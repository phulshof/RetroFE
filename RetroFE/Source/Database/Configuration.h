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
#include <vector>

class Configuration
{
public:
    Configuration();
    virtual ~Configuration();
    static void initialize();
    static std::string convertToAbsolutePath(const std::string& prefix, const std::string& path);
    static std::string trimEnds(std::string str);
	void clearProperties( );
    // gets the global configuration
    bool import(std::string keyPrefix, std::string file, bool mustExist = true);
    bool import(std::string collection, std::string keyPrefix, std::string file, bool mustExist = true);
    bool getProperty(std::string key, std::string &value);
    bool getProperty(std::string key, int &value);
    bool getProperty(std::string key, bool &value);
    void childKeyCrumbs(std::string parent, std::vector<std::string> &children);
    void setProperty(std::string key, std::string value);
    bool propertiesEmpty();
    bool propertyExists(std::string key);
    bool propertyPrefixExists(std::string key);
    bool getPropertyAbsolutePath(std::string key, std::string &value);
    void getMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, std::string &value);
    void getMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, bool system, std::string &value);
    void getCollectionAbsolutePath(std::string collectionName, std::string &value);
    static std::string absolutePath;
	static int AvdecMaxThreads;
	static bool HardwareVideoAccel; // Declare HardwareVideoAccel as a static member variable
	static bool MuteVideo;

private:
    bool getRawProperty(std::string key, std::string &value);
    bool parseLine(std::string collection, std::string keyPrefix, std::string line, int lineCount);
    typedef std::map<std::string, std::string> PropertiesType;
    typedef std::pair<std::string, std::string> PropertiesPair;

    PropertiesType properties_;

};
