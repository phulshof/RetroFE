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
#include "../../Video/VLCVideo.h"
#include "../../Video/VideoFactory.h"
#include "../../SDL.h"

VideoComponent::VideoComponent(IVideo *videoInst, Page &p, std::string videoFile)
    : Component(p)
    , videoFile_(videoFile)
    , videoInst_(videoInst)
    , isPlaying_(false)
{
//   AllocateGraphicsMemory();
}

VideoComponent::~VideoComponent()
{
    freeGraphicsMemory();

    if(videoInst_)
    {
        videoInst_->stop();
        if ( VideoFactory::canDelete( videoInst_ ) )
            delete videoInst_;
        videoInst_ = NULL;
    }
}

void VideoComponent::update(float dt)
{
    if (videoInst_)
    {
        bool wasPlaying = isPlaying_;
        isPlaying_ = ((VLCVideo *)(videoInst_))->isPlaying();

        // Only handle volume-based start/stop for non-intro videos
        // Check if this is likely a background video/audio by looking at the file
        bool isBackgroundMedia = (videoFile_.find("sounds/") != std::string::npos) ||
                                  (videoFile_.find("video/") != std::string::npos &&
                                   videoFile_.find("intro") == std::string::npos);

        if(isBackgroundMedia)
        {
            // Start playing if volume becomes > 0.01 and not already playing
            // Using 0.01 threshold to allow very quiet audio
            if(!wasPlaying && !isPlaying_ && baseViewInfo.Volume > 0.01f)
            {
                isPlaying_ = videoInst_->play(videoFile_);
            }
            // Stop playing if volume becomes very low (essentially 0)
            else if(isPlaying_ && baseViewInfo.Volume <= 0.01f)
            {
                videoInst_->stop();
                isPlaying_ = false;
            }
        }
    }

    if(isPlaying_)
    {
        videoInst_->setVolume(baseViewInfo.Volume);
        videoInst_->update(dt);

        // video needs to run a frame to start getting size info
        if(baseViewInfo.ImageHeight == 0 && baseViewInfo.ImageWidth == 0)
        {
            baseViewInfo.ImageHeight = static_cast<float>(videoInst_->getHeight());
            baseViewInfo.ImageWidth = static_cast<float>(videoInst_->getWidth());
        }
    }

    Component::update(dt);

}

void VideoComponent::allocateGraphicsMemory()
{
    Component::allocateGraphicsMemory();

    // Check if this is likely a background video/audio
    bool isBackgroundMedia = (videoFile_.find("sounds/") != std::string::npos) ||
                              (videoFile_.find("video/") != std::string::npos &&
                               videoFile_.find("intro") == std::string::npos);

    // Only apply volume check for background media, not intro videos
    if(!isPlaying_)
    {
        if(isBackgroundMedia)
        {
            // Only start playing if volume is greater than 0.01
            if(baseViewInfo.Volume > 0.01f)
            {
                isPlaying_ = videoInst_->play(videoFile_);
            }
        }
        else
        {
            // Always play intro videos and other non-background videos
            isPlaying_ = videoInst_->play(videoFile_);
        }
    }
}

void VideoComponent::freeGraphicsMemory()
{
    videoInst_->stop();
    isPlaying_ = false;

    Component::freeGraphicsMemory();
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
