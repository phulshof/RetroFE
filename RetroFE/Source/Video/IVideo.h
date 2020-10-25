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

#include <SDL2/SDL.h>
#include <string>

class IVideo
{
public:
    virtual ~IVideo() {}
    virtual bool initialize() = 0;
    virtual bool play(std::string file) = 0;
    virtual bool stop() = 0;
    virtual bool deInitialize() = 0;
    virtual SDL_Texture *getTexture() const = 0;
    virtual void update(float dt) = 0;
    virtual void draw() = 0;
    virtual int getHeight() = 0;
    virtual int getWidth() = 0;
    virtual void setVolume(float volume) = 0;
};
