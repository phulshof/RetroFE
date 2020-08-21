#pragma once

#include "InputHandler.h"

class JoyButtonHandler : public InputHandler
{
public:
    JoyButtonHandler(SDL_JoystickID joynum, Uint8 button);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();
	void updateKeystate() {};

private:
    SDL_JoystickID joynum_;
    Uint8 button_;
    bool pressed_;
};

