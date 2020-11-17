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

#include "VideoFactory.h"
#include "IVideo.h"
#include "../Utility/Log.h"
#include "GStreamerVideo.h"

bool VideoFactory::enabled_ = true;
int VideoFactory::numLoops_ = 0;
IVideo *VideoFactory::instance_ = NULL;


IVideo *VideoFactory::createVideo( int monitor, bool isTypeVideo, int numLoops )
{
    IVideo *instance = NULL;
    if ( enabled_ && (!isTypeVideo || !instance_) )
    {
        instance = new GStreamerVideo( monitor );
        instance->initialize();
        if ( isTypeVideo )
            instance_ = instance;
    }
    if ( isTypeVideo )
        instance = instance_;

    if (numLoops > 0 )
        ((GStreamerVideo *)(instance))->setNumLoops(numLoops);
    else
        ((GStreamerVideo *)(instance))->setNumLoops(numLoops_);

    return instance;
}


void VideoFactory::setEnabled(bool enabled)
{
    enabled_ = enabled;
}


void VideoFactory::setNumLoops(int numLoops)
{
    numLoops_ = numLoops;
}
