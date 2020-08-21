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


class Configuration;


class SDL
{
public:
    static bool initialize( Configuration &config );
    static bool deInitialize( );
    static SDL_Renderer *getRenderer( );
    static SDL_mutex *getMutex( );
    static SDL_Window *getWindow( );
    static bool renderCopy( SDL_Texture *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo, float scaleX, float scaleY );
    static int getWindowWidth( )
    {
        return windowWidth_;
    }
    static int getWindowHeight( )
    {
        return windowHeight_;
    }
    static bool isFullscreen( )
    {
        return fullscreen_;
    }

private:
    static SDL_Window   *window_;
    static SDL_Renderer *renderer_;
    static SDL_mutex    *mutex_;
    static int           displayWidth_;
    static int           displayHeight_;
    static int           windowWidth_;
    static int           windowHeight_;
    static bool          fullscreen_;
};
