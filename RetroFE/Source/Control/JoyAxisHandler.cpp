#include "JoyAxisHandler.h"

JoyAxisHandler::JoyAxisHandler(SDL_JoystickID joyid, Uint8 axis, Sint16 min, Sint16 max)
: joyid_(joyid)
, axis_(axis)
, min_(min)
, max_(max)
, pressed_(false)
{
}

void JoyAxisHandler::reset()
{
    pressed_= false;
}

bool JoyAxisHandler::update(SDL_Event &e)
{
    if(e.type != SDL_JOYAXISMOTION || (joyid_ != -1 && e.jaxis.which != joyid_) || e.jaxis.axis != axis_) return false;
    pressed_ = (min_ <= e.jaxis.value && e.jaxis.value <= max_);

    return true;
}

bool JoyAxisHandler::pressed()
{
    return pressed_;
}

