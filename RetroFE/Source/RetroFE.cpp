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


#include "RetroFE.h"
#include "Collection/CollectionInfoBuilder.h"
#include "Collection/CollectionInfo.h"
#include "Database/Configuration.h"
#include "Collection/Item.h"
#include "Execute/Launcher.h"
#include "Menu/Menu.h"
#include "Utility/Log.h"
#include "Utility/Utils.h"
#include "Collection/MenuParser.h"
#include "SDL.h"
#include "Control/UserInput.h"
#include "Graphics/PageBuilder.h"
#include "Graphics/Page.h"
#include "Graphics/Component/ScrollingList.h"
#include "Graphics/Component/Video.h"
#include <gst/gst.h>
#include "Video/VideoFactory.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <SDL2/SDL_ttf.h>

#if defined(__linux) || defined(__APPLE__)
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>
#endif

#ifdef WIN32
#include <Windows.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_thread.h>
#endif


RetroFE::RetroFE( Configuration &c )
    : initialized(false)
    , initializeError(false)
    , initializeThread(NULL)
    , config_(c)
    , db_(NULL)
    , metadb_(NULL)
    , input_(config_)
    , currentPage_(NULL)
    , keyInputDisable_(0)
    , currentTime_(0)
    , lastLaunchReturnTime_(0)
    , keyLastTime_(0)
    , keyDelayTime_(.3f)
    , reboot_(false)
    , kioskLock_(false)
    , paused_(false)
{
    menuMode_                            = false;
    attractMode_                         = false;
    attractModePlaylistCollectionNumber_ = 0;
    firstPlaylist_                       = "all"; // todo
}

#ifdef WIN32
void RetroFE::postMessage(LPCTSTR windowTitle, UINT Msg, WPARAM wParam, LPARAM lParam ) {
    HWND hwnd = FindWindow(NULL, windowTitle);
	if (hwnd != NULL) {
        PostMessage(hwnd, Msg, wParam, lParam);
    }
}
#endif

RetroFE::~RetroFE( )
{
    deInitialize( );
}


// Render the current page to the screen
void RetroFE::render( )
{

    SDL_LockMutex( SDL::getMutex( ) );
    for ( int i = 0; i < SDL::getScreenCount( ); ++i )
    {
        SDL_SetRenderDrawColor( SDL::getRenderer( i ), 0x0, 0x0, 0x00, 0xFF );
        SDL_RenderClear( SDL::getRenderer( i ) );
    }

    if ( currentPage_ )
    {
        currentPage_->draw( );
    }

    for ( int i = 0; i < SDL::getScreenCount( ); ++i )
    {
        SDL_RenderPresent( SDL::getRenderer( i ) );
    }
    SDL_UnlockMutex( SDL::getMutex( ) );

}


// Initialize the configuration and database
int RetroFE::initialize( void *context )
{

    RetroFE *instance = static_cast<RetroFE *>(context);

    Logger::write( Logger::ZONE_INFO, "RetroFE", "Initializing" );

    if ( !instance->input_.initialize( ) )
    {
        Logger::write( Logger::ZONE_ERROR, "RetroFE", "Could not initialize user controls" );
        instance->initializeError = true;
        return -1;
    }

    instance->db_ = new DB( Utils::combinePath( Configuration::absolutePath, "meta.db" ) );

    if ( !instance->db_->initialize( ) )
    {
        Logger::write( Logger::ZONE_ERROR, "RetroFE", "Could not initialize database" );
        instance->initializeError = true;
        return -1;
    }

    instance->metadb_ = new MetadataDatabase( *(instance->db_), instance->config_ );

    if ( !instance->metadb_->initialize( ) )
    {
        Logger::write( Logger::ZONE_ERROR, "RetroFE", "Could not initialize meta database" );
        instance->initializeError = true;
        return -1;
    }

    instance->initialized = true;
    return 0;

}


// Launch a game/program
void RetroFE::launchEnter( )
{

    // Disable window focus
    SDL_SetWindowGrab(SDL::getWindow( 0 ), SDL_FALSE);
    // Free the textures, and optionally take down SDL
    //freeGraphicsMemory();

    bool hideMouse = false;
    int  mouseX    = 5000;
    int  mouseY    = 5000;
    config_.getProperty( "hideMouse", hideMouse );
    config_.getProperty( "mouseX",    mouseX );
    config_.getProperty( "mouseY",    mouseY );
    if ( hideMouse )
        SDL_WarpMouseGlobal( mouseX, mouseY );
}


// Return from the launch of a game/program
void RetroFE::launchExit( )
{
    // Optionally set up SDL, and load the textures
    //allocateGraphicsMemory();

    // Restore the SDL settings
    SDL_RestoreWindow( SDL::getWindow( 0 ) );
    SDL_RaiseWindow( SDL::getWindow( 0 ) );
    SDL_SetWindowGrab( SDL::getWindow( 0 ), SDL_TRUE );

    // Empty event queue, but handle joystick add/remove events
    SDL_Event e;
    while ( SDL_PollEvent( &e ) )
    {
        if ( e.type == SDL_JOYDEVICEADDED || e.type == SDL_JOYDEVICEREMOVED )
        {
            input_.update( e );
        }
    }
    input_.resetStates( );
    attract_.reset( );

    // Restore time settings
    currentTime_ = static_cast<float>( SDL_GetTicks( ) ) / 1000;
    keyLastTime_ = currentTime_;
    lastLaunchReturnTime_ = currentTime_;

    bool hideMouse = false;
    int  mouseX    = 5000;
    int  mouseY    = 5000;
    config_.getProperty( "hideMouse", hideMouse );
    config_.getProperty( "mouseX",    mouseX );
    config_.getProperty( "mouseY",    mouseY );
    if ( hideMouse )
        SDL_WarpMouseGlobal( mouseX, mouseY );

}


// Free the textures, and optionall take down SDL
void RetroFE::freeGraphicsMemory( )
{

    // Free textures
    if ( currentPage_ )
    {
        currentPage_->freeGraphicsMemory( );
    }

    // Close down SDL
    bool unloadSDL = false;
    config_.getProperty( "unloadSDL", unloadSDL );
    if ( unloadSDL )
    {
        currentPage_->deInitializeFonts( );
        SDL::deInitialize( );
        input_.clearJoysticks( );
    }

}


// Optionally set up SDL, and load the textures
void RetroFE::allocateGraphicsMemory( )
{

    // Reopen SDL
    bool unloadSDL = false;
    config_.getProperty( "unloadSDL", unloadSDL );
    if ( unloadSDL )
    {
        SDL::initialize( config_ );
        currentPage_->initializeFonts( );
    }

    // Allocate textures
    if ( currentPage_ )
    {
        currentPage_->allocateGraphicsMemory( );
    }

}


// Deinitialize RetroFE
bool RetroFE::deInitialize( )
{

    bool retVal = true;

    // Free textures
    freeGraphicsMemory( );

    // Delete page
    if ( currentPage_ )
    {
        currentPage_->deInitialize( );
        delete currentPage_;
        currentPage_ = NULL;
    }

    // Delete databases
    if ( metadb_ )
    {
        delete metadb_;
        metadb_ = NULL;
    }

    if ( db_ )
    {
        delete db_;
        db_ = NULL;
    }

    initialized = false;

    if ( reboot_ )
    {
        Logger::write( Logger::ZONE_INFO, "RetroFE", "Rebooting" );
    }
    else
    {
        Logger::write( Logger::ZONE_INFO, "RetroFE", "Exiting" );
        SDL::deInitialize( );
        gst_deinit( );
    }

    return retVal;
}


