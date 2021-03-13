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
#include "Image.h"
#include "../ViewInfo.h"
#include "../../SDL.h"
#include "../../Utility/Log.h"
#include <SDL2/SDL_image.h>

Image::Image(std::string file, std::string altFile, Page &p, int monitor)
    : Component(p)
    , texture_(NULL)
    , file_(file)
    , altFile_(altFile)
{
    baseViewInfo.Monitor = monitor;
    allocateGraphicsMemory();
}

Image::~Image()
{
    freeGraphicsMemory();
}

void Image::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();

    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        SDL_DestroyTexture(texture_);
        texture_ = NULL;
    }
    SDL_UnlockMutex(SDL::getMutex());
}

void Image::allocateGraphicsMemory()
{
    int width;
    int height;

    if(!texture_)
    {
        SDL_LockMutex(SDL::getMutex());
        texture_ = IMG_LoadTexture(SDL::getRenderer(baseViewInfo.Monitor), file_.c_str());
        if (!texture_ && altFile_ != "")
        {
            texture_ = IMG_LoadTexture(SDL::getRenderer(baseViewInfo.Monitor), altFile_.c_str());
        }

        if (texture_ != NULL)
        {
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
            SDL_QueryTexture(texture_, NULL, NULL, &width, &height);
            baseViewInfo.ImageWidth  = (float)width;
            baseViewInfo.ImageHeight = (float)height;
        }
        SDL_UnlockMutex(SDL::getMutex());

    }

    Component::allocateGraphicsMemory();

}


void Image::draw()
{
    Component::draw();

    if(texture_)
    {
        SDL_Rect rect;

        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

        SDL::renderCopy(texture_, baseViewInfo.Alpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));
    }
}
