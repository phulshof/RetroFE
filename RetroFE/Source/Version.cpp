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


#include "Version.h"
#include <string>


std::string retrofe_version_major = "0";
std::string retrofe_version_minor = "10";
std::string retrofe_version_build = "16";


std::string Version::getString( )
{
    std::string return_string = retrofe_version_major + "." + retrofe_version_minor + "." + retrofe_version_build;
    return return_string;
}