// Run RetroFE
bool RetroFE::run( )
{

    // Initialize SDL
    if(! SDL::initialize( config_ ) ) return false;
    fontcache_.initialize( );

#ifdef WIN32
    bool highPriority = false;
    config_.getProperty("highPriority", highPriority);
    if (highPriority) {
        SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    }
#endif

    // Define control configuration
    std::string controlsConfPath = Utils::combinePath( Configuration::absolutePath, "controls" );
    config_.import("controls", controlsConfPath + ".conf");
    for (int i = 1; i < 10; i++)
        config_.import("controls", controlsConfPath + std::to_string(i) + ".conf", false);

    if (config_.propertiesEmpty())
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "No controls.conf found");

        return false;
    }

    float preloadTime = 0;

    // Initialize video
    bool videoEnable = true;
    int  videoLoop   = 0;
    config_.getProperty( "videoEnable", videoEnable );
    config_.getProperty( "videoLoop", videoLoop );
    VideoFactory::setEnabled( videoEnable );
    VideoFactory::setNumLoops( videoLoop );
    Video::setEnabled( videoEnable );

    initializeThread = SDL_CreateThread( initialize, "RetroFEInit", (void *)this );

    if (!initializeThread)
    {
        Logger::write( Logger::ZONE_INFO, "RetroFE", "Could not initialize RetroFE" );

        return false;
    }

    bool attractModeFast = false;
    int attractModeTime           = 0;
    int attractModeNextTime       = 0;
    int attractModePlaylistTime   = 0;
    int attractModeCollectionTime = 0;
    int attractModeMinTime        = 1000;
    int attractModeMaxTime        = 5000;
    std::string firstCollection   = "Main";
    bool running                  = true;
    RETROFE_STATE state           = RETROFE_NEW;

    config_.getProperty( "attractModeTime", attractModeTime );
    config_.getProperty( "attractModeNextTime", attractModeNextTime );
    config_.getProperty( "attractModePlaylistTime", attractModePlaylistTime );
    config_.getProperty( "attractModeCollectionTime", attractModeCollectionTime );
    config_.getProperty( "attractModeMinTime", attractModeMinTime );
    config_.getProperty( "attractModeMaxTime", attractModeMaxTime );
    config_.getProperty( "firstCollection", firstCollection );
    config_.getProperty("attractModeFast", attractModeFast);

    attract_.idleTime           = static_cast<float>(attractModeTime);
    attract_.idleNextTime       = static_cast<float>(attractModeNextTime);
    attract_.idlePlaylistTime   = static_cast<float>(attractModePlaylistTime);
    attract_.idleCollectionTime = static_cast<float>(attractModeCollectionTime);
    attract_.minTime            = attractModeMinTime;
    attract_.maxTime            = attractModeMaxTime;
    attract_.isFast = attractModeFast;

    int fps     = 60;
    int fpsIdle = 60;
    config_.getProperty( "fps", fps );
    config_.getProperty( "fpsIdle", fpsIdle );
    double fpsTime     = 1000.0 / static_cast<double>(fps);
    double fpsIdleTime = 1000.0 / static_cast<double>(fpsIdle);
	bool vSync = false;
	config_.getProperty("vSync", vSync);

    int initializeStatus = 0;
    bool inputClear      = false;

    // load the initial splash screen, unload it once it is complete
    currentPage_        = loadSplashPage( );
    state               = RETROFE_ENTER;
    bool splashMode     = true;
    bool exitSplashMode = false;

    Launcher l( config_ );
    Menu     m( config_, input_ );
    preloadTime = static_cast<float>( SDL_GetTicks( ) ) / 1000;
    l.LEDBlinky( 1 );
    config_.getProperty("kiosk", kioskLock_);

    while ( running )
    {

        float lastTime = 0;
        float deltaTime = 0;

        // Exit splash mode when an active key is pressed
        SDL_Event e;
        if ( splashMode && SDL_PollEvent( &e ) )
        {
            if (input_.update(e))
            {
                if (input_.keystate(UserInput::KeyCodeSelect)) {
                    exitSplashMode = true;
                    while (SDL_PollEvent(&e))
                    {
                        if (e.type == SDL_JOYDEVICEADDED || e.type == SDL_JOYDEVICEREMOVED)
                        {
                            input_.update(e);
                        }
                    }
                    input_.resetStates();
                    attract_.reset();
                }
                else if (input_.keystate(UserInput::KeyCodeQuit)) {
                    running = false;
                    break;
                }
            }
        }

        if ( !currentPage_ )
        {
            Logger::write( Logger::ZONE_WARNING, "RetroFE", "Could not load page"  );
            running = false;
            break;
        }

        switch(state)
        {

        // Idle state; waiting for input
        case RETROFE_IDLE:

            currentPage_->cleanup( );

            // Not in splash mode
            if ( currentPage_ && !splashMode )
            {
                // account for when returning from a menu and the previous key was still "stuck"
                if ( lastLaunchReturnTime_ == 0 || (currentTime_ - lastLaunchReturnTime_ > .3) )
                {
                    if ( currentPage_->isIdle( ) )
                    {
                        state = processUserInput( currentPage_ );
                    }
                    lastLaunchReturnTime_ = 0;
                }
            }

            // Handle end of splash mode
            if ( (initialized || initializeError) && splashMode && (exitSplashMode || (currentPage_->getMinShowTime( ) <= (currentTime_ - preloadTime) && !(currentPage_->isPlaying( )))) )
            {
                SDL_WaitThread( initializeThread, &initializeStatus );

                if ( initializeError )
                {
                    state = RETROFE_QUIT_REQUEST;
                    break;
                }

                currentPage_->stop( );
                state = RETROFE_SPLASH_EXIT;

            }

            break;

        // Load art on entering RetroFE
        case RETROFE_LOAD_ART:
            currentPage_->start( );
#ifdef WIN32            
			postMessage("MediaplayerHiddenWindow",0x8001, 50, 0);		
#endif			
            state = RETROFE_ENTER;
            break;

        // Wait for onEnter animation to finish
        case RETROFE_ENTER:
            if ( currentPage_->isIdle( ) )
            {
                bool startCollectionEnter = false;
                config_.getProperty( "startCollectionEnter", startCollectionEnter );
                nextPageItem_ = currentPage_->getSelectedItem( );
                if ( !splashMode && startCollectionEnter && !nextPageItem_->leaf )
                {
                    state = RETROFE_NEXT_PAGE_REQUEST;
                }
                else
                {
                    state = RETROFE_IDLE;
                }
            }
            break;

        // Handle end of splash mode
        case RETROFE_SPLASH_EXIT:
            if ( currentPage_->isIdle( ) )
            {
                // delete the splash screen and use the standard menu
                currentPage_->deInitialize( );
                delete currentPage_;

                // find first collection
                std::string firstCollection = "Main";
                config_.getProperty("firstCollection", firstCollection);
                currentPage_ = loadPage(firstCollection);
                currentPage_->setLocked(kioskLock_);
                splashMode = false;
                if ( currentPage_ )
                {
                    currentPage_->setLocked(kioskLock_);
                    CollectionInfo* info;

                    // add collections to cycle
                    std::string cycleString;
                    config_.getProperty("cycleCollection", cycleString);
                    Utils::listToVector(cycleString, collectionCycle_, ',');
                    collectionCycleIt_ = collectionCycle_.begin();

                    config_.setProperty( "currentCollection", firstCollection );
                    info = getCollection(firstCollection);

                    if (info == NULL) {
                        state = RETROFE_QUIT_REQUEST;

                        break;
                    }
                    currentPage_->pushCollection(info);

                    // check collection for setting
                    std::string settingPrefix = "collections." + currentPage_->getCollectionName() + ".";
                    if (config_.propertyExists(settingPrefix + "firstPlaylist")) {
                        config_.getProperty(settingPrefix + "firstPlaylist", firstPlaylist_);
                    }
                    else {
                        config_.getProperty("firstPlaylist", firstPlaylist_);
                    }

                    currentPage_->selectPlaylist( firstPlaylist_ );
                    if (currentPage_->getPlaylistName() != firstPlaylist_ )
                        currentPage_->selectPlaylist( "all" );

                    currentPage_->onNewItemSelected();
                    currentPage_->reallocateMenuSpritePoints(); // update playlist menu

                    state = RETROFE_LOAD_ART;
                }
                else
                {
                    state = RETROFE_QUIT_REQUEST;
                }
            }
            break;

        // Switch playlist; start onHighlightExit animation
        case RETROFE_PLAYLIST_REQUEST:
            inputClear = false;
            config_.getProperty( "playlistInputClear", inputClear );
            if (  inputClear  )
            {
                // Empty event queue
                SDL_Event e;
                while ( SDL_PollEvent( &e ) )
                    input_.update(e);
                input_.resetStates( );
            }
            currentPage_->playlistExit( );
            currentPage_->setScrolling(Page::ScrollDirectionIdle);
            state = RETROFE_PLAYLIST_EXIT;
            break;

        // Switch playlist; wait for onHighlightExit animation to finish; load art
        case RETROFE_PLAYLIST_EXIT:
            if (currentPage_->isIdle( ))
            {
                // lots of different toggles and menu jumps trigger this by accident
                if (currentPage_->fromPlaylistNav) {
                    if (currentPage_->fromPreviousPlaylist) {
                        currentPage_->playlistPrevExit();
                    }
                    else {
                        currentPage_->playlistNextExit();
                    }
                }
                bool rememberMenu = false;
                config_.getProperty("rememberMenu", rememberMenu);
                if (rememberMenu && currentPage_->getPlaylistName() != "lastplayed") {
                    currentPage_->returnToRememberSelectedItem();
                }
                state = RETROFE_PLAYLIST_LOAD_ART;
            }
            break;

        // Switch playlist; start onHighlightEnter animation
        case RETROFE_PLAYLIST_LOAD_ART:
            if (currentPage_->isIdle( ))
            {
                currentPage_->onNewItemSelected();
                currentPage_->reallocateMenuSpritePoints(); // update playlist menu
                currentPage_->playlistEnter();
                state = RETROFE_PLAYLIST_ENTER;
            }
            break;

        // Switch playlist; wait for onHighlightEnter animation to finish
        case RETROFE_PLAYLIST_ENTER:
            if (currentPage_->isIdle( ))
            {
                state = RETROFE_IDLE;
            }
            break;

        // Jump in menu; start onMenuJumpExit animation
        case RETROFE_MENUJUMP_REQUEST:
            inputClear = false;
            config_.getProperty( "jumpInputClear", inputClear );
            if (  inputClear  )
            {
                // Empty event queue
                SDL_Event e;
                while ( SDL_PollEvent( &e ) )
                    input_.update(e);
                input_.resetStates( );
            }
            currentPage_->menuJumpExit( );
            currentPage_->setScrolling(Page::ScrollDirectionIdle);
            state = RETROFE_MENUJUMP_EXIT;
            break;

        // Jump in menu; wait for onMenuJumpExit animation to finish; load art
        case RETROFE_MENUJUMP_EXIT:
            if (currentPage_->isIdle( ))
            {
                state = RETROFE_MENUJUMP_LOAD_ART;
            }
            break;

        // Jump in menu; start onMenuJumpEnter animation
        case RETROFE_MENUJUMP_LOAD_ART:
            if (currentPage_->isIdle( ))
            {
                currentPage_->onNewItemSelected();
                currentPage_->reallocateMenuSpritePoints(false); // skip updating playlist menu
                currentPage_->menuJumpEnter( );
                state = RETROFE_MENUJUMP_ENTER;
            }
            break;

        // Jump in menu; wait for onMenuJump animation to finish
        case RETROFE_MENUJUMP_ENTER:
            if (currentPage_->isIdle( ))
            {
                state = RETROFE_IDLE;
            }
            break;

        // Start onHighlightExit animation
        case RETROFE_HIGHLIGHT_REQUEST:
            currentPage_->setScrolling(Page::ScrollDirectionIdle);
            currentPage_->highlightExit( );
            state = RETROFE_HIGHLIGHT_EXIT;
            break;

        // Wait for onHighlightExit animation to finish; load art
        case RETROFE_HIGHLIGHT_EXIT:
            if (currentPage_->isIdle( ))
            {
                currentPage_->highlightLoadArt( );
                state = RETROFE_HIGHLIGHT_LOAD_ART;
            }
            break;

        // Start onHighlightEnter animation
        case RETROFE_HIGHLIGHT_LOAD_ART:
            currentPage_->highlightEnter( );
            if ( currentPage_->getSelectedItem( ) )
                l.LEDBlinky( 9, currentPage_->getSelectedItem( )->collectionInfo->name, currentPage_->getSelectedItem( ) );
            state = RETROFE_HIGHLIGHT_ENTER;
            break;

        // Wait for onHighlightEnter animation to finish
        case RETROFE_HIGHLIGHT_ENTER:
            RETROFE_STATE state_tmp;
            if (currentPage_->isMenuIdle( ) &&
                ((state_tmp = processUserInput( currentPage_ )) == RETROFE_HIGHLIGHT_REQUEST ||
                  state_tmp                                     == RETROFE_MENUJUMP_REQUEST  ||
                  state_tmp                                     == RETROFE_PLAYLIST_REQUEST) )
            {
                state = state_tmp;
            }
            else if (currentPage_->isIdle( ))
            {
                state = RETROFE_IDLE;
            }
            break;

        // Next page; start onMenuExit animation
        case RETROFE_NEXT_PAGE_REQUEST:
            currentPage_->exitMenu( );
            state = RETROFE_NEXT_PAGE_MENU_EXIT;
            break;

        // Wait for onMenuExit animation to finish; load new page if applicable; load art
        case RETROFE_NEXT_PAGE_MENU_EXIT:
            if ( currentPage_->isIdle( ) )
            {
                std::string collectionName = currentPage_->getCollectionName();
                if ( currentPage_->getSelectedItem( ) )
                    l.LEDBlinky( 8, currentPage_->getSelectedItem( )->name, currentPage_->getSelectedItem( ) );
                lastMenuOffsets_[collectionName] = currentPage_->getScrollOffsetIndex();
                lastMenuPlaylists_[collectionName] = currentPage_->getPlaylistName( );
                std::string nextPageName = nextPageItem_->name;
                if ( !menuMode_ )
                {
                    // Load new layout if available
                    std::string layoutName;
                    config_.getProperty( "layout", layoutName );
                    PageBuilder pb( layoutName, getLayoutFileName(), config_, &fontcache_ );
                    Page *page = pb.buildPage( nextPageItem_->name );
                    if ( page )
                    {
                        if (page->controlsType() != "") {
                            updatePageControls(page->controlsType());
                            page->setControlsType("");
                        }
                        currentPage_->freeGraphicsMemory( );
                        pages_.push( currentPage_ );
                        currentPage_ = page;
                        currentPage_->setLocked(kioskLock_);
                    }
                }

                config_.setProperty( "currentCollection", nextPageName );

                CollectionInfo *info;
                if ( menuMode_ )
                    info = getMenuCollection( nextPageName );
                else
                    info = getCollection( nextPageName );

                currentPage_->pushCollection(info);

                bool rememberMenu = false;
                config_.getProperty( "rememberMenu", rememberMenu );

                std::string autoPlaylist = "all";
                config_.getProperty( "autoPlaylist", autoPlaylist );
                bool returnToRememberedPlaylist = rememberMenu && lastMenuPlaylists_.find(nextPageName) != lastMenuPlaylists_.end();
                if (returnToRememberedPlaylist)
                {
                    currentPage_->selectPlaylist( lastMenuPlaylists_[nextPageName] ); // Switch to last playlist
                }
                else
                {
                    currentPage_->selectPlaylist( autoPlaylist );
                    if (currentPage_->getPlaylistName() != autoPlaylist )
                        currentPage_->selectPlaylist( "all" );
                }

                if (returnToRememberedPlaylist)
                {
                    if (lastMenuOffsets_.size() && lastMenuPlaylists_.find(nextPageName) != lastMenuPlaylists_.end()) {
                        currentPage_->setScrollOffsetIndex(lastMenuOffsets_[nextPageName]);
                    }
                }

                currentPage_->onNewItemSelected();
                currentPage_->reallocateMenuSpritePoints(); // update playlist menu

                state = RETROFE_NEXT_PAGE_MENU_LOAD_ART;

                // Check if we've entered an empty collection and need to go back automatically
                if (currentPage_->getCollectionSize() == 0)
                {
                    bool backOnEmpty = false;
                    config_.getProperty( "backOnEmpty", backOnEmpty );
                    if (backOnEmpty)
                        state = RETROFE_BACK_MENU_EXIT;
                }


             }
             break;

        // Start onMenuEnter animation
        case RETROFE_NEXT_PAGE_MENU_LOAD_ART:
            if (currentPage_->getMenuDepth( ) != 1 )
            {
                currentPage_->enterMenu( );
            }
            else
            {
                currentPage_->start( );
            }
            if ( currentPage_->getSelectedItem( ) )
                l.LEDBlinky( 9, currentPage_->getSelectedItem( )->collectionInfo->name, currentPage_->getSelectedItem( ) );
            state = RETROFE_NEXT_PAGE_MENU_ENTER;
            break;

        // Wait for onMenuEnter animation to finish
        case RETROFE_NEXT_PAGE_MENU_ENTER:
            if ( currentPage_->isIdle( ) )
            {
                inputClear = false;
                config_.getProperty( "collectionInputClear", inputClear );
                if (  inputClear  )
                {
                    // Empty event queue
                    SDL_Event e;
                    while ( SDL_PollEvent( &e ) )
                        input_.update(e);
                    input_.resetStates( );
                }
                state = RETROFE_IDLE;
            }
            break;

        // Start exit animation
        case RETROFE_COLLECTION_DOWN_REQUEST:
            if ( !pages_.empty( ) && currentPage_->getMenuDepth( ) == 1) // Inside a collection with a different layout
            {
                currentPage_->stop( );
                m.clearPage( );
                menuMode_ = false;
                state = RETROFE_COLLECTION_DOWN_EXIT;
            }
            else if ( currentPage_->getMenuDepth( ) > 1 ) // Inside a collection with the same layout
            {
                currentPage_->exitMenu( );
                state = RETROFE_COLLECTION_DOWN_EXIT;
            }
            else // Not in a collection
            {
                state = RETROFE_COLLECTION_DOWN_ENTER;

                if ( attractMode_ ) // Check playlist change in attract mode
                {
                    attractModePlaylistCollectionNumber_   += 1;
                    int attractModePlaylistCollectionNumber = 0;
                    config_.getProperty( "attractModePlaylistCollectionNumber", attractModePlaylistCollectionNumber );
                    // Check if playlist should be changed
                    if ( attractModePlaylistCollectionNumber_ > 0 && attractModePlaylistCollectionNumber_ >= attractModePlaylistCollectionNumber )
                    {
                        attractModePlaylistCollectionNumber_ = 0;
                        currentPage_->nextPlaylist( );
                      
                        if (isInAttractModeSkipPlaylist(currentPage_->getPlaylistName()))
                        {
                            //todo find next playlist that isn't in skip list
                            currentPage_->nextPlaylist();
                        }

                        state = RETROFE_PLAYLIST_REQUEST;
                    }
                }
            }
            break;

        // Wait for the menu exit animation to finish
        case RETROFE_COLLECTION_DOWN_EXIT:
            if ( currentPage_->isIdle( ) )
            {
                std::string collectionName = currentPage_->getCollectionName();
                lastMenuOffsets_[collectionName] = currentPage_->getScrollOffsetIndex();
                lastMenuPlaylists_[collectionName] = currentPage_->getPlaylistName( );
                if (currentPage_->getMenuDepth( ) == 1) // Inside a collection with a different layout
                {
                    currentPage_->deInitialize( );
                    delete currentPage_;
                    currentPage_ = pages_.top( );
                    pages_.pop( );
                    currentPage_->allocateGraphicsMemory( );
                    currentPage_->setLocked(kioskLock_);
                }
                else // Inside a collection with the same layout
                {
                    currentPage_->popCollection( );
                }
                collectionName = currentPage_->getCollectionName();
                config_.setProperty( "currentCollection", collectionName );

                bool rememberMenu = false;
                config_.getProperty( "rememberMenu", rememberMenu );

                std::string autoPlaylist = "all";
                config_.getProperty( "autoPlaylist", autoPlaylist );
                bool returnToRememberedPlaylist = rememberMenu && lastMenuPlaylists_.find(collectionName) != lastMenuPlaylists_.end();
                if (returnToRememberedPlaylist)
                {
                    currentPage_->selectPlaylist( lastMenuPlaylists_[collectionName] ); // Switch to last playlist
                }
                else
                {
                    currentPage_->selectPlaylist( autoPlaylist );
                    if (currentPage_->getPlaylistName() != autoPlaylist )
                        currentPage_->selectPlaylist( "all" );
                }

                if (returnToRememberedPlaylist)
                {
                    if (lastMenuOffsets_.size() && lastMenuPlaylists_.find(collectionName) != lastMenuPlaylists_.end()) {
                        currentPage_->setScrollOffsetIndex(lastMenuOffsets_[collectionName]);
                    }
                }

                state = RETROFE_COLLECTION_DOWN_MENU_ENTER;
                currentPage_->onNewItemSelected( );

                if ( attractMode_ ) // Check playlist change in attract mode
                {
                    attractModePlaylistCollectionNumber_   += 1;
                    int attractModePlaylistCollectionNumber = 0;
                    config_.getProperty( "attractModePlaylistCollectionNumber", attractModePlaylistCollectionNumber );
                    // Check if playlist should be changed
                    if ( attractModePlaylistCollectionNumber_ > 0 && attractModePlaylistCollectionNumber_ >= attractModePlaylistCollectionNumber )
                    {
                        attractModePlaylistCollectionNumber_ = 0;
                        currentPage_->nextPlaylist( );

                        if (isInAttractModeSkipPlaylist(currentPage_->getPlaylistName())) 
                        {
                            //todo find next playlist that isn't in skip list
                            currentPage_->nextPlaylist();
                        }

                        state = RETROFE_PLAYLIST_REQUEST;
                    }
                }
            }
            break;


        // Start menu enter animation
        case RETROFE_COLLECTION_DOWN_MENU_ENTER:
            currentPage_->enterMenu( );
            state = RETROFE_COLLECTION_DOWN_ENTER;
            break;
            

        // Waiting for enter animation to stop
        case RETROFE_COLLECTION_DOWN_ENTER:
            if ( currentPage_->isIdle( ) )
            {
                int attractModePlaylistCollectionNumber = 0;
                config_.getProperty( "attractModePlaylistCollectionNumber", attractModePlaylistCollectionNumber );
                if (!( attractMode_ && attractModePlaylistCollectionNumber > 0 && attractModePlaylistCollectionNumber_ == 0 ))
                {
                    currentPage_->setScrolling(Page::ScrollDirectionForward);
                    currentPage_->scroll(true);
                    currentPage_->updateScrollPeriod( );
                }
                state = RETROFE_COLLECTION_DOWN_SCROLL;
            }
            break;

        // Waiting for scrolling animation to stop
        case RETROFE_COLLECTION_DOWN_SCROLL:
            if ( currentPage_->isMenuIdle( ) )
            {
                std::string attractModeSkipCollection = "";
                config_.getProperty( "attractModeSkipCollection", attractModeSkipCollection );
                // Check if we need to skip this collection in attract mode or if we can select it
                if ( attractMode_ && currentPage_->getSelectedItem( )->name == attractModeSkipCollection )
                {
                    currentPage_->setScrolling(Page::ScrollDirectionForward);
                    currentPage_->scroll(true);
                    currentPage_->updateScrollPeriod( );
                }
                else
                {
                    RETROFE_STATE state_tmp = processUserInput( currentPage_ );
                    if ( state_tmp == RETROFE_COLLECTION_DOWN_REQUEST )
                    {
                        state = RETROFE_COLLECTION_DOWN_REQUEST;
                    }
                    else if ( state_tmp == RETROFE_COLLECTION_UP_REQUEST )
                    {
                        state = RETROFE_COLLECTION_UP_REQUEST;
                    }
                    else
                    {
                        currentPage_->setScrolling(Page::ScrollDirectionIdle); // Stop scrolling
                        nextPageItem_ = currentPage_->getSelectedItem( );
                        bool enterOnCollection = true;
                        config_.getProperty( "enterOnCollection", enterOnCollection );
                        if ( currentPage_->getSelectedItem( )->leaf || (!attractMode_ && !enterOnCollection) ) // Current selection is a game or enterOnCollection is not set
                        {
                            state = RETROFE_HIGHLIGHT_REQUEST;
                        }
                        else // Current selection is a menu
                        {
                            state = RETROFE_COLLECTION_HIGHLIGHT_EXIT;
                        }
                    }
                }
            }
            break;


        // Start onHighlightExit animation
        case RETROFE_COLLECTION_HIGHLIGHT_REQUEST:
            currentPage_->highlightExit( );
            state = RETROFE_COLLECTION_HIGHLIGHT_EXIT;
            break;

        // Wait for onHighlightExit animation to finish; load art
        case RETROFE_COLLECTION_HIGHLIGHT_EXIT:
            if (currentPage_->isIdle( ))
            {
                currentPage_->highlightLoadArt( );
                state = RETROFE_COLLECTION_HIGHLIGHT_LOAD_ART;
            }
            break;

        // Start onHighlightEnter animation
        case RETROFE_COLLECTION_HIGHLIGHT_LOAD_ART:
            currentPage_->highlightEnter( );
            if ( currentPage_->getSelectedItem( ) )
                l.LEDBlinky( 9, currentPage_->getSelectedItem( )->collectionInfo->name, currentPage_->getSelectedItem( ) );
            state = RETROFE_COLLECTION_HIGHLIGHT_ENTER;
            break;

        // Wait for onHighlightEnter animation to finish
        case RETROFE_COLLECTION_HIGHLIGHT_ENTER:
            if (currentPage_->isIdle( ))
            {
                RETROFE_STATE state_tmp = processUserInput( currentPage_ );
                if ( state_tmp == RETROFE_COLLECTION_DOWN_REQUEST )
                {
                    state = RETROFE_COLLECTION_DOWN_REQUEST;
                }
                else if ( state_tmp == RETROFE_COLLECTION_UP_REQUEST )
                {
                    state = RETROFE_COLLECTION_UP_REQUEST;
                }
                else
                {
                    state = RETROFE_NEXT_PAGE_REQUEST;
                }
            }
            break;

        // Start exit animation
        case RETROFE_COLLECTION_UP_REQUEST:
            if ( !pages_.empty( ) && currentPage_->getMenuDepth( ) == 1) // Inside a collection with a different layout
            {
                currentPage_->stop( );
                m.clearPage( );
                menuMode_ = false;
                state = RETROFE_COLLECTION_UP_EXIT;
            }
            else if ( currentPage_->getMenuDepth( ) > 1 ) // Inside a collection with the same layout
            {
                currentPage_->exitMenu( );
                state = RETROFE_COLLECTION_UP_EXIT;
            }
            else // Not in a collection
            {
                state = RETROFE_COLLECTION_UP_ENTER;
            }
            break;

        // Wait for the menu exit animation to finish
        case RETROFE_COLLECTION_UP_EXIT:
            if ( currentPage_->isIdle( ) )
            {
                std::string collectionName = currentPage_->getCollectionName();
                lastMenuOffsets_[collectionName] = currentPage_->getScrollOffsetIndex();
                lastMenuPlaylists_[collectionName] = currentPage_->getPlaylistName( );
                if (currentPage_->getMenuDepth( ) == 1) // Inside a collection with a different layout
                {
                    currentPage_->deInitialize( );
                    delete currentPage_;
                    currentPage_ = pages_.top( );
                    pages_.pop( );
                    currentPage_->allocateGraphicsMemory( );
                    currentPage_->setLocked(kioskLock_);
                }
                else // Inside a collection with the same layout
                {
                    currentPage_->popCollection( );
                }
                collectionName = currentPage_->getCollectionName();
                config_.setProperty( "currentCollection", collectionName );

                bool rememberMenu = false;
                config_.getProperty( "rememberMenu", rememberMenu );

                std::string autoPlaylist = "all";
                config_.getProperty( "autoPlaylist", autoPlaylist );
                bool returnToRememberedPlaylist = rememberMenu && lastMenuPlaylists_.find(collectionName) != lastMenuPlaylists_.end();
                if (returnToRememberedPlaylist)
                {
                    currentPage_->selectPlaylist( lastMenuPlaylists_[collectionName] ); // Switch to last playlist
                }
                else
                {
                    currentPage_->selectPlaylist( autoPlaylist );
                    if (currentPage_->getPlaylistName() != autoPlaylist )
                        currentPage_->selectPlaylist( "all" );
                }

                if (returnToRememberedPlaylist)
                {
                   if (lastMenuOffsets_.size() && lastMenuPlaylists_.find(collectionName) != lastMenuPlaylists_.end()) {
                       currentPage_->setScrollOffsetIndex(lastMenuOffsets_[collectionName]);
                   }
                }

                currentPage_->onNewItemSelected( );
                state = RETROFE_COLLECTION_UP_MENU_ENTER;
            }
            break;


        // Start menu enter animation
        case RETROFE_COLLECTION_UP_MENU_ENTER:
            currentPage_->enterMenu( );
            state = RETROFE_COLLECTION_UP_ENTER;
            break;
            

        // Waiting for enter animation to stop
        case RETROFE_COLLECTION_UP_ENTER:
            if ( currentPage_->isIdle( ) )
            {
                currentPage_->setScrolling(Page::ScrollDirectionBack);
                currentPage_->scroll(false);
                currentPage_->updateScrollPeriod( );
                state = RETROFE_COLLECTION_UP_SCROLL;
            }
            break;

        // Waiting for scrolling animation to stop
        case RETROFE_COLLECTION_UP_SCROLL:
            if ( currentPage_->isMenuIdle( ) )
            {
                RETROFE_STATE state_tmp;
                state_tmp = processUserInput( currentPage_ );
                if ( state_tmp == RETROFE_COLLECTION_DOWN_REQUEST )
                {
                    state = RETROFE_COLLECTION_DOWN_REQUEST;
                }
                else if ( state_tmp == RETROFE_COLLECTION_UP_REQUEST )
                {
                    state = RETROFE_COLLECTION_UP_REQUEST;
                }
                else
                {
                    currentPage_->setScrolling(Page::ScrollDirectionIdle); // Stop scrolling
                    nextPageItem_ = currentPage_->getSelectedItem( );
                    bool enterOnCollection = true;
                    config_.getProperty( "enterOnCollection", enterOnCollection );
                    if ( currentPage_->getSelectedItem( )->leaf || !enterOnCollection ) // Current selection is a game or enterOnCollection is not set
                    {
                        state = RETROFE_HIGHLIGHT_REQUEST;
                    }
                    else // Current selection is a menu
                    {
                        state = RETROFE_COLLECTION_HIGHLIGHT_EXIT;
                    }
                }
            }
            break;


        // Launching a menu entry
        case RETROFE_HANDLE_MENUENTRY:

            // Empty event queue
            SDL_Event e;
            while ( SDL_PollEvent( &e ) )
                input_.update(e);
            input_.resetStates( );

            // Handle menu entry
            m.handleEntry( currentPage_->getSelectedItem( ) );

            // Empty event queue
            while ( SDL_PollEvent( &e ) )
                input_.update(e);
            input_.resetStates( );

            state = RETROFE_IDLE;
            break;

        // Launching game; start onGameEnter animation
        case RETROFE_LAUNCH_ENTER:
            currentPage_->enterGame( );  // Start onGameEnter animation
            currentPage_->playSelect( ); // Play launch sound
            state = RETROFE_LAUNCH_REQUEST;
            break;

        // Wait for onGameEnter animation to finish; launch game; start onGameExit animation
        case RETROFE_LAUNCH_REQUEST:
            if ( currentPage_->isIdle( ) && !currentPage_->isSelectPlaying( ) )
            {
                nextPageItem_ = currentPage_->getSelectedItem( );
                launchEnter( );
                CollectionInfoBuilder cib(config_, *metadb_);
                std::string lastPlayedSkipCollection = "";
                int         size = 0;
                config_.getProperty( "lastPlayedSkipCollection", lastPlayedSkipCollection );
                config_.getProperty( "lastplayedSize", size );

                if (nextPageItem_->collectionInfo->name != lastPlayedSkipCollection)
                {
                    cib.updateLastPlayedPlaylist(currentPage_->getCollection(), nextPageItem_, size); // Update last played playlist if not currently in the skip playlist (e.g. settings)
                    currentPage_->updateReloadables(0);
                    
                    // with new sort by last played return to first
                    if (currentPage_->getPlaylistName() == "lastplayed")
                    {
                        currentPage_->setScrollOffsetIndex(0);
                        currentPage_->highlightLoadArt();
                        currentPage_->reallocateMenuSpritePoints();
                    }
                }

                l.LEDBlinky( 3, nextPageItem_->collectionInfo->name, nextPageItem_ );
                if (l.run(nextPageItem_->collectionInfo->name, nextPageItem_, currentPage_)) // Run and check if we need to reboot
                {
                    attract_.reset( );
                    reboot_ = true;
                    state   = RETROFE_QUIT_REQUEST;
                }
                else
                {
                    attract_.reset();
                    launchExit( );
                    l.LEDBlinky( 4 );
                    currentPage_->exitGame( );
                    
                    state = RETROFE_LAUNCH_EXIT;
                }
            }
            break;

        // Wait for onGameExit animation to finish
        case RETROFE_LAUNCH_EXIT:
            if ( currentPage_->isIdle( ) )
            {
                state = RETROFE_IDLE;
            }
            break;

        // Go back a page; start onMenuExit animation
        case RETROFE_BACK_REQUEST:
            if (currentPage_->getMenuDepth( ) == 1)
            {
                currentPage_->stop( );
                m.clearPage( );
                menuMode_ = false;
            }
            else
            {
                currentPage_->exitMenu( );
            }
            state = RETROFE_BACK_MENU_EXIT;
            break;

        // Wait for onMenuExit animation to finish; load previous page; load art
        case RETROFE_BACK_MENU_EXIT:
            if ( currentPage_->isIdle( ) )
            {
                std::string collectionName = currentPage_->getCollectionName();
                lastMenuOffsets_[collectionName] = currentPage_->getScrollOffsetIndex();
                lastMenuPlaylists_[collectionName] = currentPage_->getPlaylistName( );
                if (currentPage_->getMenuDepth( ) == 1)
                {
                    currentPage_->deInitialize( );
                    delete currentPage_;
                    currentPage_ = pages_.top( );
                    pages_.pop( );
                    if (currentPage_->getSelectedItem() != NULL) {
                        currentPage_->allocateGraphicsMemory();
                        currentPage_->setLocked(kioskLock_);
                    }
                }
                else
                {
                    currentPage_->popCollection( );
                }
                collectionName = currentPage_->getCollectionName();
                config_.setProperty( "currentCollection", collectionName );

                bool rememberMenu = false;
                config_.getProperty( "rememberMenu", rememberMenu );

                std::string autoPlaylist = "all";
                config_.getProperty( "autoPlaylist", autoPlaylist );
                bool returnToRememberedPlaylist = rememberMenu && lastMenuPlaylists_.find(collectionName) != lastMenuPlaylists_.end();
                if (returnToRememberedPlaylist)
                {
                    currentPage_->selectPlaylist( lastMenuPlaylists_[collectionName] ); // Switch to last playlist
                }
                else
                {
                    currentPage_->selectPlaylist( autoPlaylist );
                    if (currentPage_->getPlaylistName() != autoPlaylist )
                        currentPage_->selectPlaylist( "all" );
                }

                if (returnToRememberedPlaylist)
                {
                    if (lastMenuOffsets_.size() && lastMenuPlaylists_.find(collectionName) != lastMenuPlaylists_.end()) {
                        currentPage_->setScrollOffsetIndex(lastMenuOffsets_[collectionName]);
                    }
                }

                currentPage_->onNewItemSelected();
                currentPage_->reallocateMenuSpritePoints(); // update playlist menu
                state = RETROFE_BACK_MENU_LOAD_ART;
            }
            break;

        // Start onMenuEnter animation
        case RETROFE_BACK_MENU_LOAD_ART:
            currentPage_->enterMenu( );
            state = RETROFE_BACK_MENU_ENTER;
            break;

        // Wait for onMenuEnter animation to finish
        case RETROFE_BACK_MENU_ENTER:
            if ( currentPage_->isIdle( ) )
            {
                bool collectionInputClear = false;
                config_.getProperty( "collectionInputClear", collectionInputClear );
                if (  collectionInputClear  )
                {
                    // Empty event queue
                    SDL_Event e;
                    while ( SDL_PollEvent( &e ) )
                        input_.update(e);
                    input_.resetStates( );
                }
                state = RETROFE_IDLE;
            }
            break;

        // Start menu mode
        case RETROFE_MENUMODE_START_REQUEST:
            if ( currentPage_->isIdle( ) )
            {
                std::string collectionName = currentPage_->getCollectionName();
                lastMenuOffsets_[collectionName]   = currentPage_->getScrollOffsetIndex( );
                lastMenuPlaylists_[collectionName] = currentPage_->getPlaylistName( );
                std::string layoutName;
                config_.getProperty( "layout", layoutName );
                PageBuilder pb( layoutName, getLayoutFileName(), config_, &fontcache_, true );
                Page *page = pb.buildPage( );
                if ( page )
                {
                    if (page->controlsType() != "") {
                        updatePageControls(page->controlsType());
                        page->setControlsType("");
                    }
                    currentPage_->freeGraphicsMemory( );
                    pages_.push( currentPage_ );
                    currentPage_ = page;
                    currentPage_->setLocked(kioskLock_);
                    menuMode_ = true;
                    m.setPage( page );
                }
                config_.setProperty( "currentCollection", "menu" );
                CollectionInfo *info = getMenuCollection( "menu" );
                currentPage_->pushCollection(info);
                currentPage_->onNewItemSelected();
                currentPage_->reallocateMenuSpritePoints(); // update playlist menu
                state = RETROFE_MENUMODE_START_LOAD_ART;
            }
            break;

        case RETROFE_MENUMODE_START_LOAD_ART:
            currentPage_->start();
            state = RETROFE_MENUMODE_START_ENTER;
            break;

        case RETROFE_MENUMODE_START_ENTER:
            if ( currentPage_->isIdle( ) )
            {
                SDL_Event e;
                while ( SDL_PollEvent( &e ) )
                    input_.update(e);
                input_.resetStates( );
                state = RETROFE_IDLE;
            }
            break;

        // Wait for splash mode animation to finish
        case RETROFE_NEW:
            if ( currentPage_->isIdle( ) )
            {
                state = RETROFE_IDLE;
            }
            break;

        // Start the onExit animation
        case RETROFE_QUIT_REQUEST:
            currentPage_->stop( );
            state = RETROFE_QUIT;
            break;

        // Wait for onExit animation to finish before quitting RetroFE
        case RETROFE_QUIT:
            if ( currentPage_->isGraphicsIdle( ) )
            {
                l.LEDBlinky( 2 );
                running = false;
            }
            break;
        }
		
        // Handle screen updates and attract mode
        if ( running )
        {
            lastTime = currentTime_;
            currentTime_ = static_cast<float>( SDL_GetTicks( ) ) / 1000;

            if ( currentTime_ < lastTime )
            {
                currentTime_ = lastTime;
            }

            deltaTime = currentTime_ - lastTime;
            double sleepTime;
			if (state == RETROFE_IDLE)
                sleepTime = fpsIdleTime - deltaTime*1000;
            else
                sleepTime = fpsTime - deltaTime*1000;
            if ( sleepTime > 0 && sleepTime < 1000 )
			{
				if (vSync == false)
				{
					SDL_Delay( static_cast<unsigned int>( sleepTime ) );
				}
			}
		
            if ( currentPage_ )
            {
                if (!splashMode && !paused_)
                {
                    int attractReturn = attract_.update( deltaTime, *currentPage_ );
                    if (!kioskLock_ && attractReturn == 1) // Change playlist
                    {
                        attract_.reset( attract_.isSet( ) );
                        std::string settingPrefix = "collections." + currentPage_->getCollectionName() + ".";

                        bool cyclePlaylist = true;
                        // check if collection has different setting
                        if (config_.propertyExists(settingPrefix + "attractModeCyclePlaylist")) {
                            config_.getProperty(settingPrefix + "attractModeCyclePlaylist", cyclePlaylist);
                        }
                        else {
                            config_.getProperty("attractModeCyclePlaylist", cyclePlaylist);
                        }

                        std::string cycleString;
                        // check if collection has different setting
                        if (config_.propertyExists(settingPrefix + "cyclePlaylist")) {
                            config_.getProperty(settingPrefix + "cyclePlaylist", cycleString);
                        } else {
                            config_.getProperty("cyclePlaylist", cycleString);
                        }

                        // go to next playlist in cycle or from all found playlists
                        std::vector<std::string> cycleVector;
                        Utils::listToVector(cycleString, cycleVector, ',');
                        if (cyclePlaylist)
                            currentPage_->nextCyclePlaylist(cycleVector);
                        else
                            currentPage_->nextPlaylist();

                        // if that next playlist is one to skip for attract, then find one that isn't
                        if (isInAttractModeSkipPlaylist(currentPage_->getPlaylistName()))
                        {
                            if (cyclePlaylist) {
                                goToNextAttractModePlaylistByCycle(cycleVector);
                            }
                            else {
                                // todo perform smarter playlist skipping
                                currentPage_->nextPlaylist();
                            }
                        }
                        state = RETROFE_PLAYLIST_REQUEST;
                    }
                    if (!kioskLock_ && attractReturn == 2) // Change collection
                    {
                        attract_.reset( attract_.isSet( ) );
                        state = RETROFE_COLLECTION_DOWN_REQUEST;
                    }
                }
                if ( menuMode_ )
                {
                    attract_.reset( );
                }
                currentPage_->update( deltaTime );
                SDL_PumpEvents( );
                input_.updateKeystate( );
                if (!splashMode && !paused_)
                {
                    if ( currentPage_->isAttractIdle( ) )
                    {
                        if ( !attractMode_ && attract_.isSet( ) )
                        {
                            currentPage_->attractEnter( );
                            l.LEDBlinky( 5 );
                        }
                        else if ( attractMode_ && !attract_.isSet( ) )
                        {
                            currentPage_->attractExit( );
                            l.LEDBlinky( 6 );
                        }
                        else if ( attract_.isSet( ) )
                        {
                            currentPage_->attract( );
                        }
                        attractMode_ = attract_.isSet( );
                    }
                }
            }

            render( );
        }
    }
    return reboot_;
}


