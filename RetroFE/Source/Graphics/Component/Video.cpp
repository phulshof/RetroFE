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

#include "Video.h"
#include "VideoComponent.h"
#include "VideoBuilder.h"
#include "../../Video/IVideo.h"
#include "../../Video/GStreamerVideo.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"


bool Video::enabled_ = true;


Video::Video(std::string file, std::string altFile, int numLoops, Page &p, int monitor)
    : Component(p)
    , video_(NULL)
    , file_(file)
    , altFile_(altFile)
    , numLoops_(numLoops)

{
    baseViewInfo.Monitor = monitor;
    allocateGraphicsMemory( );
}

Video::~Video( )
{
    if (video_ != NULL)
    {
        delete video_;
    }
}


void Video::setEnabled(bool enabled)
{
    enabled_ = enabled;
}


void Video::update(float dt)
{
    if(video_)
    {
        // video needs to run a frame to start getting size info
        if(baseViewInfo.ImageHeight == 0 && baseViewInfo.ImageWidth == 0)
        {
            baseViewInfo.ImageWidth = video_->baseViewInfo.ImageWidth;
            baseViewInfo.ImageHeight = video_->baseViewInfo.ImageHeight;
        }

        video_->update(dt);
    }
    Component::update(dt);
}


void Video::freeGraphicsMemory( )
{
    Component::freeGraphicsMemory( );
    if(video_)
    {
        video_->freeGraphicsMemory( );
    }
}


void Video::allocateGraphicsMemory( )
{

    Component::allocateGraphicsMemory( );

    if (enabled_ && !video_)
    {
        std::string file = "";

        std::ifstream f(altFile_.c_str( ));
        if (f.good( ))
            file = altFile_;

        std::ifstream g(file_.c_str( ));
        if (g.good( ))
            file = file_;

        if (file != "")
        {
            IVideo      *video = new GStreamerVideo( baseViewInfo.Monitor );
            video->initialize();
            ((GStreamerVideo *)(video))->setNumLoops( numLoops_ );
            video_ = new VideoComponent( video, page, file );
        }
    }

    if (video_)
        video_->allocateGraphicsMemory();

}


void Video::draw( )
{
    Component::draw( );
    if(video_)
    {
    	baseViewInfo.ImageHeight = video_->baseViewInfo.ImageHeight;
    	baseViewInfo.ImageWidth  = video_->baseViewInfo.ImageWidth;
        video_->baseViewInfo     = baseViewInfo;
        video_->draw( );
    }
}


bool Video::isPlaying( )
{
    if (video_)
    {
        return video_->isPlaying( );
    }
    else
    {
        return false;
    }
}
