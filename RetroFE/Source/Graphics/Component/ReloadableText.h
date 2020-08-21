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
#pragma once
#include "Component.h"
#include "Text.h"
#include "../Font.h"
#include "../Page.h"
#include "../../Collection/Item.h"
#include <SDL2/SDL.h>
#include <string>

class ReloadableText : public Component
{
public:
    ReloadableText(std::string type, Page &page, Configuration &config, bool systemMode, Font *font, std::string layoutKey, std::string timeFormat, std::string textFormat, std::string singlePrefix, std::string singlePostfix, std::string pluralPrefix, std::string pluralPostfix);
    virtual ~ReloadableText();
    void     update(float dt);
    void     draw();
    void     freeGraphicsMemory();
    void     allocateGraphicsMemory();
    void     deInitializeFonts();
    void     initializeFonts();

private:
    void ReloadTexture();

    Configuration &config_;
    bool systemMode_;
    Text *imageInst_;
    std::string type_;
    std::string layoutKey_;
    Font *fontInst_;
    std::string timeFormat_;
    std::string textFormat_;
    std::string singlePrefix_;
    std::string singlePostfix_;
    std::string pluralPrefix_;
    std::string pluralPostfix_;
};
