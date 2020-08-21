#include "KeyboardHandler.h"

KeyboardHandler::KeyboardHandler(SDL_Scancode s)
: scancode_(s)
, pressed_(false)
{
}

void KeyboardHandler::reset()
{
    pressed_= false;
}

bool KeyboardHandler::update(SDL_Event &e)
{
    if(e.type != SDL_KEYUP && e.type != SDL_KEYDOWN) return false;

    if(e.key.keysym.scancode == scancode_) 
    {
        pressed_ = (e.type == SDL_KEYDOWN);
        return true;
    }

    return false;
}

bool KeyboardHandler::pressed()
{
    return pressed_;
}

void KeyboardHandler::updateKeystate()
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	pressed_ = state[scancode_];
}
