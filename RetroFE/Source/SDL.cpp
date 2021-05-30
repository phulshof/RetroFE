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

std::vector<SDL_Window *>   SDL::window_;
std::vector<SDL_Renderer *> SDL::renderer_;
SDL_mutex                  *SDL::mutex_ = NULL;
std::vector<int>            SDL::displayWidth_;
std::vector<int>            SDL::displayHeight_;
std::vector<int>            SDL::windowWidth_;
std::vector<int>            SDL::windowHeight_;
std::vector<bool>           SDL::fullscreen_;
std::vector<int>            SDL::rotation_;
std::vector<bool>           SDL::mirror_;
int                         SDL::numScreens_ = 1;
int                         SDL::numDisplays_ = 1;


// Initialize SDL
bool SDL::initialize( Configuration &config )
{

    int         audioRate     = MIX_DEFAULT_FREQUENCY;
    Uint16      audioFormat   = MIX_DEFAULT_FORMAT; /* 16-bit stereo */
    int         audioChannels = 1;
    int         audioBuffers  = 4096;
    bool        hideMouse;

    Logger::write( Logger::ZONE_INFO, "SDL", "Initializing" );
    if ( SDL_Init( SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS ) != 0 )
    {
        std::string error = SDL_GetError( );
        Logger::write( Logger::ZONE_ERROR, "SDL", "Initialize failed: " + error );
        return false;
    }

    if ( config.getProperty( "hideMouse", hideMouse ) )
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
    config.getProperty( "numScreens", numScreens_ );

    if ( numScreens_ <= 0 )
    {
        Logger::write( Logger::ZONE_ERROR, "SDL", "Number of requested displays < 0." );
        return false;
    }

    numDisplays_ = SDL_GetNumVideoDisplays( );
    Logger::write( Logger::ZONE_INFO, "SDL", "Number of displays found: " + std::to_string( numDisplays_ ) );
    Logger::write( Logger::ZONE_INFO, "SDL", "Number of displays requested: " + std::to_string( numScreens_ ) );

    // Preset the SDL settings for all monitors
    for (int i = 0; i < numScreens_; ++i)
    {

        SDL_DisplayMode mode;
        Uint32          windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;

        int screenNum = 0;
        if ( !config.getProperty( "screenNum" + std::to_string( i ), screenNum ) && i != 0 )
        {
            Logger::write( Logger::ZONE_ERROR, "SDL", "screenNum" + std::to_string(i) + " parameter not defined.");
            return false;
        }

        if ( SDL_GetCurrentDisplayMode( screenNum, &mode ) != 0 )
        {
            if ( i == 0 )
            {
                Logger::write( Logger::ZONE_ERROR, "SDL", "Display " + std::to_string(screenNum) + " does not exist.");
                return false;
            }
            else
            {
                Logger::write( Logger::ZONE_WARNING, "SDL", "Display " + std::to_string(screenNum) + " does not exist.");
                windowWidth_.push_back(0);
                windowHeight_.push_back(0);
                displayWidth_.push_back(0);
                displayHeight_.push_back(0);
                window_.push_back(NULL);
                renderer_.push_back(NULL);
            }
        }
        else
        {
            windowWidth_.push_back(mode.w);
            displayWidth_.push_back(mode.w);
            std::string hString = "";
            if ( i == 0 )
                config.getProperty( "horizontal", hString );
            config.getProperty( "horizontal" + std::to_string(i), hString );
            if ( hString == "" )
            {
                Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property \"horizontal\"" + std::to_string(i) );
                return false;
            }
            else if ( hString != "stretch" && (i == 0 && !config.getProperty( "horizontal", windowWidth_[i] )) && !config.getProperty( "horizontal" + std::to_string(i), windowWidth_[i] ))
            {
                Logger::write( Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"horizontal\"" + std::to_string(i) );
                return false;
            }

            windowHeight_.push_back(mode.h);
            displayHeight_.push_back(mode.h);
            std::string vString = "";
            if ( i == 0 )
                config.getProperty( "vertical", vString );
            config.getProperty( "vertical" + std::to_string(i), vString );
            if ( vString == "" )
            {
                Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property \"vertical\"" + std::to_string(i) );
                return false;
            }
            else if ( vString != "stretch" && (i == 0 && !config.getProperty( "vertical", windowHeight_[i] )) && !config.getProperty( "vertical" + std::to_string(i), windowHeight_[i] ) )
            {
                Logger::write( Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"vertical\"" + std::to_string(i) );
                return false;
            }


            bool fullscreen = false;
            config.getProperty( "fullscreen", fullscreen );
            if ( i == 0 && !config.getProperty( "fullscreen", fullscreen ) && !config.getProperty( "fullscreen" + std::to_string(i), fullscreen ) )
            {
                Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property: \"fullscreen\"" + std::to_string(i) );
                return false;
            }
            fullscreen_.push_back( fullscreen );

            if ( fullscreen_[i] )
            {
#ifdef WIN32
                windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
                windowFlags |= SDL_WINDOW_FULLSCREEN;
#endif
            }

            int rotation= 0;
            config.getProperty( "rotation" + std::to_string(i), rotation );
            Logger::write( Logger::ZONE_INFO, "Configuration", "Setting rotation for screen " + std::to_string(i) + " to " + std::to_string( rotation * 90 ) + " degrees." );
            rotation_.push_back( rotation );

            bool mirror = false;
            config.getProperty( "mirror" + std::to_string(i), mirror );
            if ( mirror )
                Logger::write( Logger::ZONE_INFO, "Configuration", "Setting mirror mode for screen " + std::to_string(i) + "." );
            mirror_.push_back( mirror );

            window_.push_back(NULL);
            renderer_.push_back(NULL);
            std::string fullscreenStr = fullscreen_[i] ? "yes" : "no";
            std::stringstream ss;
            ss << "Creating "<< windowWidth_[i] << "x" << windowHeight_[i] << " window (fullscreen: " << fullscreenStr << ")" << " on display " << screenNum;
            Logger::write( Logger::ZONE_INFO, "SDL", ss.str( ));
            window_[i] = SDL_CreateWindow( "RetroFE", SDL_WINDOWPOS_CENTERED_DISPLAY(screenNum), SDL_WINDOWPOS_CENTERED_DISPLAY(screenNum), windowWidth_[i], windowHeight_[i], windowFlags );

            if ( window_[i] == NULL )
            {
                std::string error = SDL_GetError( );
                if ( i == 0 )
                {
                    Logger::write( Logger::ZONE_ERROR, "SDL", "Create window " + std::to_string(i) + " on display " + std::to_string(screenNum) + " failed: " + error );
                    return false;
                }
                else
                {
                    Logger::write( Logger::ZONE_WARNING, "SDL", "Create window " + std::to_string(i) + " on display " + std::to_string(screenNum) + " failed: " + error );
                }
            }
            else
            {
                renderer_[i] = SDL_CreateRenderer( window_[i], -1, SDL_RENDERER_ACCELERATED );
                if ( renderer_[i] == NULL )
                {
                    std::string error = SDL_GetError( );
                    Logger::write( Logger::ZONE_ERROR, "SDL", "Create renderer " + std::to_string(i) + " failed: " + error );
                    return false;
                }
            }
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

    mutex_ = SDL_CreateMutex( );

    if ( mutex_ == NULL )
    {
        std::string error = SDL_GetError( );
        Logger::write( Logger::ZONE_ERROR, "SDL", "Mutex creation failed: " + error );
        return false;
    }

    if ( Mix_OpenAudio( audioRate, audioFormat, audioChannels, audioBuffers ) == -1 )
    {
        std::string error = Mix_GetError( );
        Logger::write( Logger::ZONE_WARNING, "SDL", "Audio initialize failed: " + error );
    }

    return true;

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

    for ( int i = 0; i < numScreens_; ++i )
    {
        if ( renderer_.size( ) > 0 )
        {
            if ( renderer_[0] )
            {
                SDL_DestroyRenderer( renderer_[0] );
            }
            renderer_.erase( renderer_.begin( ) );
        }

        if ( window_.size( ) > 0 )
        {
            if ( window_[0] )
            {
                SDL_DestroyWindow( window_[0] );
            }
            window_.erase( window_.begin( ) );
        }
    }
    displayWidth_.clear( );
    displayHeight_.clear( );
    windowWidth_.clear( );
    windowHeight_.clear( );
    fullscreen_.clear( );

    SDL_ShowCursor( SDL_TRUE );

    SDL_Quit( );

    return true;
}


