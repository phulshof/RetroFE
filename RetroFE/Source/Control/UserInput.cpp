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

#include "UserInput.h"
#include "../Database/Configuration.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include "JoyAxisHandler.h"
#include "JoyButtonHandler.h"
#include "JoyHatHandler.h"
#include "KeyboardHandler.h"
#include "MouseButtonHandler.h"

UserInput::UserInput(Configuration &c)
    : config_(c)
{
    for(unsigned int i = 0; i < KeyCodeMax; ++i)
    {
        currentKeyState_[i] = false;
        lastKeyState_[i] = false;
    }
    for ( unsigned int i = 0; i < cMaxJoy; i++ )
    {
        joysticks_[i] = -1;
    }
}

UserInput::~UserInput()
{
    for (unsigned int i = 0; i < keyHandlers_.size(); ++i)
    {
        if (keyHandlers_[i].first)
        {
            delete keyHandlers_[i].first;
        }
    }
}

bool UserInput::initialize()
{

    // Optional keys
    MapKey("playlistUp", KeyCodePlaylistUp, false );
    MapKey("playlistDown", KeyCodePlaylistDown, false );
    MapKey("playlistLeft", KeyCodePlaylistLeft, false );
    MapKey("playlistRight", KeyCodePlaylistRight, false );
    MapKey("collectionUp", KeyCodeCollectionUp, false );
    MapKey("collectionDown", KeyCodeCollectionDown, false );
    MapKey("collectionLeft", KeyCodeCollectionLeft, false );
    MapKey("collectionRight", KeyCodeCollectionRight, false );
    MapKey("pageDown", KeyCodePageDown, false );
    MapKey("pageUp", KeyCodePageUp, false );
    MapKey("letterDown", KeyCodeLetterDown, false);
    MapKey("letterUp", KeyCodeLetterUp, false);
    MapKey("favPlaylist", KeyCodeFavPlaylist, false);
    MapKey("nextPlaylist", KeyCodeNextPlaylist, false);
    MapKey("prevPlaylist", KeyCodePrevPlaylist, false);
    MapKey("cyclePlaylist", KeyCodeCyclePlaylist, false);
    MapKey("nextCyclePlaylist", KeyCodeNextCyclePlaylist, false);
    MapKey("prevCyclePlaylist", KeyCodePrevCyclePlaylist, false);
    MapKey("addPlaylist", KeyCodeAddPlaylist, false);
    MapKey("removePlaylist", KeyCodeRemovePlaylist, false);
    MapKey("togglePlaylist", KeyCodeTogglePlaylist, false);
    MapKey("random", KeyCodeRandom, false);
    MapKey("menu", KeyCodeMenu, false);
	MapKey("reboot", KeyCodeReboot, false);
	MapKey("saveFirstPlaylist", KeyCodeSaveFirstPlaylist, false);

    bool retVal = true;

    // At least have controls for either a vertical or horizontal menu
    if(!MapKey("up", KeyCodeUp))
    {
        retVal = MapKey("left", KeyCodeUp) && retVal;
    }
    if(!MapKey("left", KeyCodeLeft))
    {
        retVal = MapKey("up", KeyCodeLeft) && retVal;
    }
    if(!MapKey("down", KeyCodeDown))
    {
        retVal = MapKey("right", KeyCodeDown ) && retVal;
    }
    if(!MapKey("right", KeyCodeRight ))
    {
        retVal = MapKey("down", KeyCodeRight ) && retVal;
    }

    // These keys are mandatory
    retVal = MapKey("select", KeyCodeSelect) && retVal;
    retVal = MapKey("back",   KeyCodeBack) && retVal;
    retVal = MapKey("quit",   KeyCodeQuit) && retVal;

    return retVal;
}

bool UserInput::MapKey(std::string keyDescription, KeyCode_E key)
{
    return MapKey(keyDescription, key, true);
}

