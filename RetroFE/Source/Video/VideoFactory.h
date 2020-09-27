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

class IVideo;

class VideoFactory
{
public:
    static IVideo *createVideo( int monitor, bool isTypeVideo );
    static void    setEnabled(bool enabled);
    static void    setNumLoops(int numLoops);

private:
    static bool    enabled_;
    static int     numLoops_;
    static IVideo *instance_;
};
