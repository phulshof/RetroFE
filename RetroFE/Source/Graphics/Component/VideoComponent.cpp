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
#include "../../Video/GStreamerVideo.h"
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
//        delete videoInst_;
        videoInst_ = NULL;
    }
}

void VideoComponent::update(float dt)
{
    if (videoInst_)
    {
        isPlaying_ = ((GStreamerVideo *)(videoInst_))->isPlaying();
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

    if(!isPlaying_)
    {
        isPlaying_ = videoInst_->play(videoFile_);
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
