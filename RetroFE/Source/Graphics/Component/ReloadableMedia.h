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
#include "ReloadableText.h"
#include "../../Video/IVideo.h"
#include "../../Collection/Item.h"
#include <SDL2/SDL.h>
#include <string>

class Image;

//todo: this class should aggregate Image, Text, and Video component classes
class ReloadableMedia : public Component
{
public:
    ReloadableMedia(Configuration &config, bool systemMode, bool layoutMode, bool commonMode, bool menuMode, std::string type, std::string imageType, Page &page, int displayOffset, bool isVideo, Font *font, bool jukebox, int jukeboxNumLoops);
    virtual ~ReloadableMedia();
    void update(float dt);
    void draw();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    Component *findComponent(std::string collection, std::string type, std::string basename, std::string filepath, bool systemMode, bool isVideo);

    void enableTextFallback_(bool value);
    virtual bool isJukeboxPlaying();


private:
    void reloadTexture();
    Configuration &config_;
    bool systemMode_;
    bool layoutMode_;
    bool commonMode_;
    bool menuMode_;
    Component *loadedComponent_;
    IVideo *videoInst_;
    bool isVideo_;
    Font *FfntInst_;
    bool textFallback_;
    std::string type_;
    std::string currentCollection_;
    Page *page_;
    int displayOffset_;
    std::string imageType_;
    bool jukebox_;
    int  jukeboxNumLoops_;
};
