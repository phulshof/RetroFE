#pragma once

#include "InputHandler.h"

class MouseButtonHandler : public InputHandler
{
public:
    MouseButtonHandler(Uint8 button);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();
	void updateKeystate() {};

private:
    Uint8 button_;
    bool pressed_;
};