// Check if we can go back a page or quite RetroFE
bool RetroFE::back(bool &exit)
{
    bool canGoBack  = false;
    bool exitOnBack = false;
    config_.getProperty( "exitOnFirstPageBack", exitOnBack );
    exit = false;

    if ( currentPage_->getMenuDepth( ) <= 1 && pages_.empty( ) )
    {
        exit = exitOnBack;
    }
    else
    {
        canGoBack = true;
    }

    return canGoBack;
}

bool RetroFE::isStandalonePlaylist(std::string playlist)
{
    return playlist == "street fighter and capcom fighters" ||
        playlist == "street fighter";
}

bool RetroFE::isInAttractModeSkipPlaylist(std::string playlist)
{
    if (lkupAttractModeSkipPlaylist_.empty()) {
        std::string attractModeSkipPlaylist = "";
        std::string settingPrefix = "collections." + currentPage_->getCollectionName() + ".";
        // check if collection has different setting
        if (config_.propertyExists(settingPrefix + "attractModeSkipPlaylist")) {
            config_.getProperty(settingPrefix + "attractModeSkipPlaylist", attractModeSkipPlaylist);
        }
        else {
            config_.getProperty("attractModeSkipPlaylist", attractModeSkipPlaylist);
        }

        if (attractModeSkipPlaylist != "") {
            // see if any of the comma seperated match current playlist
            std::stringstream ss(attractModeSkipPlaylist);
            std::string playlist = "";
            while (ss.good())
            {
                getline(ss, playlist, ',');
                lkupAttractModeSkipPlaylist_.insert(make_pair(playlist, true));
            }
        }
    }

    return !lkupAttractModeSkipPlaylist_.empty() && lkupAttractModeSkipPlaylist_.find(playlist) != lkupAttractModeSkipPlaylist_.end();
}

