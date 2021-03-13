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
#include "Graphics/ViewInfo.h"
#include <vector>


class Configuration;


class SDL
{
public:
    static bool initialize( Configuration &config );
    static bool deInitialize( );
    static SDL_Renderer *getRenderer( int index );
    static SDL_mutex *getMutex( );
    static SDL_Window *getWindow( int index );
    static bool renderCopy( SDL_Texture *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo, int layoutWidth, int layoutHeight );
    static int getWindowWidth( int index )
    {
        return (index < numDisplays_ ? windowWidth_[index] : windowWidth_[0]);
    }
    static int getWindowHeight( int index )
    {
        return (index < numDisplays_ ? windowHeight_[index] : windowHeight_[0]);
    }
    static bool isFullscreen( int index )
    {
        return (index < numDisplays_ ? fullscreen_[index] : fullscreen_[0]);
    }
    static int getNumScreens( )
    {
        return numScreens_;
    }
    static int getNumDisplays( )
    {
        return numDisplays_;
    }

private:
    static std::vector<SDL_Window *>   window_;
    static std::vector<SDL_Renderer *> renderer_;
    static SDL_mutex                  *mutex_;
    static std::vector<int>            displayWidth_;
    static std::vector<int>            displayHeight_;
    static std::vector<int>            windowWidth_;
    static std::vector<int>            windowHeight_;
    static std::vector<bool>           fullscreen_;
    static int                         numScreens_;
    static int                         numDisplays_;
};
