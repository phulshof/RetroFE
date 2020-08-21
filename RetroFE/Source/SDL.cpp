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


#include "SDL.h"
#include "Database/Configuration.h"
#include "Utility/Log.h"
#include <SDL2/SDL_mixer.h>


SDL_Window   *SDL::window_        = NULL;
SDL_Renderer *SDL::renderer_      = NULL;
SDL_mutex    *SDL::mutex_         = NULL;
int           SDL::displayWidth_  = 0;
int           SDL::displayHeight_ = 0;
int           SDL::windowWidth_   = 0;
int           SDL::windowHeight_  = 0;
bool          SDL::fullscreen_    = false;


// Initialize SDL
bool SDL::initialize( Configuration &config )
{

    bool        retVal        = true;
    std::string hString;
    std::string vString;
    Uint32      windowFlags   = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
    int         audioRate     = MIX_DEFAULT_FREQUENCY;
    Uint16      audioFormat   = MIX_DEFAULT_FORMAT; /* 16-bit stereo */
    int         audioChannels = 1;
    int         audioBuffers  = 4096;
    bool        hideMouse;

    Logger::write( Logger::ZONE_INFO, "SDL", "Initializing" );
    if (retVal && SDL_Init( SDL_INIT_EVERYTHING ) != 0)
    {
        std::string error = SDL_GetError( );
        Logger::write( Logger::ZONE_ERROR, "SDL", "Initialize failed: " + error );
        retVal = false;
    }

    if ( retVal && config.getProperty( "hideMouse", hideMouse ) )
    {
        if ( hideMouse )
        {
            SDL_ShowCursor( SDL_FALSE );
        }
        else
        {
            SDL_ShowCursor( SDL_TRUE );
        }
    }

    // check for a few other necessary Configurations
    if ( retVal )
    {
        Logger::write( Logger::ZONE_INFO, "SDL", "Number of displays: " + std::to_string(SDL_GetNumVideoDisplays( )) );
        // Get current display mode of all displays.
        for(int i = 0; i < SDL_GetNumVideoDisplays( ); ++i)
        {
            SDL_DisplayMode mode;
            if ( SDL_GetCurrentDisplayMode( i, &mode ) == 0 )
            {
                displayWidth_  = mode.w;
                displayHeight_ = mode.h;
                break;
            }
        }

        if ( !config.getProperty( "horizontal", hString ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property \"horizontal\"" );
            retVal = false;
        }
        else if ( hString == "stretch" )
        {
            // Get current display mode of all displays.
            for(int i = 0; i < SDL_GetNumVideoDisplays( ); ++i)
            {
                SDL_DisplayMode mode;
                if ( SDL_GetCurrentDisplayMode( i, &mode ) == 0 )
                {
                    windowWidth_ = mode.w;
                    break;
                }
            }
        }
        else if ( !config.getProperty( "horizontal", windowWidth_ ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"horizontal\"" );
        }
    }

    if ( retVal )
    {
        if ( !config.getProperty( "vertical", vString ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property \"vertical\"" );
            retVal = false;
        }
        else if ( vString == "stretch" )
        {
            // Get current display mode of all displays.
            for(int i = 0; i < SDL_GetNumVideoDisplays( ); ++i)
            {
                SDL_DisplayMode mode;
                if ( SDL_GetDesktopDisplayMode( i, &mode ) == 0 )
                {
                    windowHeight_ = mode.h;
                    break;
                }
            }
        }
        else if ( !config.getProperty( "vertical", windowHeight_ ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"vertical\"" );
        }
    }

    if ( retVal && !config.getProperty( "fullscreen", fullscreen_ ) )
    {
        Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property: \"fullscreen\"" );
        retVal = false;
    }

    if (retVal && fullscreen_)
    {
#ifdef WIN32
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
        windowFlags |= SDL_WINDOW_FULLSCREEN;
#endif
    }

    if ( retVal )
    {
        std::string fullscreenStr = fullscreen_ ? "yes" : "no";
        std::stringstream ss;
        ss << "Creating "<< windowWidth_ << "x" << windowHeight_ << " window (fullscreen: " << fullscreenStr << ")";

        Logger::write( Logger::ZONE_INFO, "SDL", ss.str( ));

        window_ = SDL_CreateWindow( "RetroFE", SDL_WINDOWPOS_CENTERED_DISPLAY(1), SDL_WINDOWPOS_CENTERED_DISPLAY(1), windowWidth_, windowHeight_, windowFlags );

        if ( window_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "Create window failed: " + error );
            retVal = false;
        }
    }

    if ( retVal )
    {
        renderer_ = SDL_CreateRenderer( window_, -1, SDL_RENDERER_ACCELERATED );

        if ( renderer_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "Create renderer failed: " + error );
            retVal = false;
        }
    }

    if ( SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1") != SDL_TRUE )
    {
        Logger::write( Logger::ZONE_ERROR, "SDL", "Improve scale quality. Continuing with low-quality settings." );
    }

    bool minimize_on_focus_loss_;
    if ( config.getProperty( "minimize_on_focus_loss", minimize_on_focus_loss_ ) )
    {
        if ( minimize_on_focus_loss_ )
        {
            SDL_SetHintWithPriority( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1", SDL_HINT_OVERRIDE );
        }
        else
        {
            SDL_SetHintWithPriority( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0", SDL_HINT_OVERRIDE );
        }
    }

    if ( retVal )
    {
        mutex_ = SDL_CreateMutex( );

        if ( mutex_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "Mutex creation failed: " + error );
            retVal = false;
        }
    }

    if ( retVal && Mix_OpenAudio( audioRate, audioFormat, audioChannels, audioBuffers ) == -1 )
    {
        std::string error = Mix_GetError( );
        Logger::write( Logger::ZONE_WARNING, "SDL", "Audio initialize failed: " + error );
    }

    return retVal;

}


// Deinitialize SDL
bool SDL::deInitialize( )
{
    std::string error = SDL_GetError( );
    Logger::write( Logger::ZONE_INFO, "SDL", "DeInitializing" );

    Mix_CloseAudio( );
    Mix_Quit( );

    if ( mutex_ )
    {
        SDL_DestroyMutex(mutex_);
        mutex_ = NULL;
    }

    if ( renderer_ )
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = NULL;
    }

    if ( window_ )
    {
        SDL_DestroyWindow(window_);
        window_ = NULL;
    }

    SDL_ShowCursor( SDL_TRUE );

    SDL_Quit( );

    return true;
}


