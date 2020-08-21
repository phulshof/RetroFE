#include "JoyHatHandler.h"

JoyHatHandler::JoyHatHandler(SDL_JoystickID joynum, Uint8 hatnum, Uint8 direction)
: joynum_(joynum)
, hatnum_(hatnum)
, direction_(direction)
, pressed_(false)
{
}

void JoyHatHandler::reset()
{
    pressed_= false;
}

bool JoyHatHandler::update(SDL_Event &e)
{
    if(e.type != SDL_JOYHATMOTION || (joynum_ != -1 && e.jhat.which != joynum_) || e.jhat.hat != hatnum_) return false;

    pressed_ = (e.jhat.value == direction_);
    return true;
}

bool JoyHatHandler::pressed()
{
    return pressed_;
}

