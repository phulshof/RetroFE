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
#include <vector>
#include <string>
#include <algorithm>

bool VideoFactory::enabled_ = true;
int VideoFactory::numLoops_ = 0;
std::vector<IVideo*> VideoFactory::videoInstances_ = {};


IVideo *VideoFactory::createVideo(int monitor, bool isTypeVideo, int numLoops)
{
    GStreamerVideo *instance = NULL;
    if (enabled_)
    {
        instance = new GStreamerVideo(monitor);
        instance->initialize();
        addInstance(instance);  // Add to the list of instances
        Logger::write(Logger::ZONE_DEBUG, "VideoFactory", "Video count = " + std::to_string(getInstanceCount()));
    }

    if (numLoops > 0 )
    {
        ((GStreamerVideo *)(instance))->setNumLoops(numLoops);
    }
    else
    {
        ((GStreamerVideo *)(instance))->setNumLoops(numLoops_);
    }

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

void VideoFactory::addInstance(IVideo* instance)
{
    videoInstances_.push_back(instance);
}

void VideoFactory::removeInstance(IVideo* instance)
{
    std::vector<IVideo*>::iterator it = std::find(videoInstances_.begin(), videoInstances_.end(), instance);
    if (it != videoInstances_.end()) {
        videoInstances_.erase(it);
    }
}

int VideoFactory::getInstanceCount()
{
    return static_cast<int>(videoInstances_.size());
}