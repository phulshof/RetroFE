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

#include "Component/Image.h"
#include "FontCache.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <rapidxml.hpp>
#include <vector>

static const int MENU_INDEX_HIGH = 16;

class ScrollingList;
class Page;
class ViewInfo;
class Configuration;
class Font;

class PageBuilder
{
public:
    PageBuilder(std::string layoutKey, std::string layoutPage, Configuration &c, FontCache *fc, bool isMenu = false);
    virtual ~PageBuilder();
    Page *buildPage( std::string collectionName = "" );

private:
    std::string layoutKey;
    std::string layoutPage;
    std::string layoutPath;
    Configuration &config_;
    int screenHeight_;
    int screenWidth_;
    int layoutHeight_;
    int layoutWidth_;
    int monitor_;
    SDL_Color fontColor_;
    std::string fontName_;
    int fontSize_;
    FontCache *fontCache_;
    bool isMenu_;

    Font *addFont(rapidxml::xml_node<> *component, rapidxml::xml_node<> *defaults);
    void loadReloadableImages(rapidxml::xml_node<> *layout, std::string tagName, Page *page);
    float getVerticalAlignment(rapidxml::xml_attribute<> *attribute, float valueIfNull);
    float getHorizontalAlignment(rapidxml::xml_attribute<> *attribute, float valueIfNull);
    void buildViewInfo(rapidxml::xml_node<> *componentXml, ViewInfo &info, rapidxml::xml_node<> *defaultXml = NULL);
    bool buildComponents(rapidxml::xml_node<> *layout, Page *page);
    void loadTweens(Component *c, rapidxml::xml_node<> *componentXml);
    AnimationEvents *createTweenInstance(rapidxml::xml_node<> *componentXml);
    void buildTweenSet(AnimationEvents *tweens, rapidxml::xml_node<> *componentXml, std::string tagName, std::string tweenName);
    ScrollingList * buildMenu(rapidxml::xml_node<> *menuXml, Page &p);
    void buildCustomMenu(ScrollingList *menu, rapidxml::xml_node<> *menuXml, rapidxml::xml_node<> *itemDefaults);
    void buildVerticalMenu(ScrollingList *menu, rapidxml::xml_node<> *menuXml, rapidxml::xml_node<> *itemDefaults);
    int parseMenuPosition(std::string strIndex);
    rapidxml::xml_attribute<> *findAttribute(rapidxml::xml_node<> *componentXml, std::string attribute, rapidxml::xml_node<> *defaultXml);
    void getTweenSet(rapidxml::xml_node<> *node, Animation *animation);
    void getAnimationEvents(rapidxml::xml_node<> *node, TweenSet &tweens);
    ViewInfo * createMenuItemInfo(rapidxml::xml_node<> *component, rapidxml::xml_node<> *defaults, float y);
};
