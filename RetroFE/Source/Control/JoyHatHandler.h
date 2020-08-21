#pragma once

#include "InputHandler.h"

class JoyHatHandler : public InputHandler
{
public:
    JoyHatHandler(SDL_JoystickID joynum, Uint8 hatnum, Uint8 direction);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();
	void updateKeystate() {};

private:
    SDL_JoystickID joynum_;
    Uint8 hatnum_;
    Uint8 direction_;
    bool pressed_;
};

