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
#include "Component.h"
#include "Image.h"
#include "../Page.h"
#include "../../Collection/Item.h"
#include "../../Video/IVideo.h"
#include <SDL2/SDL.h>
#include <string>

class VideoComponent : public Component
{
public:
    VideoComponent(IVideo *videoInst, Page &p, const std::string& videoFile);
    virtual ~VideoComponent();
    bool update(float dt);
    void draw();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    virtual bool isPlaying();
    virtual void skipForward( );
    virtual void skipBackward( );
    virtual void skipForwardp( );
    virtual void skipBackwardp( );
    virtual void pause( );
    virtual void restart( );
    virtual unsigned long long getCurrent( );
    virtual unsigned long long getDuration( );
    virtual bool isPaused( );

private:
    std::string videoFile_;
    std::string name_;
    IVideo *videoInst_;
    bool isPlaying_;
};