void RetroFE::goToNextAttractModePlaylistByCycle(std::vector<std::string> cycleVector)
{
    // find current position
    std::vector<std::string>::iterator it = cycleVector.begin();
    while (it != cycleVector.end() && *it != currentPage_->getPlaylistName())
        ++it;
    // find next playlist that is not in list 
    for (;;) {
        if (!isInAttractModeSkipPlaylist(*it)) {
            break;
        }
        ++it;
        if (it == cycleVector.end())
            it = cycleVector.begin();
    }
    if (currentPage_->playlistExists(*it)) {
        currentPage_->selectPlaylist(*it);                            
    }
}

// Process the user input
RetroFE::RETROFE_STATE RetroFE::processUserInput( Page *page )
{
    bool exit = false;
    RETROFE_STATE state = RETROFE_IDLE;

    // Poll all events until we find an active one
    SDL_Event e;
    while ( SDL_PollEvent( &e ) )
    {
        input_.update(e);
        if ( e.type == SDL_KEYDOWN && !SDL_KEYUP )
        {
            break;
        }
    }

    // Handle next/previous game inputs
    if ( page->isHorizontalScroll( ) )
    {
        if (input_.keystate(UserInput::KeyCodeRight))
        {
            attract_.reset( );
            page->setScrolling(Page::ScrollDirectionForward);
            page->scroll(true);
            page->updateScrollPeriod( );
            return state;
        }
        else if (input_.keystate(UserInput::KeyCodeLeft))
        {
            attract_.reset( );
            page->setScrolling(Page::ScrollDirectionBack);
            page->scroll(false);
            page->updateScrollPeriod( );
            return state;
        }
    }
    else
    {
        if (input_.keystate(UserInput::KeyCodeDown))
        {
            attract_.reset( );
            page->setScrolling(Page::ScrollDirectionForward);
            page->scroll(true);
            page->updateScrollPeriod( );
            return state;
        }
        else if (input_.keystate(UserInput::KeyCodeUp))
        {
            attract_.reset( );
            page->setScrolling(Page::ScrollDirectionBack);
            page->scroll(false);
            page->updateScrollPeriod( );
            return state;
        }
    }

    // Ignore other keys while the menu is scrolling
    if ( page->isIdle( ) && currentTime_ - keyLastTime_ > keyDelayTime_ )
    {

        // lock or unlock playlist/collection/menu nav and fav toggle
        if (input_.keystate(UserInput::KeyCodeKisok)) {
            attract_.reset();
            kioskLock_ = !kioskLock_;
            page->setLocked(kioskLock_);
            page->onNewItemSelected();
            keyLastTime_ = currentTime_;
        }else if ( input_.keystate(UserInput::KeyCodeMenu) && !menuMode_)
        {
            state = RETROFE_MENUMODE_START_REQUEST;
        }
        else if (input_.keystate(UserInput::KeyCodeQuitCombo1) && input_.keystate(UserInput::KeyCodeQuitCombo2)) {
            attract_.reset();
            bool controllerComboExit = false;
            config_.getProperty("controllerComboExit", controllerComboExit);
            if (controllerComboExit) {
#ifdef WIN32
                postMessage("MediaplayerHiddenWindow", 0x8001, 51, 0);
#endif              			
                return RETROFE_QUIT_REQUEST;
            }
        }

        // Handle Collection Up/Down keys
        else if (!kioskLock_ && ((input_.keystate(UserInput::KeyCodeCollectionUp)   && ( page->isHorizontalScroll( ) || !input_.keystate(UserInput::KeyCodeUp))) ||
                 (input_.keystate(UserInput::KeyCodeCollectionLeft) && (!page->isHorizontalScroll( ) || !input_.keystate(UserInput::KeyCodeLeft)))))
        {
            attract_.reset( );
            bool backOnCollection = false;
            config_.getProperty( "backOnCollection", backOnCollection );
            if ( page->getMenuDepth( ) == 1 || !backOnCollection )
                state = RETROFE_COLLECTION_UP_REQUEST;
            else
                state = RETROFE_BACK_REQUEST;
        }

        else if (!kioskLock_ && ((input_.keystate(UserInput::KeyCodeCollectionDown)  && ( page->isHorizontalScroll( ) || !input_.keystate(UserInput::KeyCodeDown))) ||
                 (input_.keystate(UserInput::KeyCodeCollectionRight) && (!page->isHorizontalScroll( ) || !input_.keystate(UserInput::KeyCodeRight)))))
        {
            attract_.reset( );
            bool backOnCollection = false;
            config_.getProperty( "backOnCollection", backOnCollection );
            if ( page->getMenuDepth( ) == 1 || !backOnCollection )
                state = RETROFE_COLLECTION_DOWN_REQUEST;
            else
                state = RETROFE_BACK_REQUEST;
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodePageUp))
        {
            attract_.reset( );
            page->pageScroll(Page::ScrollDirectionBack);
            state = RETROFE_MENUJUMP_REQUEST;
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodePageDown))
        {
            attract_.reset( );
            page->pageScroll(Page::ScrollDirectionForward);
            state = RETROFE_MENUJUMP_REQUEST;
        }

        else if (input_.keystate(UserInput::KeyCodeLetterUp))
        {
            attract_.reset( );
            if (currentPage_->getPlaylistName() != "lastplayed") {
                // if playlist same name as meta sort value then support meta jump
                if (Item::validSortType(page->getPlaylistName())) {
                    page->metaScroll(Page::ScrollDirectionBack, page->getPlaylistName());
                }
                else {
                    bool cfwLetterSub;
                    config_.getProperty("cfwLetterSub", cfwLetterSub);
                    if (cfwLetterSub && page->hasSubs())
                        page->cfwLetterSubScroll(Page::ScrollDirectionBack);
                    else
                        page->letterScroll(Page::ScrollDirectionBack);
                }
                state = RETROFE_MENUJUMP_REQUEST;
            }
        }

        else if (input_.keystate(UserInput::KeyCodeLetterDown))
        {
            attract_.reset( );
            if (currentPage_->getPlaylistName() != "lastplayed") {
                if (Item::validSortType(page->getPlaylistName())) {
                    page->metaScroll(Page::ScrollDirectionForward, page->getPlaylistName());
                }
                else {
                    bool cfwLetterSub;
                    config_.getProperty("cfwLetterSub", cfwLetterSub);
                    if (cfwLetterSub && page->hasSubs())
                        page->cfwLetterSubScroll(Page::ScrollDirectionForward);
                    else
                        page->letterScroll(Page::ScrollDirectionForward);
                }
                state = RETROFE_MENUJUMP_REQUEST;
            }
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodeFavPlaylist) )
        {
            attract_.reset( );
            page->favPlaylist( );
            state = RETROFE_PLAYLIST_REQUEST;
        }

        else if (!kioskLock_ && (input_.keystate(UserInput::KeyCodeNextPlaylist) ||
                 (input_.keystate(UserInput::KeyCodePlaylistDown)  &&  page->isHorizontalScroll( )) ||
                 (input_.keystate(UserInput::KeyCodePlaylistRight) && !page->isHorizontalScroll( ))))
        {
            attract_.reset( );
            page->nextPlaylist( );
            state = RETROFE_PLAYLIST_REQUEST;
        }

        else if (!kioskLock_ && (input_.keystate(UserInput::KeyCodePrevPlaylist) ||
                 (input_.keystate(UserInput::KeyCodePlaylistUp)   &&  page->isHorizontalScroll( )) ||
                 (input_.keystate(UserInput::KeyCodePlaylistLeft) && !page->isHorizontalScroll( ))))
        {
            attract_.reset( );
            page->playlistPrevEnter();
            page->prevPlaylist( );
            state = RETROFE_PLAYLIST_REQUEST;
        }

        else if (!kioskLock_ && (input_.keystate(UserInput::KeyCodeCyclePlaylist) ||
                  input_.keystate(UserInput::KeyCodeNextCyclePlaylist)))
        {
            if (!isStandalonePlaylist(currentPage_->getPlaylistName()))
            {
                attract_.reset();
                std::string settingPrefix = "collections." + currentPage_->getCollectionName() + ".";
                std::string cycleString;
                // check if collection has different setting
                if (config_.propertyExists(settingPrefix + "cyclePlaylist")) {
                    config_.getProperty(settingPrefix + "cyclePlaylist", cycleString);
                }
                else {
                    config_.getProperty("cyclePlaylist", cycleString);
                }

                std::vector<std::string> cycleVector;
                Utils::listToVector(cycleString, cycleVector, ',');
                page->nextCyclePlaylist(cycleVector);
                state = RETROFE_PLAYLIST_REQUEST;

            }
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodeCycleCollection))
        {
            attract_.reset();
            if (collectionCycle_.size()) {
                collectionCycleIt_++;
                if (collectionCycleIt_ == collectionCycle_.end()) {
                    collectionCycleIt_ = collectionCycle_.begin();
                }
                if (!pages_.empty() && pages_.size() > 1)
                    pages_.pop();

                nextPageItem_ = new Item();
                nextPageItem_->name = *collectionCycleIt_;
                menuMode_ = false;

                state = RETROFE_NEXT_PAGE_REQUEST;
            }
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodePrevCyclePlaylist))
        {
            if (!isStandalonePlaylist(currentPage_->getPlaylistName()))
            {
                attract_.reset();
                std::string settingPrefix = "collections." + currentPage_->getCollectionName() + ".";
                std::string cycleString;
                // check if collection has different setting
                if (config_.propertyExists(settingPrefix + "cyclePlaylist")) {
                    config_.getProperty(settingPrefix + "cyclePlaylist", cycleString);
                }
                else {
                    config_.getProperty("cyclePlaylist", cycleString);
                }
                std::vector<std::string> cycleVector;
                Utils::listToVector(cycleString, cycleVector, ',');
                page->playlistPrevEnter();
                page->prevCyclePlaylist(cycleVector);
                state = RETROFE_PLAYLIST_REQUEST;
            }
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodeRemovePlaylist))
        {
            attract_.reset( );
            page->rememberSelectedItem();
            page->removePlaylist( );

            // don't trigger playlist change events but refresh item states
            currentPage_->onNewItemSelected();
            state = RETROFE_PLAYLIST_ENTER;
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodeAddPlaylist) )
        {
            if (!isStandalonePlaylist(currentPage_->getPlaylistName()))
            {
                attract_.reset();
                    page->rememberSelectedItem();
                    page->addPlaylist();

                    // don't trigger playlist change events but refresh item states
                    currentPage_->onNewItemSelected();
                    state = RETROFE_PLAYLIST_ENTER;
            }
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodeTogglePlaylist) )
        {
            if (currentPage_->getPlaylistName() != "favorites" && 
                !isStandalonePlaylist(currentPage_->getPlaylistName()))
            {
                attract_.reset();
                page->rememberSelectedItem();
                page->togglePlaylist();

                // don't trigger playlist change events but refresh item states
                currentPage_->onNewItemSelected();
                state = RETROFE_PLAYLIST_ENTER;
            }
	    }

        else if ( input_.keystate(UserInput::KeyCodeSkipForward) )
        {
            attract_.reset( );
            page->skipForward( );
            page->jukeboxJump( );
            keyLastTime_ = currentTime_;
        }

        else if ( input_.keystate(UserInput::KeyCodeSkipBackward) )
        {
            attract_.reset( );
            page->skipBackward( );
            page->jukeboxJump( );
            keyLastTime_ = currentTime_;
        }

        else if ( input_.keystate(UserInput::KeyCodeSkipForwardp) )
        {
            attract_.reset( );
            page->skipForwardp( );
            page->jukeboxJump( );
            keyLastTime_ = currentTime_;
        }

        else if ( input_.keystate(UserInput::KeyCodeSkipBackwardp) )
        {
            attract_.reset( );
            page->skipBackwardp( );
            page->jukeboxJump( );
            keyLastTime_ = currentTime_;
        }

        else if ( input_.keystate(UserInput::KeyCodePause) )
        {
            page->pause( );
            page->jukeboxJump( );
            keyLastTime_ = currentTime_;
            paused_ = !paused_;
            if (!paused_) {
                // trigger attract on unpause
                attract_.activate();
            }
        }

        else if ( input_.keystate(UserInput::KeyCodeRestart) )
        {
            attract_.reset( );
            page->restart( );
            keyLastTime_ = currentTime_;
        }

        else if ( input_.keystate(UserInput::KeyCodeRandom) )
        {
            attract_.reset( );
            page->selectRandom( );
            state = RETROFE_MENUJUMP_REQUEST;
        }

        else if (input_.keystate(UserInput::KeyCodeAdminMode))
        {
            //todo: add admin mode support
        }

        else if (input_.keystate(UserInput::KeyCodeSelect))
        {
            attract_.reset( );
            nextPageItem_ = page->getSelectedItem( );
            if ( nextPageItem_ )
            {
                if ( nextPageItem_->leaf )
                {
                    if ( menuMode_ )
                    {
                        state = RETROFE_HANDLE_MENUENTRY;
                    }
                    else
                    {
                        state = RETROFE_LAUNCH_ENTER;
                    }
                }
                else
                {
                    CollectionInfoBuilder cib(config_, *metadb_);
                    std::string lastPlayedSkipCollection = "";
                    int         size = 0;
                    config_.getProperty( "lastPlayedSkipCollection", lastPlayedSkipCollection );
                    config_.getProperty("lastplayedCollectionSize", size);

                    if (!isInAttractModeSkipPlaylist(currentPage_->getPlaylistName()) &&
                        nextPageItem_->collectionInfo->name != lastPlayedSkipCollection) 
                    {
                        cib.updateLastPlayedPlaylist(currentPage_->getCollection(), nextPageItem_, size); // Update last played playlist if not currently in the skip playlist (e.g. settings)
                        currentPage_->updateReloadables(0);
                    }
                    state = RETROFE_NEXT_PAGE_REQUEST;
                }
            }
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodeBack))
        {
            attract_.reset( );
            if ( back( exit ) || exit )
            {
                state = (exit) ? RETROFE_QUIT_REQUEST : RETROFE_BACK_REQUEST;
            }
        }

        else if (input_.keystate(UserInput::KeyCodeQuit)) // !kioskLock_ && 
        {
            attract_.reset( );
#ifdef WIN32
        postMessage("MediaplayerHiddenWindow", 0x8001, 51,0);		
#endif              			
            state = RETROFE_QUIT_REQUEST;
        }

        else if (input_.keystate(UserInput::KeyCodeReboot)) // !kioskLock_ && 
        {
            attract_.reset( );
            reboot_ = true;
            state   = RETROFE_QUIT_REQUEST;
        }

        else if (!kioskLock_ && input_.keystate(UserInput::KeyCodeSaveFirstPlaylist))
        {
            attract_.reset( );
            if ( page->getMenuDepth( ) == 1 )
            {
                firstPlaylist_ = page->getPlaylistName( );
                saveRetroFEState( );
            }
        }
    }

    if ( state != RETROFE_IDLE )
    {
        keyLastTime_ = currentTime_;
        return state;
    }

    // Check if we're done scrolling
    if ( !input_.keystate(UserInput::KeyCodeUp) &&
         !input_.keystate(UserInput::KeyCodeLeft) &&
         !input_.keystate(UserInput::KeyCodeDown) &&
         !input_.keystate(UserInput::KeyCodeRight) &&
         !input_.keystate(UserInput::KeyCodePlaylistUp) &&
         !input_.keystate(UserInput::KeyCodePlaylistLeft) &&
         !input_.keystate(UserInput::KeyCodePlaylistDown) &&
         !input_.keystate(UserInput::KeyCodePlaylistRight) &&
         !input_.keystate(UserInput::KeyCodeCollectionUp) &&
         !input_.keystate(UserInput::KeyCodeCollectionLeft) &&
         !input_.keystate(UserInput::KeyCodeCollectionDown) &&
         !input_.keystate(UserInput::KeyCodeCollectionRight) &&
         !input_.keystate(UserInput::KeyCodePageUp) &&
         !input_.keystate(UserInput::KeyCodePageDown) &&
         !input_.keystate(UserInput::KeyCodeLetterUp) &&
         !input_.keystate(UserInput::KeyCodeLetterDown) &&
         !attract_.isActive( ) )
    {
        page->resetScrollPeriod( );
        if (page->isMenuScrolling( ))
        {
            attract_.reset( attract_.isSet( ) );
            state = RETROFE_HIGHLIGHT_REQUEST;
        }
    }

    return state;
}