// Get the renderer
SDL_Renderer* SDL::getRenderer( int index )
{
    return (index < numDisplays_ ? renderer_[index] : renderer_[0]);
}


// Get the mutex
SDL_mutex* SDL::getMutex( )
{
    return mutex_;
}


// Get the window
SDL_Window* SDL::getWindow( int index )
{
    return (index < numDisplays_ ? window_[index] : window_[0]);
}


// Render a copy of a texture
bool SDL::renderCopy( SDL_Texture *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo, int layoutWidth, int layoutHeight )
{

    // Skip rendering if the object is invisible anyway or if renderer does not exist
    if ( alpha == 0 || viewInfo.Monitor >= numScreens_ || !renderer_[viewInfo.Monitor] )
        return true;

    float scaleX = (float)windowWidth_[viewInfo.Monitor]  / (float)layoutWidth;
    float scaleY = (float)windowHeight_[viewInfo.Monitor] / (float)layoutHeight;

    if ( rotation_[viewInfo.Monitor] % 2 == 1 ) // 90 or 270 degree rotation; change scale factors
    {
        scaleX = (float)windowHeight_[viewInfo.Monitor] / (float)layoutWidth;
        scaleY = (float)windowWidth_[viewInfo.Monitor]  / (float)layoutHeight;
    }

    if ( mirror_[viewInfo.Monitor] )
        scaleY /= 2;

    // Don't print outside the screen in mirror mode
    if ( mirror_[viewInfo.Monitor] && (viewInfo.ContainerWidth < 0 || viewInfo.ContainerHeight < 0) )
    {
        viewInfo.ContainerX      = 0;
        viewInfo.ContainerY      = 0;
        viewInfo.ContainerWidth  = static_cast<float>( layoutWidth );
        viewInfo.ContainerHeight = static_cast<float>( layoutHeight );
    }

    SDL_Rect srcRect;
    SDL_Rect dstRect;
    SDL_Rect srcRectCopy;
    SDL_Rect dstRectCopy;
    SDL_Rect srcRectOrig;
    SDL_Rect dstRectOrig;
    double   imageScaleX;
    double   imageScaleY;

    dstRect.w = dest->w;
    dstRect.h = dest->h;

    if ( fullscreen_[viewInfo.Monitor] )
    {
        dstRect.x = dest->x + (displayWidth_[viewInfo.Monitor] - windowWidth_[viewInfo.Monitor])/2;
        dstRect.y = dest->y + (displayHeight_[viewInfo.Monitor] - windowHeight_[viewInfo.Monitor])/2;
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

    // Make two copies
    srcRectOrig.x = srcRect.x;
    srcRectOrig.y = srcRect.y;
    srcRectOrig.w = srcRect.w;
    srcRectOrig.h = srcRect.h;
    dstRectOrig.x = dstRect.x;
    dstRectOrig.y = dstRect.y;
    dstRectOrig.w = dstRect.w;
    dstRectOrig.h = dstRect.h;

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

    double angle = viewInfo.Angle;
    if ( !mirror_[viewInfo.Monitor] )
        angle += rotation_[viewInfo.Monitor] * 90;

    dstRect.x = (int)(dstRect.x*scaleX);
    dstRect.y = (int)(dstRect.y*scaleY);
    dstRect.w = (int)(dstRect.w*scaleX);
    dstRect.h = (int)(dstRect.h*scaleY);

    if ( mirror_[viewInfo.Monitor] )
    {
        if ( rotation_[viewInfo.Monitor] % 2 == 0 )
        {
            if ( srcRect.h > 0 && srcRect.w > 0 )
            {
                dstRect.y += windowHeight_[viewInfo.Monitor] / 2;
                SDL_SetTextureAlphaMod( texture, static_cast<char>( alpha * 255 ) );
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE );
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                angle    += 180;
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE );
            }
        }
        else
        {
            if ( srcRect.h > 0 && srcRect.w > 0 )
            {
                int tmp   = dstRect.x;
                dstRect.x = windowWidth_[viewInfo.Monitor]/2 - dstRect.y - dstRect.h/2 - dstRect.w/2;
                dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
                angle    += 90;
                SDL_SetTextureAlphaMod( texture, static_cast<char>( alpha * 255 ) );
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE );
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                angle    += 180;
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE );
            }
        }
    }
    else
    {
        if ( rotation_[viewInfo.Monitor] == 1 ) // 90 degree rotation
        {
            int tmp   = dstRect.x;
            dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.y - dstRect.h/2 - dstRect.w/2;
            dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
        }
        if ( rotation_[viewInfo.Monitor] == 2 ) // 180 degree rotation
        {
            dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
            dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
        }
        if ( rotation_[viewInfo.Monitor] == 3 ) // 270 degree rotation
        {
            int tmp   = dstRect.x;
            dstRect.x = dstRect.y + dstRect.h/2 - dstRect.w/2;
            dstRect.y = windowHeight_[viewInfo.Monitor] - tmp - dstRect.h/2 - dstRect.w/2;
        }
    
        if ( srcRect.h > 0 && srcRect.w > 0 )
        {
            SDL_SetTextureAlphaMod( texture, static_cast<char>( alpha * 255 ) );
            SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE );
        }
    }

    // Restore original parameters
    srcRect.x     = srcRectOrig.x;
    srcRect.y     = srcRectOrig.y;
    srcRect.w     = srcRectOrig.w;
    srcRect.h     = srcRectOrig.h;
    dstRect.x     = dstRectOrig.x;
    dstRect.y     = dstRectOrig.y;
    dstRect.w     = dstRectOrig.w;
    dstRect.h     = dstRectOrig.h;
    srcRectCopy.x = srcRectOrig.x;
    srcRectCopy.y = srcRectOrig.y;
    srcRectCopy.w = srcRectOrig.w;
    srcRectCopy.h = srcRectOrig.h;
    dstRectCopy.x = dstRectOrig.x;
    dstRectCopy.y = dstRectOrig.y;
    dstRectCopy.w = dstRectOrig.w;
    dstRectCopy.h = dstRectOrig.h;

    if ( viewInfo.Reflection.find("top") != std::string::npos )
    {
        dstRect.h     = static_cast<unsigned int>( static_cast<float>(dstRect.h ) * viewInfo.ReflectionScale);
        dstRect.y     = dstRect.y - dstRect.h - viewInfo.ReflectionDistance;
        imageScaleY   = (dstRect.h > 0) ? static_cast<double>( srcRect.h ) / static_cast<double>( dstRect.h ) : 0.0;
        dstRectCopy.y = dstRect.y;
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
            }

            // Correct if the image falls to the bottom of the container
            if ( (dstRectCopy.y + dstRectCopy.h) > (viewInfo.ContainerY + viewInfo.ContainerHeight) )
            {
                dstRect.h = static_cast<int>( viewInfo.ContainerY + viewInfo.ContainerHeight ) - dstRect.y;
                srcRect.y = srcRectCopy.y + srcRectCopy.h * (dstRectCopy.h - dstRect.h) / dstRectCopy.h;
            }

            // Define source width and height
            srcRect.w = static_cast<int>( dstRect.w * imageScaleX );
            srcRect.h = static_cast<int>( dstRect.h * imageScaleY );

        }

        angle = viewInfo.Angle;
        if ( !mirror_[viewInfo.Monitor] )
            angle += rotation_[viewInfo.Monitor] * 90;

        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);

        if ( mirror_[viewInfo.Monitor] )
        {
            if ( rotation_[viewInfo.Monitor] % 2 == 0 )
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    dstRect.y += windowHeight_[viewInfo.Monitor] / 2;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                }
            }
            else
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    int tmp   = dstRect.x;
                    dstRect.x = windowWidth_[viewInfo.Monitor]/2 - dstRect.y - dstRect.h/2 - dstRect.w/2;
                    dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
                    angle    += 90;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                }
            }
        }
        else
        {
            if ( rotation_[viewInfo.Monitor] == 1 ) // 90 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.y - dstRect.h/2 - dstRect.w/2;
                dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
            }
            if ( rotation_[viewInfo.Monitor] == 2 ) // 180 degree rotation
            {
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
            }
            if ( rotation_[viewInfo.Monitor] == 3 ) // 270 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = dstRect.y + dstRect.h/2 - dstRect.w/2;
                dstRect.y = windowHeight_[viewInfo.Monitor] - tmp - dstRect.h/2 - dstRect.w/2;
            }
        
            if ( srcRect.h > 0 && srcRect.w > 0 )
            {
                SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
            }
        }
    }

    // Restore original parameters
    srcRect.x     = srcRectOrig.x;
    srcRect.y     = srcRectOrig.y;
    srcRect.w     = srcRectOrig.w;
    srcRect.h     = srcRectOrig.h;
    dstRect.x     = dstRectOrig.x;
    dstRect.y     = dstRectOrig.y;
    dstRect.w     = dstRectOrig.w;
    dstRect.h     = dstRectOrig.h;
    srcRectCopy.x = srcRectOrig.x;
    srcRectCopy.y = srcRectOrig.y;
    srcRectCopy.w = srcRectOrig.w;
    srcRectCopy.h = srcRectOrig.h;
    dstRectCopy.x = dstRectOrig.x;
    dstRectCopy.y = dstRectOrig.y;
    dstRectCopy.w = dstRectOrig.w;
    dstRectCopy.h = dstRectOrig.h;

    if ( viewInfo.Reflection.find("bottom") != std::string::npos )
    {
        dstRect.y     = dstRect.y + dstRect.h + viewInfo.ReflectionDistance;
        dstRect.h     = static_cast<unsigned int>( static_cast<float>(dstRect.h ) * viewInfo.ReflectionScale);
        imageScaleY   = (dstRect.h > 0) ? static_cast<double>( srcRect.h ) / static_cast<double>( dstRect.h ) : 0.0;
        dstRectCopy.y = dstRect.y;
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
            }

            // Correct if the image falls to the bottom of the container
            if ( (dstRectCopy.y + dstRectCopy.h) > (viewInfo.ContainerY + viewInfo.ContainerHeight) )
            {
                dstRect.h = static_cast<int>( viewInfo.ContainerY + viewInfo.ContainerHeight ) - dstRect.y;
                srcRect.y = srcRectCopy.y + srcRectCopy.h * (dstRectCopy.h - dstRect.h) / dstRectCopy.h;
            }

            // Define source width and height
            srcRect.w = static_cast<int>( dstRect.w * imageScaleX );
            srcRect.h = static_cast<int>( dstRect.h * imageScaleY );

        }

        angle = viewInfo.Angle;
        if ( !mirror_[viewInfo.Monitor] )
            angle += rotation_[viewInfo.Monitor] * 90;

        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);

        if ( mirror_[viewInfo.Monitor] )
        {
            if ( rotation_[viewInfo.Monitor] % 2 == 0 )
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    dstRect.y += windowHeight_[viewInfo.Monitor] / 2;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                }
            }
            else
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    int tmp   = dstRect.x;
                    dstRect.x = windowWidth_[viewInfo.Monitor]/2 - dstRect.y - dstRect.h/2 - dstRect.w/2;
                    dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
                    angle    += 90;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
                }
            }
        }
        else
        {
            if ( rotation_[viewInfo.Monitor] == 1 ) // 90 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.y - dstRect.h/2 - dstRect.w/2;
                dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
            }
            if ( rotation_[viewInfo.Monitor] == 2 ) // 180 degree rotation
            {
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
            }
            if ( rotation_[viewInfo.Monitor] == 3 ) // 270 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = dstRect.y + dstRect.h/2 - dstRect.w/2;
                dstRect.y = windowHeight_[viewInfo.Monitor] - tmp - dstRect.h/2 - dstRect.w/2;
            }
        
            if ( srcRect.h > 0 && srcRect.w > 0 )
            {
                SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_VERTICAL );
            }
        }
    }

    // Restore original parameters
    srcRect.x     = srcRectOrig.x;
    srcRect.y     = srcRectOrig.y;
    srcRect.w     = srcRectOrig.w;
    srcRect.h     = srcRectOrig.h;
    dstRect.x     = dstRectOrig.x;
    dstRect.y     = dstRectOrig.y;
    dstRect.w     = dstRectOrig.w;
    dstRect.h     = dstRectOrig.h;
    srcRectCopy.x = srcRectOrig.x;
    srcRectCopy.y = srcRectOrig.y;
    srcRectCopy.w = srcRectOrig.w;
    srcRectCopy.h = srcRectOrig.h;
    dstRectCopy.x = dstRectOrig.x;
    dstRectCopy.y = dstRectOrig.y;
    dstRectCopy.w = dstRectOrig.w;
    dstRectCopy.h = dstRectOrig.h;

    if ( viewInfo.Reflection.find("left") != std::string::npos )
    {
        dstRect.w     = static_cast<unsigned int>( static_cast<float>(dstRect.w ) * viewInfo.ReflectionScale);
        dstRect.x     = dstRect.x - dstRect.w - viewInfo.ReflectionDistance;
        imageScaleX   = (dstRect.h > 0) ? static_cast<double>( srcRect.w ) / static_cast<double>( dstRect.w ) : 0.0;
        dstRectCopy.x = dstRect.x;
        dstRectCopy.w = dstRect.w;

        // If a container has been defined, limit the display to the container boundaries.
        if ( viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0 &&
             dstRectCopy.w           > 0 && dstRectCopy.h            > 0 )
        {

            // Correct if the image falls to the left of the container
            if ( dstRect.x < viewInfo.ContainerX )
            {
                dstRect.x = static_cast<int>( viewInfo.ContainerX );
                dstRect.w = dstRectCopy.w + dstRectCopy.x - dstRect.x;
            }

            // Correct if the image falls to the right of the container
            if ( (dstRectCopy.x + dstRectCopy.w) > (viewInfo.ContainerX + viewInfo.ContainerWidth) )
            {
                dstRect.w = static_cast<int>( viewInfo.ContainerX + viewInfo.ContainerWidth ) - dstRect.x;
                srcRect.x = srcRectCopy.x + srcRectCopy.w * (dstRectCopy.w - dstRect.w) / dstRectCopy.w;
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

        angle = viewInfo.Angle;
        if ( !mirror_[viewInfo.Monitor] )
            angle += rotation_[viewInfo.Monitor] * 90;

        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);

        if ( mirror_[viewInfo.Monitor] )
        {
            if ( rotation_[viewInfo.Monitor] % 2 == 0 )
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    dstRect.y += windowHeight_[viewInfo.Monitor] / 2;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                }
            }
            else
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    int tmp   = dstRect.x;
                    dstRect.x = windowWidth_[viewInfo.Monitor]/2 - dstRect.y - dstRect.h/2 - dstRect.w/2;
                    dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
                    angle    += 90;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                }
            }
        }
        else
        {
            if ( rotation_[viewInfo.Monitor] == 1 ) // 90 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.y - dstRect.h/2 - dstRect.w/2;
                dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
            }
            if ( rotation_[viewInfo.Monitor] == 2 ) // 180 degree rotation
            {
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
            }
            if ( rotation_[viewInfo.Monitor] == 3 ) // 270 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = dstRect.y + dstRect.h/2 - dstRect.w/2;
                dstRect.y = windowHeight_[viewInfo.Monitor] - tmp - dstRect.h/2 - dstRect.w/2;
            }
        
            if ( srcRect.h > 0 && srcRect.w > 0 )
            {
                SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
            }
        }
    }

    // Restore original parameters
    srcRect.x     = srcRectOrig.x;
    srcRect.y     = srcRectOrig.y;
    srcRect.w     = srcRectOrig.w;
    srcRect.h     = srcRectOrig.h;
    dstRect.x     = dstRectOrig.x;
    dstRect.y     = dstRectOrig.y;
    dstRect.w     = dstRectOrig.w;
    dstRect.h     = dstRectOrig.h;
    srcRectCopy.x = srcRectOrig.x;
    srcRectCopy.y = srcRectOrig.y;
    srcRectCopy.w = srcRectOrig.w;
    srcRectCopy.h = srcRectOrig.h;
    dstRectCopy.x = dstRectOrig.x;
    dstRectCopy.y = dstRectOrig.y;
    dstRectCopy.w = dstRectOrig.w;
    dstRectCopy.h = dstRectOrig.h;

    if ( viewInfo.Reflection.find("right") != std::string::npos )
    {
        dstRect.x     = dstRect.x + dstRect.w + viewInfo.ReflectionDistance;
        dstRect.w     = static_cast<unsigned int>( static_cast<float>(dstRect.w ) * viewInfo.ReflectionScale);
        imageScaleX   = (dstRect.h > 0) ? static_cast<double>( srcRect.w ) / static_cast<double>( dstRect.w ) : 0.0;
        dstRectCopy.x = dstRect.x;
        dstRectCopy.w = dstRect.w;

        // If a container has been defined, limit the display to the container boundaries.
        if ( viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0 &&
             dstRectCopy.w           > 0 && dstRectCopy.h            > 0 )
        {

            // Correct if the image falls to the left of the container
            if ( dstRect.x < viewInfo.ContainerX )
            {
                dstRect.x = static_cast<int>( viewInfo.ContainerX );
                dstRect.w = dstRectCopy.w + dstRectCopy.x - dstRect.x;
            }

            // Correct if the image falls to the right of the container
            if ( (dstRectCopy.x + dstRectCopy.w) > (viewInfo.ContainerX + viewInfo.ContainerWidth) )
            {
                dstRect.w = static_cast<int>( viewInfo.ContainerX + viewInfo.ContainerWidth ) - dstRect.x;
                srcRect.x = srcRectCopy.x + srcRectCopy.w * (dstRectCopy.w - dstRect.w) / dstRectCopy.w;
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

        angle = viewInfo.Angle;
        if ( !mirror_[viewInfo.Monitor] )
            angle += rotation_[viewInfo.Monitor] * 90;

        dstRect.x = (int)(dstRect.x*scaleX);
        dstRect.y = (int)(dstRect.y*scaleY);
        dstRect.w = (int)(dstRect.w*scaleX);
        dstRect.h = (int)(dstRect.h*scaleY);

        if ( mirror_[viewInfo.Monitor] )
        {
            if ( rotation_[viewInfo.Monitor] % 2 == 0 )
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    dstRect.y += windowHeight_[viewInfo.Monitor] / 2;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                }
            }
            else
            {
                if ( srcRect.h > 0 && srcRect.w > 0 )
                {
                    int tmp   = dstRect.x;
                    dstRect.x = windowWidth_[viewInfo.Monitor]/2 - dstRect.y - dstRect.h/2 - dstRect.w/2;
                    dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
                    angle    += 90;
                    SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                    dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                    dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
                    angle    += 180;
                    SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
                }
            }
        }
        else
        {
            if ( rotation_[viewInfo.Monitor] == 1 ) // 90 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.y - dstRect.h/2 - dstRect.w/2;
                dstRect.y = tmp - dstRect.h/2 + dstRect.w/2;
            }
            if ( rotation_[viewInfo.Monitor] == 2 ) // 180 degree rotation
            {
                dstRect.x = windowWidth_[viewInfo.Monitor] - dstRect.x - dstRect.w;
                dstRect.y = windowHeight_[viewInfo.Monitor] - dstRect.y - dstRect.h;
            }
            if ( rotation_[viewInfo.Monitor] == 3 ) // 270 degree rotation
            {
                int tmp   = dstRect.x;
                dstRect.x = dstRect.y + dstRect.h/2 - dstRect.w/2;
                dstRect.y = windowHeight_[viewInfo.Monitor] - tmp - dstRect.h/2 - dstRect.w/2;
            }
        
            if ( srcRect.h > 0 && srcRect.w > 0 )
            {
                SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
                SDL_RenderCopyEx( renderer_[viewInfo.Monitor], texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_HORIZONTAL );
            }
        }
    }

    return true;
}
