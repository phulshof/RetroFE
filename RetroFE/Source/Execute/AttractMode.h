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

class Page;

class AttractMode
{
public:
    AttractMode();
    void reset( bool set = false );
    int   update(float dt, Page &page);
    float idleTime;
    float idleNextTime;
    float idlePlaylistTime;
    float idleCollectionTime;
	int   minTime;
	int   maxTime;
    bool  isActive();
    bool  isSet();

private:
    bool isActive_;
    bool isSet_;
    float elapsedTime_;
    float elapsedPlaylistTime_;
    float elapsedCollectionTime_;
    float activeTime_;
};