// Load a page
Page* RetroFE::loadPage(std::string collectionName)
{
    std::string layoutName;

    config_.getProperty("layout", layoutName);

    PageBuilder pb(layoutName, getLayoutFileName(), config_, &fontcache_);
    Page* page = pb.buildPage(collectionName);
    if (!page) {
        page = pb.buildPage();
    }
    if (!page)
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not create page");
    }
    else {
        if (page->controlsType() != "") {
            updatePageControls(page->controlsType());
            page->setControlsType("");
        }
    }

    return page;
}


// Load the splash page
Page *RetroFE::loadSplashPage( )
{
    std::string layoutName;
    config_.getProperty( "layout", layoutName );

    PageBuilder pb( layoutName, "splash", config_, &fontcache_ );
    Page * page = pb.buildPage( );
    page->start( );

    return page;
}

// Load a collection
CollectionInfo *RetroFE::getCollection(std::string collectionName)
{

    // Check if subcollections should be merged or split
    bool subsSplit = false;
    config_.getProperty( "subsSplit", subsSplit );

    // Build the collection
    CollectionInfoBuilder cib(config_, *metadb_);
    CollectionInfo *collection = cib.buildCollection( collectionName );
    collection->subsSplit = subsSplit;
    cib.injectMetadata( collection );

    DIR *dp;
    struct dirent *dirp;

    // check collection folder exists 
    std::string path = Utils::combinePath( Configuration::absolutePath, "collections", collectionName );
    dp = opendir( path.c_str( ) );
    if (dp == NULL) {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Failed to load collection " + collectionName);

        return NULL;
    }

    // Loading sub collection files
    while ( (dirp = readdir( dp )) != NULL )
    {
        std::string file = dirp->d_name;

        size_t position = file.find_last_of( "." );
        std::string basename = (std::string::npos == position)? file : file.substr( 0, position );

        std::string comparator = ".sub";
        int start = file.length( ) - comparator.length( );

        if ( start >= 0 )
        {
            if ( file.compare( start, comparator.length( ), comparator ) == 0 )
            {
                Logger::write( Logger::ZONE_INFO, "RetroFE", "Loading subcollection into menu: " + basename );

                CollectionInfo *subcollection = cib.buildCollection( basename, collectionName );
                collection->addSubcollection( subcollection );
                subcollection->subsSplit = subsSplit;
                cib.injectMetadata( subcollection );
                collection->hasSubs = true;
            }
        }
    }
    if (dp) closedir(dp);

    // sort a collection's items
    bool menuSort = true;
    config_.getProperty( "collections." + collectionName + ".list.menuSort", menuSort );
    if (menuSort) {
        config_.getProperty("collections." + collectionName + ".list.sortType", collection->sortType);
        if (!Item::validSortType(collection->sortType)) {
            collection->sortType = "";
        }
        collection->sortItems();
    }

    // adds items to "all" list except those found in "exclude_all.txt"
    cib.addPlaylists(collection);
    collection->sortPlaylists();

    // build collection menu if menu.txt exists
    MenuParser mp;
    mp.buildMenuItems(collection, menuSort);

    // Add extra info, if available
    for ( std::vector<Item *>::iterator it = collection->items.begin( ); it != collection->items.end( ); it++ )
    {
        std::string path = Utils::combinePath( Configuration::absolutePath, "collections", collectionName, "info", (*it)->name + ".conf" );
        (*it)->loadInfo( path );
    }

    // Remove parenthesis and brackets, if so configured
    bool showParenthesis    = true;
    bool showSquareBrackets = true;

    (void)config_.getProperty( "showParenthesis", showParenthesis );
    (void)config_.getProperty( "showSquareBrackets", showSquareBrackets );

    typedef std::map<std::string, std::vector <Item *> *> Playlists_T;
    for ( Playlists_T::iterator itP = collection->playlists.begin( ); itP != collection->playlists.end( ); itP++ )
    {
        for ( std::vector <Item *>::iterator itI = itP->second->begin( ); itI != itP->second->end( ); itI++ )
        {
            if ( !showParenthesis )
            {
                std::string::size_type firstPos  = (*itI)->title.find_first_of( "(" );
                std::string::size_type secondPos = (*itI)->title.find_first_of( ")", firstPos );
    
                while ( firstPos != std::string::npos && secondPos != std::string::npos )
                {
                    firstPos  = (*itI)->title.find_first_of( "(" );
                    secondPos = (*itI)->title.find_first_of( ")", firstPos );
    
                    if ( firstPos != std::string::npos )
                    {
                        (*itI)->title.erase( firstPos, (secondPos - firstPos) + 1 );
                    }
                }
            }
            if ( !showSquareBrackets )
            {
                std::string::size_type firstPos  = (*itI)->title.find_first_of( "[" );
                std::string::size_type secondPos = (*itI)->title.find_first_of( "]", firstPos );
    
                while ( firstPos != std::string::npos && secondPos != std::string::npos )
                {
                    firstPos  = (*itI)->title.find_first_of( "[" );
                    secondPos = (*itI)->title.find_first_of( "]", firstPos );
    
                    if ( firstPos != std::string::npos && secondPos != std::string::npos )
                    {
                        (*itI)->title.erase( firstPos, (secondPos - firstPos) + 1 );
                    }
                }
            }
        }
    }

    return collection;
}

