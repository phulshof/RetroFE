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
#include "../../Collection/Item.h"
#include <SDL2/SDL.h>
#include <vector>
#include <string>


class ReloadableScrollingText : public Component
{
public:
    ReloadableScrollingText(Configuration &config, bool systemMode, bool layoutMode, bool menuMode, std::string type, std::string textFormat, std::string singlePrefix, std::string singlePostfix, std::string pluralPrefix, std::string pluralPostfix, std::string alignment, Page &page, int displayOffset, Font *font, std::string direction, float scrollingSpeed, float startPosition, float startTime, float endTime );
    virtual ~ReloadableScrollingText( );
    void     update(float dt);
    void     draw( );
    void     allocateGraphicsMemory( );
    void     freeGraphicsMemory( );
    void     deInitializeFonts();
    void     initializeFonts();

private:
    void reloadTexture( );
    void loadText( std::string collection, std::string type, std::string basename, std::string filepath, bool systemMode );
    Configuration           &config_;
    bool                     systemMode_;
    bool                     layoutMode_;
    bool                     menuMode_;
    Font                    *fontInst_;
    std::string              type_;
    std::string              textFormat_;
    std::string              singlePrefix_;
    std::string              singlePostfix_;
    std::string              pluralPrefix_;
    std::string              pluralPostfix_;
    std::string              alignment_;
    std::vector<std::string> text_;
    std::string              direction_;
    float                    scrollingSpeed_;
    float                    startPosition_;
    float                    currentPosition_;
    float                    startTime_;
    float                    waitStartTime_;
    float                    endTime_;
    float                    waitEndTime_;
    std::string              currentCollection_;
    Page                    *page_;
    int                      displayOffset_;
    int                      textWidth_;
};
