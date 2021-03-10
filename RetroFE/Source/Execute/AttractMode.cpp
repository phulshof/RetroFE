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
#include "AttractMode.h"
#include "../Graphics/Page.h"

#include <cstdlib>

AttractMode::AttractMode()
    : idleTime(0)
    , idleNextTime(0)
    , isActive_(false)
    , isSet_(false)
    , elapsedTime_(0)
    , elapsedPlaylistTime_(0)
    , elapsedCollectionTime_(0)
    , activeTime_(0)
{
}

void AttractMode::reset( bool set )
{
    elapsedTime_ = 0;
    isActive_    = false;
    isSet_       = set;
    activeTime_  = 0;
    if (!set)
    {
        elapsedPlaylistTime_   = 0;
        elapsedCollectionTime_ = 0;
    }
}

int AttractMode::update(float dt, Page &page)
{

    elapsedTime_           += dt;
    elapsedPlaylistTime_   += dt;
    elapsedCollectionTime_ += dt;

    if ( page.isJukebox() )
    {
        if ( !isActive_ && !page.isJukeboxPlaying() && elapsedTime_ > 10 )
        {
            isActive_    = true;
            isSet_       = true;
            elapsedTime_ = 0;
            activeTime_  = ((float)(minTime+(rand() % (maxTime-minTime)))) / 1000;
        }
    }
    else
    {

        // Check if it's time to switch playlists
        if (!isActive_ && elapsedPlaylistTime_ > idlePlaylistTime && idlePlaylistTime > 0)
        {
            elapsedTime_         = 0;
            elapsedPlaylistTime_ = 0;
            return 1;
        }

        // Check if it's time to switch collections
        if (!isActive_ && elapsedCollectionTime_ > idleCollectionTime && idleCollectionTime > 0)
        {
            elapsedTime_           = 0;
            elapsedPlaylistTime_   = 0;
            elapsedCollectionTime_ = 0;
            return 2;
        }
    
        // enable attract mode when idling for the expected time. Disable if idle time is set to 0.
        if(!isActive_ && ((elapsedTime_ > idleTime && idleTime > 0) || (isSet_ && elapsedTime_ > idleNextTime && idleNextTime > 0)))
        {
            if (!isSet_)
                elapsedPlaylistTime_ = 0; // Reset playlist timer if we are entering attract mode
            isActive_    = true;
            isSet_       = true;
            elapsedTime_ = 0;
            activeTime_  = ((float)(minTime+(rand() % (maxTime-minTime)))) / 1000;
        }

    }

    if(isActive_)
    {
        page.setScrolling(Page::ScrollDirectionForward);

        if (page.isMenuIdle())
        {
            page.scroll(true);
            page.updateScrollPeriod();
        }

        if(elapsedTime_ > activeTime_)
        {
            elapsedTime_ = 0;
            isActive_ = false;
        }
    }

    return 0;

}


bool AttractMode::isActive()
{
    return isActive_;
}


bool AttractMode::isSet()
{
    return isSet_;
}
