#pragma once

#include "InputHandler.h"

class JoyAxisHandler : public InputHandler
{
public:
    JoyAxisHandler(SDL_JoystickID joyid, Uint8 axis, Sint16 min, Sint16 max);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();
	void updateKeystate() {};

private:
    SDL_JoystickID joyid_;
    Uint8 axis_;
    Sint16 min_;
    Sint16 max_;
        
    bool pressed_;
};

