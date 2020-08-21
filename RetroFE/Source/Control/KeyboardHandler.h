#pragma once

#include "InputHandler.h"

class KeyboardHandler : public InputHandler
{
public:
    KeyboardHandler(SDL_Scancode scancode);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();
	void updateKeystate( );

private:
    SDL_Scancode scancode_;
    bool pressed_;
};

