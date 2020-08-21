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


#include "Menu.h"
#include "../Collection/Item.h"
#include "../Control/UserInput.h"
#include "../Database/Configuration.h"
#include <SDL2/SDL.h>
#include <iostream>


Menu::Menu( Configuration &c, UserInput &ui )
    : config_( c ),
      input_( ui )
{
    page_ = nullptr;
}


void Menu::handleEntry( Item *item )
{
    std::cout << "Handling " + item->ctrlType + "." << std::endl;
    std::string key  = getKey();
    std::string ctrl = item->ctrlType;
    ctrl.erase( 0, 1 );
    std::cout << "Setting " + ctrl + " = " + key << std::endl;
    config_.setProperty( "controls." + ctrl, key );
    input_.reconfigure();
    return;
}


void Menu::setPage( Page *page )
{
    page_ = page;
}


void Menu::clearPage( )
{
    page_ = nullptr;
}


std::string Menu::getKey()
{

    SDL_Event   event;
    std::string return_value = "";

    while ( return_value == "" )
    {
        while ( SDL_PollEvent( &event ) )
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                    if ( return_value.empty() )
                        return_value = SDL_GetKeyName( event.key.keysym.sym);
                    break;
                case SDL_JOYBUTTONDOWN:
                    if ( return_value.empty() )
                        return_value = "joyButton" + std::to_string( int( event.jbutton.button ) );
                    break;
                case SDL_JOYAXISMOTION:
                    if ((event.jaxis.value > 30000 || event.jaxis.value < -30000) && event.jaxis.axis <= 3)
                    {
                        if ( event.jaxis.value > 0 )
                        {
                            if ( return_value.empty() )
                                return_value = "joyAxis" + std::to_string( int( event.jaxis.axis ) ) + "+";
                        }
                        else
                        {
                            if ( return_value.empty() )
                                return_value = "joyAxis" + std::to_string( int( event.jaxis.axis ) ) + "-";
                        }
                    }
                    break;
                case SDL_JOYHATMOTION:
                    switch( event.jhat.value )
                    {
                        case SDL_HAT_UP:
                            if ( return_value.empty() )
                                return_value = "joyHat" + std::to_string( int( event.jhat.hat ) ) + "Up";
                            break;
                        case SDL_HAT_DOWN:
                            if ( return_value.empty() )
                                return_value = "joyHat" + std::to_string( int( event.jhat.hat ) ) + "Down";
                            break;
                        case SDL_HAT_LEFT:
                            if ( return_value.empty() )
                                return_value = "joyHat" + std::to_string( int( event.jhat.hat ) ) + "Left";
                            break;
                        case SDL_HAT_RIGHT:
                            if ( return_value.empty() )
                                return_value = "joyHat" + std::to_string( int( event.jhat.hat ) ) + "Right";
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return return_value;

}