bool UserInput::MapKey(std::string keyDescription, KeyCode_E key, bool required)
{
    SDL_Scancode scanCode;
    std::string description;

    std::string configKey = "controls." + keyDescription;

    if(!config_.getProperty(configKey, description))
    {
        Logger::Zone zone = (required) ? Logger::ZONE_ERROR : Logger::ZONE_INFO;
        Logger::write(zone, "Input", "Missing property " + configKey);
        return false;
    }

    std::istringstream ss(description);
    std::string token;

    bool success = true;

    while (std::getline(ss, token, ','))
    {
        token = Configuration::trimEnds(token);
        if (token == "" && description != "") // Allow "," as input key
            token = ",";
        scanCode = SDL_GetScancodeFromName(token.c_str());

        bool found = false;

        if (scanCode != SDL_SCANCODE_UNKNOWN)
        {
            Logger::write(Logger::ZONE_INFO, "Input", "Binding key " + configKey);
            keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new KeyboardHandler(scanCode), key));
            found = true;
        }
        else
        {
            token = Utils::toLower(token);

            if (token.find("mouse") == 0)
            {
                std::string mousedesc = Utils::replace(Utils::toLower(token), "mouse", "");
                if (mousedesc.find("button") == 0)
                {
                    int button = 0;
                    std::stringstream ss;
                    mousedesc = Utils::replace(mousedesc, "button", "");
                    if (mousedesc == "left") button = SDL_BUTTON_LEFT;
                    else if (mousedesc == "middle") button = SDL_BUTTON_MIDDLE;
                    else if (mousedesc == "right") button = SDL_BUTTON_RIGHT;
                    else if (mousedesc == "x1") button = SDL_BUTTON_X1;
                    else if (mousedesc == "x2") button = SDL_BUTTON_X2;

                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new MouseButtonHandler(button), key));
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding mouse button " + ss.str());
                    found = true;
                }
            }
            else if (token.find("joy") == 0)
            {
                std::string joydesc = Utils::replace(Utils::toLower(token), "joy", "");
                int joynum;
                if ( isdigit( joydesc.at( 0 ) ) )
                {
                    std::stringstream ssjoy;
                    ssjoy << joydesc.at(0);
                    ssjoy >> joynum;
                    joydesc = joydesc.erase(0, 1);
                }
                else
                {
                    joynum = -1;
                }

                if (joydesc.find("button") == 0)
                {
                    unsigned int button;
                    std::stringstream ss;
                    ss << Utils::replace(joydesc, "button", "");
                    ss >> button;
                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new JoyButtonHandler(joynum, button), key));
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad button " + ss.str());
                    found = true;
                }
                else if (joydesc.find("hat") == 0)
                {
                    Uint8 hat;

                    joydesc = Utils::replace(joydesc, "hat", "");
                    std::stringstream sshat;
                    sshat << joydesc.at(0);
                    int hatnum;
                    sshat >> hatnum;
                    joydesc = joydesc.erase(0, 1);

                    if (joydesc == "leftup") hat = SDL_HAT_LEFTUP;
                    else if (joydesc == "left") hat = SDL_HAT_LEFT;
                    else if (joydesc == "leftdown") hat = SDL_HAT_LEFTDOWN;
                    else if (joydesc == "up") hat = SDL_HAT_UP;
                    //else if(joydesc == "centered") hat = SDL_HAT_CENTERED;
                    else if (joydesc == "down") hat = SDL_HAT_DOWN;
                    else if (joydesc == "rightup") hat = SDL_HAT_RIGHTUP;
                    else if (joydesc == "right") hat = SDL_HAT_RIGHT;
                    else if (joydesc == "rightdown") hat = SDL_HAT_RIGHTDOWN;

                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new JoyHatHandler(joynum, hatnum, hat), key));
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad hat " + joydesc);
                    found = true;
                }
                else if (joydesc.find("axis") == 0)
                {
                    // string is now axis0+
                    unsigned int axis;
                    Sint16       min;
                    Sint16       max;
                    int          deadZone;

                    joydesc = Utils::replace(joydesc, "axis", "");

                    if (!config_.getProperty("controls.deadZone", deadZone))
                    {
                        deadZone = 3;
                    }

                    // string is now 0+
                    if (joydesc.find("-") != std::string::npos)
                    {
                        min = -32768;
                        max = -32768 / 100 * deadZone;
                        joydesc = Utils::replace(joydesc, "-", "");
                    }
                    else if (joydesc.find("+") != std::string::npos)
                    {
                        min = 32767 / 100 * deadZone;
                        max = 32767;
                        joydesc = Utils::replace(joydesc, "+", "");
                    }

                    // string is now just the axis number
                    std::stringstream ss;
                    ss << joydesc;
                    ss >> axis;
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad axis " + ss.str());
                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new JoyAxisHandler(joynum, axis, min, max), key));
                    found = true;
                }
            }

            if (!found)
            {
                Logger::write(Logger::ZONE_ERROR, "Input", "Unsupported property value for " + configKey + "(" + token + "). See Documentation/Keycodes.txt for valid inputs");
                success = false;
            }
        }
    }

    return success;
}

