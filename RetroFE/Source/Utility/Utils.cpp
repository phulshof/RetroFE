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

#include "Utils.h"
#include "../Database/Configuration.h"
#include "Log.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <locale>
#include <list>
#include <filesystem>

#ifdef WIN32
    #include <Windows.h>
#endif

Utils::Utils()
{
}

Utils::~Utils()
{
}

#ifdef WIN32
void Utils::postMessage( LPCTSTR windowTitle, UINT Msg, WPARAM wParam, LPARAM lParam ) {
    HWND hwnd = FindWindow(NULL, windowTitle);
	if (hwnd != NULL) {
        PostMessage(hwnd, Msg, wParam, lParam);
    }
}
#endif

std::string Utils::toLower(std::string str)
{
    for(unsigned int i=0; i < str.length(); ++i)
    {
        std::locale loc;
        str[i] = std::tolower(str[i], loc);
    }

    return str;
}

std::string Utils::uppercaseFirst(std::string str)
{
    if(str.length() > 0)
    {
        std::locale loc;
        str[0] = std::toupper(str[0], loc);
    }

    return str;
}
std::string Utils::filterComments(std::string line)
{
    size_t position;

    // strip out any comments
    if((position = line.find("#")) != std::string::npos)
    {
        line = line.substr(0, position);
    }
    // unix only wants \n. Windows uses \r\n. Strip off the \r for unix.
    line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );
    
    return line;
}

std::string Utils::combinePath(const std::list<std::string>& paths)
{
    std::filesystem::path result;
    for (const auto& p : paths)
    {
        result /= p;
    }
    return result.string();
}


bool Utils::findMatchingFile(const std::string& prefix, const std::vector<std::string>& extensions, std::string& file)
{
    for(const auto& ext : extensions)
    {
        std::string temp = prefix + "." + ext;
        temp = Configuration::convertToAbsolutePath(Configuration::absolutePath, temp);

        if(std::filesystem::exists(temp)) 
        {
            file = temp;
            return true;
        }
    }
    return false;
}


std::string Utils::replace(
    std::string subject,
    const std::string& search,
    const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.erase(pos, search.length());
        subject.insert(pos, replace);
        pos += replace.length();
    }
    return subject;
}


float Utils::convertFloat(const std::string& content)
{
    return std::stof(content);
}

int Utils::convertInt(const std::string& content) {
    return std::stoi(content);
}

void Utils::replaceSlashesWithUnderscores(std::string &content)
{
    std::replace(content.begin(), content.end(), '\\', '_');
    std::replace(content.begin(), content.end(), '/', '_');
}


std::string Utils::getDirectory(std::string filePath)
{

    std::string directory = filePath;

    const size_t last_slash_idx = filePath.rfind(pathSeparator);
    if (std::string::npos != last_slash_idx)
    {
        directory = filePath.substr(0, last_slash_idx);
    }

    return directory;
}

std::string Utils::getParentDirectory(std::string directory)
{
    size_t last_slash_idx = directory.find_last_of(pathSeparator);
    if(directory.length() - 1 == last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length()-1);
        last_slash_idx = directory.find_last_of(pathSeparator);
    }

    if (std::string::npos != last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length());
    }

    return directory;
}

std::string Utils::getEnvVar(std::string const& key)
{
    char const* val = std::getenv(key.c_str());

    return val == NULL ? std::string() : std::string(val);
}

std::string Utils::getFileName(std::string filePath)
{

    std::string filename = filePath;

    const size_t last_slash_idx = filePath.rfind(pathSeparator);
    if (std::string::npos != last_slash_idx)
    {
        filename = filePath.erase(0, last_slash_idx+1);
    }

    return filename;
}


std::string Utils::trimEnds(std::string str)
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


void Utils::listToVector(const std::string& str, std::vector<std::string>& vec, char delimiter)
{
    size_t previous = 0;
    size_t current;

    std::string value;  // Use this outside the loop and reuse it

    while ((current = str.find(delimiter, previous)) != std::string::npos)
    {
        value = trimEnds(str.substr(previous, current - previous));
        if (!value.empty()) {
            vec.push_back(std::move(value));  // Use std::move to avoid copying the string
        }
        previous = current + 1;
    }

    value = trimEnds(str.substr(previous));
    if (!value.empty()) {
        vec.push_back(std::move(value));
    }
}


int Utils::gcd( int a, int b )
{
    if (b == 0)
        return a;
    return gcd( b, a % b );
}

std::string Utils::trim(std::string& str)
{
    str.erase(str.find_last_not_of(' ') + 1);         //suffixing spaces
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    return str;
}