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

#include "VideoComponent.h"
#include "../ViewInfo.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../Utility/Utils.h"
#include "../../Video/GStreamerVideo.h"
#include "../../Video/VideoFactory.h"
#include "../../SDL.h"
#include <string>

VideoComponent::VideoComponent(IVideo *videoInst, Page &p, const std::string& videoFile, const std::string& origin)
    : Component(p)
    , videoFile_(videoFile)
    , videoInst_(videoInst)
    , origin_(origin)
    , isPlaying_(false)
    , initialLoad_ (true)
    , hasPlayedOnce_ (false)
{
//   AllocateGraphicsMemory();
}

VideoComponent::~VideoComponent()
{
    freeGraphicsMemory();
}


bool VideoComponent::update(float dt)
{
    if (videoInst_)
    {
        isPlaying_ = ((GStreamerVideo *)(videoInst_))->isPlaying();
    }

    if(isPlaying_ && !hasPlayedOnce_)
    {
        // Mark this video as having played at least once.
        hasPlayedOnce_ = true;

        // If it's the first time it's playing and Restart is true, we ignore it.
        baseViewInfo.Restart = false;
    }

    if(isPlaying_)
    {
        if (baseViewInfo.Restart) {
            restart();
            baseViewInfo.Restart = false;
        }
        if (videoInst_->getTexture()) 
        {
            if (baseViewInfo.PauseOnScroll)
            {
                if (baseViewInfo.Alpha == 0.0 && !isPaused())
                {
                    videoInst_->pause( );
                    //Logger::write(Logger::ZONE_DEBUG, "VideoComponent", "Paused " + Utils::getFileName(videoFile_));
                }
                if (baseViewInfo.Alpha != 0.0 && isPaused()) 
                {
                    // unpause
                    videoInst_->pause( );
                    //Logger::write(Logger::ZONE_DEBUG, "VideoComponent", "Resumed " + Utils::getFileName(videoFile_));
                }
            }
        }
        videoInst_->setVolume(baseViewInfo.Volume);
        videoInst_->update(dt);

        // video needs to run a frame to start getting size info
        if(baseViewInfo.ImageHeight == 0 && baseViewInfo.ImageWidth == 0)
        {
            baseViewInfo.ImageHeight = static_cast<float>(videoInst_->getHeight());
            baseViewInfo.ImageWidth = static_cast<float>(videoInst_->getWidth());
        }
    }

    return Component::update(dt);
}





void VideoComponent::allocateGraphicsMemory()
{
    Component::allocateGraphicsMemory();

    if(!isPlaying_)
    {
        isPlaying_ = videoInst_->play(videoFile_);
    }
}

void VideoComponent::freeGraphicsMemory()
{
    //videoInst_->stop()
        
    Component::freeGraphicsMemory();
    Logger::write(Logger::ZONE_DEBUG, "VideoComponent", "Component Freed " + Utils::getFileName(videoFile_));
    
    if (videoInst_) 
    {
        VideoFactory::removeInstance(videoInst_);  // Inform VideoFactory about the instance removal.
        delete videoInst_;
        isPlaying_ = false;
        Logger::write(Logger::ZONE_DEBUG, "VideoComponent", "Deleted " + Utils::getFileName(videoFile_));
        videoInst_ = NULL;
        
    }
}


void VideoComponent::draw()
{
    SDL_Rect rect;

    rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
    rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
    rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
    rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

    videoInst_->draw();
    SDL_Texture *texture = videoInst_->getTexture();

    if(texture)
    {
        SDL::renderCopy(texture, baseViewInfo.Alpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));
    }
}

bool VideoComponent::isPlaying()
{
    return isPlaying_;
}


void VideoComponent::skipForward( )
{
    if ( videoInst_ )
        videoInst_->skipForward( );
}


void VideoComponent::skipBackward( )
{
    if ( videoInst_ )
        videoInst_->skipBackward( );
}


void VideoComponent::skipForwardp( )
{
    if ( videoInst_ )
        videoInst_->skipForwardp( );
}


void VideoComponent::skipBackwardp( )
{
    if ( videoInst_ )
        videoInst_->skipBackwardp( );
}


void VideoComponent::pause( )
{
    if ( videoInst_ )
        videoInst_->pause( );
}


void VideoComponent::restart( )
{
    if ( videoInst_ )
        videoInst_->restart( );
}


unsigned long long VideoComponent::getCurrent( )
{
    if ( videoInst_ )
        return videoInst_->getCurrent( );
    else
        return 0;
}


unsigned long long VideoComponent::getDuration( )
{
    if ( videoInst_ )
        return videoInst_->getDuration( );
    else
        return 0;
}


bool VideoComponent::isPaused( )
{
    if ( videoInst_ )
        return videoInst_->isPaused( );
    else
        return false;
}
