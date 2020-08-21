#include "MouseButtonHandler.h"

MouseButtonHandler::MouseButtonHandler(Uint8 button)
: button_(button)
, pressed_(false)
{
}

void MouseButtonHandler::reset()
{
    pressed_= false;
}

bool MouseButtonHandler::update(SDL_Event &e)
{
    if(e.type != SDL_MOUSEBUTTONUP && e.type != SDL_MOUSEBUTTONDOWN) return false;

    if(e.button.button == button_)
    {
        pressed_ = (e.type == SDL_MOUSEBUTTONDOWN) ? true : false;
        return true;
    }

    return false;
}

bool MouseButtonHandler::pressed()
{
    return pressed_;
}