// Get the renderer
SDL_Renderer* SDL::getRenderer( )
{
    return renderer_;
}


// Get the mutex
SDL_mutex* SDL::getMutex( )
{
    return mutex_;
}


// Get the window
SDL_Window* SDL::getWindow( )
{
    return window_;
}


// Render a copy of a texture
bool SDL::renderCopy( SDL_Texture *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo, float scaleX, float scaleY )
{

    // Skip rendering if the object is invisible anyway
    if ( alpha == 0 )
		return true;

    SDL_Rect srcRect;
    SDL_Rect dstRect;
    SDL_Rect srcRectCopy;
    SDL_Rect dstRectCopy;
    double   imageScaleX;
    double   imageScaleY;

    dstRect.w = dest->w;
    dstRect.h = dest->h;

    if ( fullscreen_ )
    {
        dstRect.x = dest->x + (displayWidth_ - windowWidth_)/2;
        dstRect.y = dest->y + (displayHeight_ - windowHeight_)/2;
    }
    else
    {
        dstRect.x = dest->x;
        dstRect.y = dest->y;
    }

    // Create the base fields to check against the container.
    if (src)
    {
        srcRect.x = src->x;
        srcRect.y = src->y;
        srcRect.w = src->w;
        srcRect.h = src->h;
    }
    else
    {
        srcRect.x = 0;
        srcRect.y = 0;
        int w = 0;
        int h = 0;
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        srcRect.w = w;
        srcRect.h = h;
    }

    // Define the scale
    imageScaleX = (dstRect.w > 0) ? static_cast<double>( srcRect.w ) / static_cast<double>( dstRect.w ) : 0.0;
    imageScaleY = (dstRect.h > 0) ? static_cast<double>( srcRect.h ) / static_cast<double>( dstRect.h ) : 0.0;

    // Make a copy
    srcRectCopy.x = srcRect.x;
    srcRectCopy.y = srcRect.y;
    srcRectCopy.w = srcRect.w;
    srcRectCopy.h = srcRect.h;
    dstRectCopy.x = dstRect.x;
    dstRectCopy.y = dstRect.y;
    dstRectCopy.w = dstRect.w;
    dstRectCopy.h = dstRect.h;

    // If a container has been defined, limit the display to the container boundaries.
    if ( viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0 &&
         dstRectCopy.w           > 0 && dstRectCopy.h            > 0 )
    {

        // Correct if the image falls to the left of the container
        if ( dstRect.x < viewInfo.ContainerX )
        {
            dstRect.x = static_cast<int>( viewInfo.ContainerX );
            dstRect.w = dstRectCopy.w + dstRectCopy.x - dstRect.x;
            srcRect.x = srcRectCopy.x + srcRectCopy.w * (dstRect.x - dstRectCopy.x) / dstRectCopy.w;
        }

        // Correct if the image falls to the right of the container
        if ( (dstRectCopy.x + dstRectCopy.w) > (viewInfo.ContainerX + viewInfo.ContainerWidth) )
        {
            dstRect.w = static_cast<int>( viewInfo.ContainerX + viewInfo.ContainerWidth ) - dstRect.x;
        }

        // Correct if the image falls to the top of the container
        if ( dstRect.y < viewInfo.ContainerY )
        {
            dstRect.y = static_cast<int>( viewInfo.ContainerY );
            dstRect.h = dstRectCopy.h + dstRectCopy.y - dstRect.y;
            srcRect.y = srcRectCopy.y + srcRectCopy.h * (dstRect.y - dstRectCopy.y) / dstRectCopy.h;
        }

        // Correct if the image falls to the bottom of the container
        if ( (dstRectCopy.y + dstRectCopy.h) > (viewInfo.ContainerY + viewInfo.ContainerHeight) )
        {
            dstRect.h = static_cast<int>( viewInfo.ContainerY + viewInfo.ContainerHeight ) - dstRect.y;
        }

        // Define source width and height
        srcRect.w = static_cast<int>( dstRect.w * imageScaleX );
        srcRect.h = static_cast<int>( dstRect.h * imageScaleY );

    }

    dstRectCopy.x = dstRect.x;
    dstRectCopy.y = dstRect.y;
    dstRectCopy.w = dstRect.w;
    dstRectCopy.h = dstRect.h;

    dstRect.x = (int)(dstRect.x*scaleX);
    dstRect.y = (int)(dstRect.y*scaleY);
    dstRect.w = (int)(dstRect.w*scaleX);
    dstRect.h = (int)(dstRect.h*scaleY);

    SDL_SetTextureAlphaMod( texture, static_cast<char>( alpha * 255 ) );
    SDL_RenderCopyEx( getRenderer( ), texture, &srcRect, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_NONE );

    dstRect.x = dstRectCopy.x;
    dstRect.y = dstRectCopy.y;
    dstRect.w = dstRectCopy.w;
    dstRect.h = dstRectCopy.h;

    if ( viewInfo.Reflection == "top" )
    {
        dstRect.h = static_cast<unsigned int>( static_cast<float>(dstRect.h ) * viewInfo.ReflectionScale);
        dstRect.y = dstRect.y - dstRect.h - viewInfo.ReflectionDistance;
        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);
        SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_VERTICAL );
    }

    if ( viewInfo.Reflection == "bottom" )
    {
        dstRect.y = dstRect.y + dstRect.h + viewInfo.ReflectionDistance;
        dstRect.h = static_cast<unsigned int>( static_cast<float>(dstRect.h ) * viewInfo.ReflectionScale);
        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);
        SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_VERTICAL );
    }

    if ( viewInfo.Reflection == "left" )
    {
        dstRect.w = static_cast<unsigned int>( static_cast<float>(dstRect.w ) * viewInfo.ReflectionScale);
        dstRect.x = dstRect.x - dstRect.w - viewInfo.ReflectionDistance;
        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);
        SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_HORIZONTAL );
    }

    if ( viewInfo.Reflection == "right" )
    {
        dstRect.x = dstRect.x + dstRect.w + viewInfo.ReflectionDistance;
        dstRect.w = static_cast<unsigned int>( static_cast<float>(dstRect.w ) * viewInfo.ReflectionScale);
        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);
        SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_HORIZONTAL );
    }

    return true;
}