void RetroFE::updatePageControls(std::string type)
{
    Logger::write(Logger::ZONE_INFO, "Layout", "Layout changed controls type " + type);
    std::string controlsConfPath = Utils::combinePath(Configuration::absolutePath, "controls");
    if (config_.import("controls", controlsConfPath + " - " + type + ".conf")) {
        input_.reconfigure();
    }
}

// Load a menu
CollectionInfo *RetroFE::getMenuCollection( std::string collectionName )
{
    std::string menuPath = Utils::combinePath( Configuration::absolutePath, "menu" );
    std::string menuFile = Utils::combinePath( menuPath, collectionName + ".txt" );
    std::vector<Item *> menuVector;
    CollectionInfoBuilder cib( config_, *metadb_ );
    CollectionInfo *collection = new CollectionInfo(config_, collectionName, menuPath, "", "", "" );
    cib.ImportBasicList( collection, menuFile, menuVector );
    for ( std::vector<Item *>::iterator it = menuVector.begin( ); it != menuVector.end( ); ++it)
    {
        (*it)->leaf = false;
        size_t position = (*it)->name.find( "=" );
        if ( position != std::string::npos )
        {
            (*it)->ctrlType  = Utils::trimEnds( (*it)->name.substr( position+1, (*it)->name.size( )-1 ) );
            (*it)->name      = Utils::trimEnds( (*it)->name.substr( 0, position ) );
            (*it)->title     = (*it)->name;
            (*it)->fullTitle = (*it)->name;
            (*it)->leaf      = true;
        }
        (*it)->collectionInfo = collection;
        collection->items.push_back( *it );
    }
    collection->playlists["all"] = &collection->items;
    return collection;
}


void RetroFE::saveRetroFEState( )
{
    std::string file = Utils::combinePath(Configuration::absolutePath, "settings_saved.conf");
    Logger::write(Logger::ZONE_INFO, "RetroFE", "Saving settings_saved.conf");
    std::ofstream filestream;
    try
    {
        filestream.open(file.c_str());
        filestream << "firstPlaylist = " << firstPlaylist_ << std::endl;
        filestream.close();
    }
    catch(std::exception &)
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Save failed: " + file);
    }
}

std::string RetroFE::getLayoutFileName()
{
    std::string layoutName = "layout";
    std::string randomLayoutNames;
    config_.getProperty("randomLayout", randomLayoutNames);
    if (randomLayoutNames != "") {
        Logger::write(Logger::ZONE_INFO, "RetroFE", "Choosing random layout from: " + randomLayoutNames);
        std::vector<std::string> randomLayoutVector;
        Utils::listToVector(randomLayoutNames, randomLayoutVector, ',');
        if (randomLayoutVector.size() > 1) {
            layoutName = randomLayoutVector[rand() % randomLayoutVector.size()];
        }
        else {
            layoutName = randomLayoutVector[0];
        }
    }

    return layoutName;
}