void UserInput::resetStates()
{
    for (unsigned int i = 0; i < keyHandlers_.size(); ++i)
    {
        if (keyHandlers_[i].first)
        {
            keyHandlers_[i].first->reset();
        }
        currentKeyState_[keyHandlers_[i].second] = false;
    }
}


bool UserInput::update( SDL_Event &e )
{
    bool updated = false;

    memcpy( lastKeyState_, currentKeyState_, sizeof( lastKeyState_ ) );
    memset( currentKeyState_, 0, sizeof( currentKeyState_ ) );

    // Handle adding a joystick
    if ( e.type == SDL_JOYDEVICEADDED )
    {
        SDL_JoystickID id = SDL_JoystickInstanceID( SDL_JoystickOpen( e.jdevice.which ) );
        for ( unsigned int i = 0; i < cMaxJoy; i++ )
        {
            if ( joysticks_[i] == -1 )
            {
                joysticks_[i] = id;
                break;
            }
        }
    }

    // Handle removing a joystick
    if ( e.type == SDL_JOYDEVICEREMOVED )
    {
        for ( unsigned int i = 0; i < cMaxJoy; i++ )
        {
            if ( joysticks_[i] == e.jdevice.which )
            {
                joysticks_[i] = -1;
                break;
            }
        }
        SDL_JoystickClose( SDL_JoystickFromInstanceID( e.jdevice.which ) );
    }

    // Remap joystick events
    if ( e.type == SDL_JOYAXISMOTION ||
         e.type == SDL_JOYBUTTONUP   ||
         e.type == SDL_JOYBUTTONDOWN ||
         e.type == SDL_JOYHATMOTION )
    {
        for ( unsigned int i = 0; i < cMaxJoy; i++ )
        {
            if ( joysticks_[i] == e.jdevice.which )
            {
                e.jdevice.which = i;
                e.jaxis.which   = i;
                e.jbutton.which = i;
                e.jhat.which    = i;
                break;
            }
        }
    }

    for ( unsigned int i = 0; i < keyHandlers_.size( ); ++i )
    {
        InputHandler *h = keyHandlers_[i].first;
        if ( h )
        {
            if ( h->update( e ) ) updated = true;

            currentKeyState_[keyHandlers_[i].second] |= h->pressed( );
        }
    }
    
    return updated;
}


bool UserInput::keystate(KeyCode_E code)
{
    return currentKeyState_[code];
}

bool UserInput::newKeyPressed(KeyCode_E code)
{
    return currentKeyState_[code] && !lastKeyState_[code];
}


void UserInput::clearJoysticks( )
{
    for ( unsigned int i = 0; i < cMaxJoy; i++ )
    {
        joysticks_[i] = -1;
    }
}


void UserInput::reconfigure( )
{
    for (unsigned int i = 0; i < keyHandlers_.size(); ++i)
    {
        if (keyHandlers_[i].first)
        {
            delete keyHandlers_[i].first;
        }
    }
    keyHandlers_.clear();
    initialize( );
}


void UserInput::updateKeystate( )
{
    for ( unsigned int i = 0; i < keyHandlers_.size( ); ++i )
    {
        InputHandler *h = keyHandlers_[i].first;
        if ( h )
        {
			h->updateKeystate( );
            currentKeyState_[keyHandlers_[i].second] |= h->pressed( );
        }
    }
}
