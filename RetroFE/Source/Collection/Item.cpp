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

#include "Item.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>

Item::Item()
    : collectionInfo(NULL)
    , leaf(true)
{
    file = "";
    isFavorite = false;
}

Item::~Item()
{
}

std::string Item::filename()
{
    return Utils::getFileName(filepath);
}



std::string Item::lowercaseTitle()
{
    std::string lcstr = title;
    std::transform(lcstr.begin(), lcstr.end(), lcstr.begin(), ::tolower);
    return lcstr;
}

std::string Item::lowercaseFullTitle()
{
    std::string lcstr = fullTitle;
    std::transform(lcstr.begin(), lcstr.end(), lcstr.begin(), ::tolower);
    return lcstr;
}


void Item::setInfo( std::string key, std::string value )
{
    info_.insert( InfoPair( key, value ) );
}


bool Item::getInfo( std::string key, std::string & value )
{

   bool retVal = false;

   if ( info_.find( key ) != info_.end( ) )
   {
       value  = info_[key];
       retVal = true;
   }

   return retVal;

}


void Item::loadInfo( std::string path )
{

    int           lineCount = 0;
    std::string   line;
    std::ifstream ifs( path.c_str( ) );
    size_t        position;
    std::string   key;
    std::string   value;

    if ( !ifs.is_open( ) )
    {
        return;
    }

    while ( std::getline( ifs, line ) )
    {
        lineCount++;
        line = Utils::filterComments( line );
        // Check if the line has an assigment operator
        if ( (position = line.find( "=" )) != std::string::npos )
        {
            key   = line.substr( 0, position );
            key   = Utils::trimEnds( key );
            value = line.substr( position + 1, line.size( )-1 );
            value = Utils::trimEnds( value );
            setInfo( key, value );
        }
        else
        {
            std::stringstream ss;
            ss << "Missing an assignment operator (=) on line " << lineCount;
            Logger::write(Logger::ZONE_ERROR, "Item", ss.str());
        }
    }
    
}
