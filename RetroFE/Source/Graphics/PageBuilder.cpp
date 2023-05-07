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

#include "PageBuilder.h"
#include "Page.h"
#include "ViewInfo.h"
#include "Component/Container.h"
#include "Component/Image.h"
#include "Component/Text.h"
#include "Component/ReloadableText.h"
#include "Component/ReloadableMedia.h"
#include "Component/ReloadableScrollingText.h"
#include "Component/ScrollingList.h"
#include "Component/Video.h"
#include "Animate/AnimationEvents.h"
#include "Animate/TweenTypes.h"
#include "../Sound/Sound.h"
#include "../Collection/Item.h"
#include "../SDL.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include <algorithm>
#include <cfloat>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>

using namespace rapidxml;

static const int MENU_FIRST = 0;   // first visible item in the list
static const int MENU_LAST = -3;   // last visible item in the list
static const int MENU_START = -1;  // first item transitions here after it scrolls "off the menu/screen"
static const int MENU_END = -2;    // last item transitions here after it scrolls "off the menu/screen"
static const int MENU_CENTER = -4;

//todo: this file is starting to become a god class of building. Consider splitting into sub-builders
PageBuilder::PageBuilder(std::string layoutKey, std::string layoutPage, Configuration &c, FontCache *fc, bool isMenu)
    : layoutKey(layoutKey)
    , layoutPage(layoutPage)
    , config_(c)
    , screenHeight_(0)
    , screenWidth_(0)
    , layoutHeight_(0)
    , layoutWidth_(0)
    , fontSize_(24)
    , fontCache_(fc)
    , isMenu_(isMenu)
{
    screenWidth_  = SDL::getWindowWidth(0);
    screenHeight_ = SDL::getWindowHeight(0);
    monitor_      = 0;
    fontColor_.a  = 255;
    fontColor_.r  = 0;
    fontColor_.g  = 0;
    fontColor_.b  = 0;
}

PageBuilder::~PageBuilder()
{
}

Page *PageBuilder::buildPage( std::string collectionName )
{
    Page *page = NULL;

    std::string layoutFile;
    std::string layoutFileAspect;
    std::string layoutName = layoutKey;

    if ( isMenu_ )
    {
        layoutPath = Utils::combinePath(Configuration::absolutePath, "menu");
    }
    else if ( collectionName == "" )
    {
        layoutPath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName);
    }
    else
    {
        layoutPath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", collectionName);
        layoutPath = Utils::combinePath(layoutPath, "layout");
    }

    std::vector<std::string> monitors;
    monitors.push_back("");
    for ( int i = 0; i < SDL::getNumScreens(); i++ )
        monitors.push_back( " - " + std::to_string( i ) );
   

    for ( size_t monitor = 0; monitor < monitors.size(); monitor++ )
    {
		if ( monitor > 0 )
            monitor_ = monitor - 1;
		else
            monitor_ = monitor;
        layoutFile       = Utils::combinePath(layoutPath, layoutPage + monitors[monitor] + ".xml");
        layoutFileAspect = Utils::combinePath(layoutPath, layoutPage + " " + std::to_string( screenWidth_/Utils::gcd( screenWidth_, screenHeight_ ) ) + "x" + std::to_string( screenHeight_/Utils::gcd( screenWidth_, screenHeight_ ) ) + monitors[monitor] + ".xml" );

        Logger::write(Logger::ZONE_INFO, "Layout", "Initializing " + layoutFileAspect);

        rapidxml::xml_document<> doc;
        std::ifstream file(layoutFileAspect.c_str());

        if ( !file.good( ) )
        {
            Logger::write( Logger::ZONE_INFO, "Layout", "could not find layout file: " + layoutFileAspect );
            Logger::write( Logger::ZONE_INFO, "Layout", "Initializing " + layoutFile );
            file.open( layoutFile.c_str( ) );
            if ( !file.good( ) )
            {
                Logger::write( Logger::ZONE_INFO, "Layout", "could not find layout file: " + layoutFile );
                continue;
            }
        }

        std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        try
        {
            buffer.push_back('\0');

            doc.parse<0>(&buffer[0]);

            xml_node<> *root = doc.first_node("layout");


            if(!root)
            {
                Logger::write(Logger::ZONE_ERROR, "Layout", "Missing <layout> tag");
                return NULL;
            }
            else
            {
                xml_attribute<> *layoutWidthXml = root->first_attribute("width");
                xml_attribute<> *layoutHeightXml = root->first_attribute("height");
                xml_attribute<> *fontXml = root->first_attribute("font");
                xml_attribute<> *fontColorXml = root->first_attribute("fontColor");
                xml_attribute<> *fontSizeXml = root->first_attribute("loadFontSize");
                xml_attribute<> *minShowTimeXml = root->first_attribute("minShowTime");

                if(!layoutWidthXml || !layoutHeightXml)
                {
                    Logger::write(Logger::ZONE_ERROR, "Layout", "<layout> tag must specify a width and height");
                    return NULL;
                }
                if(fontXml)
                {
                    fontName_ = config_.convertToAbsolutePath(
                               Utils::combinePath(config_.absolutePath, "layouts", layoutKey, ""),
                               fontXml->value());
                }

                if(fontColorXml)
                {
                    int intColor = 0;
                    std::stringstream ss;
                    ss << std::hex << fontColorXml->value();
                    ss >> intColor;

                    fontColor_.b = intColor & 0xFF;
                    intColor >>= 8;
                    fontColor_.g = intColor & 0xFF;
                    intColor >>= 8;
                    fontColor_.r = intColor & 0xFF;
                }

                if(fontSizeXml)
                {
                    fontSize_ = Utils::convertInt(fontSizeXml->value());
                }

                layoutWidth_ = Utils::convertInt(layoutWidthXml->value());
                layoutHeight_ = Utils::convertInt(layoutHeightXml->value());

                if(layoutWidth_ == 0 || layoutHeight_ == 0)
                {
                    Logger::write(Logger::ZONE_ERROR, "Layout", "Layout width and height cannot be set to 0");
                    return NULL;
                }

                std::stringstream ss;
                ss << layoutWidth_ << "x" << layoutHeight_ << " (scale " << (float)screenWidth_ / (float)layoutWidth_ << "x" << (float)screenHeight_ / (float)layoutHeight_ << ")";
                Logger::write(Logger::ZONE_INFO, "Layout", "Layout resolution " + ss.str());

                if ( !page )
                    page = new Page(config_, layoutWidth_, layoutHeight_ );
                else
                {
                    page->setLayoutWidth( monitor,  layoutWidth_ );
                    page->setLayoutHeight( monitor, layoutHeight_ ); 
                }

                if(minShowTimeXml) 
                {
                    page->setMinShowTime(Utils::convertFloat(minShowTimeXml->value()));
                }

                // load sounds
                for(xml_node<> *sound = root->first_node("sound"); sound; sound = sound->next_sibling("sound"))
                {
                    xml_attribute<> *src  = sound->first_attribute("src");
                    xml_attribute<> *type = sound->first_attribute("type");
                    std::string file      = Configuration::convertToAbsolutePath(layoutPath, src->value());
                    std::string layoutName;
                    config_.getProperty("layout", layoutName);
                    std::string altfile   = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, std::string(src->value()));
                    if(!type)
                    {
                        Logger::write(Logger::ZONE_ERROR, "Layout", "Sound tag missing type attribute");
                    }
                    else
                    {
                        Sound *sound = new Sound(file, altfile);
                        std::string soundType = type->value();

                        if(!soundType.compare("load"))
                        {
                            page->setLoadSound(sound);
                        }
                        else if(!soundType.compare("unload"))
                        {
                            page->setUnloadSound(sound);
                        }
                        else if(!soundType.compare("highlight"))
                        {
                            page->setHighlightSound(sound);
                        }
                        else if(!soundType.compare("select"))
                        {
                            page->setSelectSound(sound);
                        }
                        else
                        {
                            Logger::write(Logger::ZONE_WARNING, "Layout", "Unsupported sound effect type \"" + soundType + "\"");
                        }
                    }
                }

                if(!buildComponents(root, page))
                {
                    delete page;
                    page = NULL;
                }

            }

        }
        catch(rapidxml::parse_error &e)
        {
            std::string what = e.what();
            long line = static_cast<long>(std::count(&buffer.front(), e.where<char>(), char('\n')) + 1);
            std::stringstream ss;
            ss << "Could not parse layout file. [Line: " << line << "] Reason: " << e.what();

            Logger::write(Logger::ZONE_ERROR, "Layout", ss.str());
        }
        catch(std::exception &e)
        {
            std::string what = e.what();
            Logger::write(Logger::ZONE_ERROR, "Layout", "Could not parse layout file. Reason: " + what);
        }

        if(page)
        {
            Logger::write(Logger::ZONE_INFO, "Layout", "Initialized");
        }
        else
        {
            Logger::write(Logger::ZONE_ERROR, "Layout", "Could not initialize layout (see previous messages for reason)");
        }
    }

    return page;
}



float PageBuilder::getHorizontalAlignment(xml_attribute<> *attribute, float valueIfNull)
{
    float value;
    std::string str;

    if(!attribute)
    {
        value = valueIfNull;
    }
    else
    {
        str = attribute->value();

        if(!str.compare("left"))
        {
            value = 0;
        }
        else if(!str.compare("center"))
        {
            value = static_cast<float>(layoutWidth_) / 2;
        }
        else if(!str.compare("right") || !str.compare("stretch"))
        {
            value = static_cast<float>(layoutWidth_);
        }
        else
        {
            value = Utils::convertFloat(str);
        }
    }

    return value;
}

float PageBuilder::getVerticalAlignment(xml_attribute<> *attribute, float valueIfNull)
{
    float value;
    std::string str;
    if(!attribute)
    {
        value = valueIfNull;
    }
    else
    {
        str = attribute->value();

        if(!str.compare("top"))
        {
            value = 0;
        }
        else if(!str.compare("center"))
        {
            value = static_cast<float>(layoutHeight_ / 2);
        }
        else if(!str.compare("bottom") || !str.compare("stretch"))
        {
            value = static_cast<float>(layoutHeight_);
        }
        else
        {
            value = Utils::convertFloat(str);
        }
    }
    return value;
}




bool PageBuilder::buildComponents(xml_node<> *layout, Page *page)
{
    for(xml_node<> *componentXml = layout->first_node("menu"); componentXml; componentXml = componentXml->next_sibling("menu"))
    {
        ScrollingList *scrollingList = buildMenu(componentXml,*page);
        xml_attribute<> *indexXml = componentXml->first_attribute("menuIndex");
        int index = indexXml ? Utils::convertInt(indexXml->value()) : -1;
        if (scrollingList->playlistType_) {
            page->playlistMenu_ = scrollingList;
        }
        page->pushMenu(scrollingList, index);
    }

    for(xml_node<> *componentXml = layout->first_node("container"); componentXml; componentXml = componentXml->next_sibling("container"))
    {
        Container *c = new Container(*page);
        xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
        if (menuScrollReload &&
            (Utils::toLower(menuScrollReload->value()) == "true" ||
             Utils::toLower(menuScrollReload->value()) == "yes"))
        {
            c->setMenuScrollReload(true);
        }
        buildViewInfo(componentXml, c->baseViewInfo);
        loadTweens(c, componentXml);
        page->addComponent(c);
    }


    for(xml_node<> *componentXml = layout->first_node("image"); componentXml; componentXml = componentXml->next_sibling("image"))
    {
        xml_attribute<> *src        = componentXml->first_attribute("src");
        xml_attribute<> *idXml      = componentXml->first_attribute("id");
        xml_attribute<> *monitorXml = componentXml->first_attribute("monitor");

        int id = -1;
        if (idXml)
        {
            id = Utils::convertInt(idXml->value());
        }

        if (!src)
        {
            Logger::write(Logger::ZONE_ERROR, "Layout", "Image component in layout does not specify a source image file");
        }
        else
        {
            std::string imagePath;
            imagePath = Utils::combinePath(Configuration::convertToAbsolutePath(layoutPath, imagePath), std::string(src->value()));
            std::string layoutName;
            config_.getProperty("layout", layoutName);
            std::string altImagePath;
            altImagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, std::string(src->value()));

            int monitor = monitorXml ? Utils::convertInt(monitorXml->value()) : monitor_;
            Image *c = new Image(imagePath, altImagePath, *page, monitor);
            c->setId( id );
            xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
            if (menuScrollReload &&
                (Utils::toLower(menuScrollReload->value()) == "true" ||
                 Utils::toLower(menuScrollReload->value()) == "yes"))
            {
                c->setMenuScrollReload(true);
            }
            buildViewInfo(componentXml, c->baseViewInfo);
            loadTweens(c, componentXml);
            page->addComponent(c);
        }
    }


    for(xml_node<> *componentXml = layout->first_node("video"); componentXml; componentXml = componentXml->next_sibling("video"))
    {
        xml_attribute<> *srcXml      = componentXml->first_attribute("src");
        xml_attribute<> *numLoopsXml = componentXml->first_attribute("numLoops");
        xml_attribute<> *idXml = componentXml->first_attribute("id");
        xml_attribute<> *monitorXml = componentXml->first_attribute("monitor");

        int id = -1;
        if (idXml)
        {
            id = Utils::convertInt(idXml->value());
        }

        if (!srcXml)
        {
            Logger::write(Logger::ZONE_ERROR, "Layout", "Video component in layout does not specify a source video file");
        }
        else
        {
            std::string videoPath;
            videoPath = Utils::combinePath(Configuration::convertToAbsolutePath(layoutPath, videoPath), std::string(srcXml->value()));
            std::string layoutName;
            config_.getProperty("layout", layoutName);
            std::string altVideoPath;
            altVideoPath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, std::string(srcXml->value()));
            int numLoops = numLoopsXml ? Utils::convertInt(numLoopsXml->value()) : 1;

            int monitor = monitorXml ? Utils::convertInt(monitorXml->value()) : monitor_;
            Video *c = new Video(videoPath, altVideoPath, numLoops, *page, monitor);
            c->setId( id );
            xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
            if (menuScrollReload &&
                (Utils::toLower(menuScrollReload->value()) == "true" ||
                 Utils::toLower(menuScrollReload->value()) == "yes"))
            {
                c->setMenuScrollReload(true);
            }
            buildViewInfo(componentXml, c->baseViewInfo);
            loadTweens(c, componentXml);
            page->addComponent(c);
        }
    }


    for(xml_node<> *componentXml = layout->first_node("text"); componentXml; componentXml = componentXml->next_sibling("text"))
    {
        xml_attribute<> *value      = componentXml->first_attribute("value");
        xml_attribute<> *idXml      = componentXml->first_attribute("id");
        xml_attribute<> *monitorXml = componentXml->first_attribute("monitor");

        int id = -1;
        if (idXml)
        {
            id = Utils::convertInt(idXml->value());
        }

        if (!value)
        {
            Logger::write(Logger::ZONE_WARNING, "Layout", "Text component in layout does not specify a value");
        }
        else
        {
            Font *font = addFont(componentXml, NULL);
            int monitor = monitorXml ? Utils::convertInt(monitorXml->value()) : monitor_;
            Text *c = new Text(value->value(), *page, font, monitor);
            c->setId( id );
            xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
            if (menuScrollReload &&
                (Utils::toLower(menuScrollReload->value()) == "true" ||
                 Utils::toLower(menuScrollReload->value()) == "yes"))
            {
                c->setMenuScrollReload(true);
            }
            buildViewInfo(componentXml, c->baseViewInfo);
            loadTweens(c, componentXml);
            page->addComponent(c);
        }
    }

    for(xml_node<> *componentXml = layout->first_node("statusText"); componentXml; componentXml = componentXml->next_sibling("statusText"))
    {
        xml_attribute<> *monitorXml = componentXml->first_attribute("monitor");
        Font *font = addFont(componentXml, NULL);
        int monitor = monitorXml ? Utils::convertInt(monitorXml->value()) : monitor_;
        Text *c = new Text("", *page, font, monitor);
        xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
        if (menuScrollReload &&
            (Utils::toLower(menuScrollReload->value()) == "true" ||
             Utils::toLower(menuScrollReload->value()) == "yes"))
        {
            c->setMenuScrollReload(true);
        }
        buildViewInfo(componentXml, c->baseViewInfo);
        loadTweens(c, componentXml);
        page->addComponent(c);
        page->setStatusTextComponent(c);
    }


    loadReloadableImages(layout, "reloadableImage",         page);
    loadReloadableImages(layout, "reloadableAudio",         page);
    loadReloadableImages(layout, "reloadableVideo",         page);
    loadReloadableImages(layout, "reloadableText",          page);
    loadReloadableImages(layout, "reloadableScrollingText", page);

    return true;
}

void PageBuilder::loadReloadableImages(xml_node<> *layout, std::string tagName, Page *page)
{
    
    for(xml_node<> *componentXml = layout->first_node(tagName.c_str()); componentXml; componentXml = componentXml->next_sibling(tagName.c_str()))
    {
        std::string reloadableImagePath;
        std::string reloadableVideoPath;
        xml_attribute<> *type              = componentXml->first_attribute("type");
        xml_attribute<> *imageType         = componentXml->first_attribute("imageType");
        xml_attribute<> *mode              = componentXml->first_attribute("mode");
        xml_attribute<> *timeFormatXml     = componentXml->first_attribute("timeFormat");
        xml_attribute<> *textFormatXml     = componentXml->first_attribute("textFormat");
        xml_attribute<> *singlePrefixXml   = componentXml->first_attribute("singlePrefix");
        xml_attribute<> *singlePostfixXml  = componentXml->first_attribute("singlePostfix");
        xml_attribute<> *pluralPrefixXml   = componentXml->first_attribute("pluralPrefix");
        xml_attribute<> *pluralPostfixXml  = componentXml->first_attribute("pluralPostfix");
        xml_attribute<> *selectedOffsetXml = componentXml->first_attribute("selectedOffset");
        xml_attribute<> *directionXml      = componentXml->first_attribute("direction");
        xml_attribute<> *scrollingSpeedXml = componentXml->first_attribute("scrollingSpeed");
        xml_attribute<> *startPositionXml  = componentXml->first_attribute("startPosition");
        xml_attribute<> *startTimeXml      = componentXml->first_attribute("startTime");
        xml_attribute<> *endTimeXml        = componentXml->first_attribute("endTime");
        xml_attribute<> *alignmentXml      = componentXml->first_attribute("alignment");
        xml_attribute<> *idXml             = componentXml->first_attribute("id");
        bool systemMode = false;
        bool layoutMode = false;
        bool commonMode = false;
        bool menuMode   = false;
        int selectedOffset = 0;

        int id = -1;
        if (idXml)
        {
            id = Utils::convertInt(idXml->value());
        }

        if(!imageType && (tagName == "reloadableVideo" || tagName == "reloadableAudio"))
        {
            Logger::write(Logger::ZONE_WARNING, "Layout", "<reloadableImage> component in layout does not specify an imageType for when the video does not exist");
        }
        if(!type && (tagName == "reloadableImage" || tagName == "reloadableText"))
        {
            Logger::write(Logger::ZONE_ERROR, "Layout", "Image component in layout does not specify a source image file");
        }
        if(!type && tagName == "reloadableScrollingText")
        {
            Logger::write(Logger::ZONE_ERROR, "Layout", "Reloadable scroling text component in layout does not specify a type");
        }


        if(mode)
        {
            std::string sysMode = mode->value();
            if(sysMode == "system")
            {
                systemMode = true;
            }
            if(sysMode == "layout")
            {
                layoutMode = true;
            }
            if(sysMode == "common")
            {
                commonMode = true;
            }
            if(sysMode == "commonlayout")
            {
                layoutMode = true;
                commonMode = true;
            }
            if(sysMode == "systemlayout")
            {
                systemMode = true;
                layoutMode = true;
            }
            if(sysMode == "menu")
            {
                menuMode = true;
            }
        }

        if(selectedOffsetXml) 
        {
            std::stringstream ss;
            ss << selectedOffsetXml->value();
            ss >> selectedOffset;
        }


        Component *c = NULL;

        if(tagName == "reloadableText")
        {
            if(type)
            {
                Font *font = addFont(componentXml, NULL);
                std::string timeFormat = "%H:%M";
                if (timeFormatXml)
                {
                    timeFormat = timeFormatXml->value();
                }
                std::string textFormat = "";
                if (textFormatXml)
                {
                    textFormat = textFormatXml->value();
                }
                std::string singlePrefix = "";
                if (singlePrefixXml)
                {
                    singlePrefix = singlePrefixXml->value();
                }
                std::string singlePostfix = "";
                if (singlePostfixXml)
                {
                    singlePostfix = singlePostfixXml->value();
                }
                std::string pluralPrefix = "";
                if (pluralPrefixXml)
                {
                    pluralPrefix = pluralPrefixXml->value();
                }
                std::string pluralPostfix = "";
                if (pluralPostfixXml)
                {
                    pluralPostfix = pluralPostfixXml->value();
                }
                c = new ReloadableText(type->value(), *page, config_, systemMode, font, layoutKey, timeFormat, textFormat, singlePrefix, singlePostfix, pluralPrefix, pluralPostfix);
                c->setId( id );
                xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
                if (menuScrollReload &&
                    (Utils::toLower(menuScrollReload->value()) == "true" ||
                     Utils::toLower(menuScrollReload->value()) == "yes"))
                {
                    c->setMenuScrollReload(true);
                }
            }
        }
        else if(tagName == "reloadableScrollingText")
        {
            if(type)
            {
                Font *font = addFont(componentXml, NULL);
                std::string direction = "horizontal";
                std::string textFormat = "";
                if (textFormatXml)
                {
                    textFormat = textFormatXml->value();
                }
                if (directionXml)
                {
                    direction = directionXml->value();
                }
                float scrollingSpeed = 1.0f;
                if (scrollingSpeedXml)
                {
                    scrollingSpeed = Utils::convertFloat(scrollingSpeedXml->value());
                }
                float startPosition = 0.0f;
                if (startPositionXml)
                {
                    startPosition = Utils::convertFloat(startPositionXml->value());
                }
                float startTime = 0.0f;
                if (startTimeXml)
                {
                    startTime = Utils::convertFloat(startTimeXml->value());
                }
                float endTime = 0.0f;
                if (endTimeXml)
                {
                    endTime = Utils::convertFloat(endTimeXml->value());
                }
                std::string alignment = "";
                if (alignmentXml)
                {
                    alignment = alignmentXml->value();
                }
                
                std::string singlePrefix = "";
                if (singlePrefixXml)
                {
                    singlePrefix = singlePrefixXml->value();
                }
                std::string singlePostfix = "";
                if (singlePostfixXml)
                {
                    singlePostfix = singlePostfixXml->value();
                }
                std::string pluralPrefix = "";
                if (pluralPrefixXml)
                {
                    pluralPrefix = pluralPrefixXml->value();
                }
                std::string pluralPostfix = "";
                if (pluralPostfixXml)
                {
                    pluralPostfix = pluralPostfixXml->value();
                }
                c = new ReloadableScrollingText(config_, systemMode, layoutMode, menuMode, type->value(), singlePrefix, singlePostfix, pluralPrefix, pluralPostfix, textFormat, alignment, *page, selectedOffset, font, direction, scrollingSpeed, startPosition, startTime, endTime);
                c->setId( id );
                xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
                if (menuScrollReload &&
                    (Utils::toLower(menuScrollReload->value()) == "true" ||
                     Utils::toLower(menuScrollReload->value()) == "yes"))
                {
                    c->setMenuScrollReload(true);
                }
            }
        }
        else
        {
            xml_attribute<> *jukeboxXml = componentXml->first_attribute("jukebox");
            bool jukebox         = false;
            int  jukeboxNumLoops = 0;
            if(jukeboxXml &&
               (Utils::toLower(jukeboxXml->value()) == "true" ||
                Utils::toLower(jukeboxXml->value()) == "yes"))
            {
                jukebox = true;
                page->setJukebox();
                xml_attribute<> *numLoopsXml = componentXml->first_attribute("jukeboxNumLoops");
                jukeboxNumLoops = numLoopsXml ? Utils::convertInt(numLoopsXml->value()) : 1;
            }
            Font *font = addFont(componentXml, NULL);
            std::string typeString      = "video";
            std::string imageTypeString = "";
            if ( type )
                typeString = type->value();
            if ( imageType )
                imageTypeString = imageType->value();
            c = new ReloadableMedia(config_, systemMode, layoutMode, commonMode, menuMode, typeString, imageTypeString, *page, selectedOffset, (tagName == "reloadableVideo") || (tagName == "reloadableAudio"), font, jukebox, jukeboxNumLoops);
            c->setId( id );
            xml_attribute<> *menuScrollReload = componentXml->first_attribute("menuScrollReload");
            if (menuScrollReload &&
                (Utils::toLower(menuScrollReload->value()) == "true" ||
                 Utils::toLower(menuScrollReload->value()) == "yes"))
            {
                c->setMenuScrollReload(true);
            }
            xml_attribute<> *textFallback = componentXml->first_attribute("textFallback");
            if(textFallback && Utils::toLower(textFallback->value()) == "true")
            {
                static_cast<ReloadableMedia *>(c)->enableTextFallback_(true);
            }
            else
            {
                static_cast<ReloadableMedia *>(c)->enableTextFallback_(false);
            }
        }

        if(c)
        {
            loadTweens(c, componentXml);

            page->addComponent(c);
        }
    }
}

Font *PageBuilder::addFont(xml_node<> *component, xml_node<> *defaults)
{
    xml_attribute<> *fontXml = component->first_attribute("font");
    xml_attribute<> *fontColorXml = component->first_attribute("fontColor");
    xml_attribute<> *fontSizeXml = component->first_attribute("loadFontSize");
    xml_attribute<> *monitorXml = component->first_attribute("monitor");

    if(defaults)
    {
        if(!fontXml && defaults->first_attribute("font"))
        {
            fontXml = defaults->first_attribute("font");
        }

        if(!fontColorXml && defaults->first_attribute("fontColor"))
        {
            fontColorXml = defaults->first_attribute("fontColor");
        }

        if(!fontSizeXml && defaults->first_attribute("loadFontSize"))
        {
            fontSizeXml = defaults->first_attribute("loadFontSize");
        }
    }


    // use layout defaults unless overridden
    std::string fontName = fontName_;
    SDL_Color fontColor = fontColor_;
    int fontSize = fontSize_;

    if(fontXml)
    {
        fontName = config_.convertToAbsolutePath(
                    Utils::combinePath(config_.absolutePath, "layouts", layoutKey,""),
                    fontXml->value());

        Logger::write(Logger::ZONE_DEBUG, "Layout", "loading font " + fontName );
    }
    if(fontColorXml)
    {
        int intColor = 0;
        std::stringstream ss;
        ss << std::hex << fontColorXml->value();
        ss >> intColor;

        fontColor.b = intColor & 0xFF;
        intColor >>= 8;
        fontColor.g = intColor & 0xFF;
        intColor >>= 8;
        fontColor.r = intColor & 0xFF;
    }

    if(fontSizeXml)
    {
        fontSize = Utils::convertInt(fontSizeXml->value());
    }

    int monitor = monitorXml ? Utils::convertInt(monitorXml->value()) : monitor_;
    fontCache_->loadFont(fontName, fontSize, fontColor, monitor);

    return fontCache_->getFont(fontName, fontSize, fontColor);
}

void PageBuilder::loadTweens(Component *c, xml_node<> *componentXml)
{
    buildViewInfo(componentXml, c->baseViewInfo);

    c->setTweens(createTweenInstance(componentXml));
}

AnimationEvents *PageBuilder::createTweenInstance(xml_node<> *componentXml)
{
    AnimationEvents *tweens = new AnimationEvents();

    buildTweenSet(tweens, componentXml, "onEnter",          "enter");
    buildTweenSet(tweens, componentXml, "onExit",           "exit");
    buildTweenSet(tweens, componentXml, "onIdle",           "idle");
    buildTweenSet(tweens, componentXml, "onMenuIdle",       "menuIdle");
    buildTweenSet(tweens, componentXml, "onMenuScroll",     "menuScroll");
    buildTweenSet(tweens, componentXml, "onHighlightEnter", "highlightEnter");
    buildTweenSet(tweens, componentXml, "onHighlightExit",  "highlightExit");
    buildTweenSet(tweens, componentXml, "onMenuEnter",      "menuEnter");
    buildTweenSet(tweens, componentXml, "onMenuExit",       "menuExit");
    buildTweenSet(tweens, componentXml, "onGameEnter",      "gameEnter");
    buildTweenSet(tweens, componentXml, "onGameExit",       "gameExit");
    buildTweenSet(tweens, componentXml, "onPlaylistEnter",  "playlistEnter");
    buildTweenSet(tweens, componentXml, "onPlaylistExit",   "playlistExit");
    buildTweenSet(tweens, componentXml, "onMenuJumpEnter",  "menuJumpEnter");
    buildTweenSet(tweens, componentXml, "onMenuJumpExit",   "menuJumpExit");
    buildTweenSet(tweens, componentXml, "onAttractEnter",   "attractEnter");
    buildTweenSet(tweens, componentXml, "onAttract",        "attract");
    buildTweenSet(tweens, componentXml, "onAttractExit",    "attractExit");
    buildTweenSet(tweens, componentXml, "onJukeboxJump",    "jukeboxJump");

    buildTweenSet(tweens, componentXml, "onMenuActionInputEnter",  "menuActionInputEnter");
    buildTweenSet(tweens, componentXml, "onMenuActionInputExit",   "menuActionInputExit");
    buildTweenSet(tweens, componentXml, "onMenuActionSelectEnter", "menuActionSelectEnter");
    buildTweenSet(tweens, componentXml, "onMenuActionSelectExit",  "menuActionSelectExit");

    return tweens;
}

void PageBuilder::buildTweenSet(AnimationEvents *tweens, xml_node<> *componentXml, std::string tagName, std::string tweenName)
{
    for(componentXml = componentXml->first_node(tagName.c_str()); componentXml; componentXml = componentXml->next_sibling(tagName.c_str()))
    {
        xml_attribute<> *indexXml = componentXml->first_attribute("menuIndex");

        if(indexXml)
        {
            std::string indexs = indexXml->value();
            if(indexs[0] == '!')
            {
                indexs.erase(0,1);
                int index = Utils::convertInt(indexs);
                for(int i = 0; i < MENU_INDEX_HIGH-1; i++)
                {
                    if(i != index)
                    {
                        Animation *animation = new Animation();
                        getTweenSet(componentXml, animation);
                        tweens->setAnimation(tweenName, i, animation);
                    }
                }
            }
            else if(indexs[0] == '<')
            {
                indexs.erase(0,1);
                int index = Utils::convertInt(indexs);
                for(int i = 0; i < MENU_INDEX_HIGH-1; i++)
                {
                    if(i < index)
                    {
                        Animation *animation = new Animation();
                        getTweenSet(componentXml, animation);
                        tweens->setAnimation(tweenName, i, animation);
                    }
                }
            }
            else if(indexs[0] == '>')
            {
                indexs.erase(0,1);
                int index = Utils::convertInt(indexs);
                for(int i = 0; i < MENU_INDEX_HIGH-1; i++)
                {
                    if(i > index)
                    {
                        Animation *animation = new Animation();
                        getTweenSet(componentXml, animation);
                        tweens->setAnimation(tweenName, i, animation);
                    }
                }
            }
            else if(indexs[0] == 'i')
            {
                Animation *animation = new Animation();
                getTweenSet(componentXml, animation);
                tweens->setAnimation(tweenName, MENU_INDEX_HIGH, animation);
            }
            else
            {
                int index = Utils::convertInt(indexXml->value());
                Animation *animation = new Animation();
                getTweenSet(componentXml, animation);
                tweens->setAnimation(tweenName, index, animation);
            }
        }
        else
        {
            Animation *animation = new Animation();
            getTweenSet(componentXml, animation);
            tweens->setAnimation(tweenName, -1, animation);
        }
    }
}


ScrollingList * PageBuilder::buildMenu(xml_node<> *menuXml, Page &page)
{
    ScrollingList *menu = NULL;
    std::string menuType = "vertical";
    std::string imageType = "null";
    std::string videoType = "null";
    std::map<int, xml_node<> *> overrideItems;
    xml_node<> *itemDefaults               = menuXml->first_node("itemDefaults");
    xml_attribute<> *modeXml               = menuXml->first_attribute("mode");
    xml_attribute<> *imageTypeXml          = menuXml->first_attribute("imageType");
    xml_attribute<> *videoTypeXml          = menuXml->first_attribute("videoType");
    xml_attribute<> *menuTypeXml           = menuXml->first_attribute("type");
    xml_attribute<> *scrollTimeXml         = menuXml->first_attribute("scrollTime");
    xml_attribute<> *scrollAccelerationXml = menuXml->first_attribute("scrollAcceleration");
    xml_attribute<> *minScrollTimeXml      = menuXml->first_attribute("minScrollTime");
    xml_attribute<> *scrollOrientationXml  = menuXml->first_attribute("orientation");

    if(menuTypeXml)
    {
        menuType = menuTypeXml->value();
    }

    // ensure <menu> has an <itemDefaults> tag
    if(!itemDefaults)
    {
        Logger::write(Logger::ZONE_WARNING, "Layout", "Menu tag is missing <itemDefaults> tag.");
    }

    bool playlistType = false;
    if(imageTypeXml)
    {
        imageType = imageTypeXml->value();
        if (imageType.rfind("playlist", 0) == 0) {
            playlistType = true;
        }
    }

    if(videoTypeXml)
    {
        videoType = videoTypeXml->value();
        if (videoType.rfind("playlist", 0) == 0) {
            playlistType = true;
        }
    }

    bool layoutMode = false;
    bool commonMode = false;
    if(modeXml)
    {
        std::string sysMode = modeXml->value();
        if(sysMode == "layout")
        {
            layoutMode = true;
        }
        if(sysMode == "common")
        {
            commonMode = true;
        }
        if(sysMode == "commonlayout")
        {
            layoutMode = true;
            commonMode = true;
        }
    }

    // on default, text will be rendered to the menu. Preload it into cache.
    Font *font = addFont(itemDefaults, NULL);

    menu = new ScrollingList(config_, page, layoutMode, commonMode, playlistType, font, layoutKey, imageType, videoType);
    buildViewInfo(menuXml, menu->baseViewInfo);

    if(scrollTimeXml)
    {
        menu->setStartScrollTime(Utils::convertFloat(scrollTimeXml->value()));
    }

    if(scrollAccelerationXml)
    {
        menu->setScrollAcceleration(Utils::convertFloat(scrollAccelerationXml->value()));
        menu->setMinScrollTime(Utils::convertFloat(scrollAccelerationXml->value()));
    }

    if(minScrollTimeXml)
    {
        menu->setMinScrollTime(Utils::convertFloat(minScrollTimeXml->value()));
    }

    if(scrollOrientationXml)
    {
        std::string scrollOrientation = scrollOrientationXml->value();
        if(scrollOrientation == "horizontal")
        {
            menu->horizontalScroll = true;
        }
    }

    buildViewInfo(menuXml, menu->baseViewInfo);

    if(menuType == "custom")
    {
        buildCustomMenu(menu, menuXml, itemDefaults);
    }
    else
    {
        buildVerticalMenu(menu, menuXml, itemDefaults);
    }

    loadTweens(menu, menuXml);

    return menu;
}


void PageBuilder::buildCustomMenu(ScrollingList *menu, xml_node<> *menuXml, xml_node<> *itemDefaults)
{
    std::vector<ViewInfo *> *points = new std::vector<ViewInfo *>();
    std::vector<AnimationEvents *> *tweenPoints = new std::vector<AnimationEvents *>();

    int i = 0;

    for(xml_node<> *componentXml = menuXml->first_node("item"); componentXml; componentXml = componentXml->next_sibling("item"))
    {
        ViewInfo *viewInfo = new ViewInfo();
        buildViewInfo(componentXml, *viewInfo, itemDefaults);
        viewInfo->Monitor = menu->baseViewInfo.Monitor;

        points->push_back(viewInfo);
        tweenPoints->push_back(createTweenInstance(componentXml));
        xml_attribute<> *selected = componentXml->first_attribute("selected");

        if(selected)
        {
            menu->setSelectedIndex(i);
        }

        i++;
    }

    menu->setPoints(points, tweenPoints);
}

void PageBuilder::buildVerticalMenu(ScrollingList *menu, xml_node<> *menuXml, xml_node<> *itemDefaults)
{
    std::vector<ViewInfo *> *points = new std::vector<ViewInfo *>();
    std::vector<AnimationEvents *> *tweenPoints = new std::vector<AnimationEvents *>();

    int selectedIndex = MENU_FIRST;
    std::map<int, xml_node<> *> overrideItems;

    // By default the menu will automatically determine the offsets for your list items.
    // We can override individual menu points to have unique characteristics (i.e. make the first item opaque or
    // make the selected item a different color).
    for(xml_node<> *componentXml = menuXml->first_node("item"); componentXml; componentXml = componentXml->next_sibling("item"))
    {
        xml_attribute<> *xmlIndex = componentXml->first_attribute("index");

        if(xmlIndex)
        {
            int itemIndex = parseMenuPosition(xmlIndex->value());
            overrideItems[itemIndex] = componentXml;

            // check to see if the item specified is the selected index
            xml_attribute<> *xmlSelectedIndex = componentXml->first_attribute("selected");

            if(xmlSelectedIndex)
            {
                selectedIndex = itemIndex;
            }
        }
    }

    bool end = false;

    //menu start

    float height = 0;
    int index = 0;

    if(overrideItems.find(MENU_START) != overrideItems.end())
    {
        xml_node<> *component = overrideItems[MENU_START];
        ViewInfo *viewInfo = createMenuItemInfo(component, itemDefaults, menu->baseViewInfo.Y + height);
        points->push_back(viewInfo);
        tweenPoints->push_back(createTweenInstance(component));
        height += viewInfo->Height;

        // increment the selected index to account for the new "invisible" menu item
        selectedIndex++;
    }
    while(!end)
    {
        ViewInfo *viewInfo = new ViewInfo();
        xml_node<> *component = itemDefaults;

        // uss overridden item setting if specified by layout for the given index
        if(overrideItems.find(index) != overrideItems.end())
        {
            component = overrideItems[index];
        }

        // calculate the total height of our menu items if we can load any additional items
        buildViewInfo(component, *viewInfo, itemDefaults);
        xml_attribute<> *itemSpacingXml = component->first_attribute("spacing");
        int itemSpacing = itemSpacingXml ? Utils::convertInt(itemSpacingXml->value()) : 0;
        float nextHeight = height + viewInfo->Height + itemSpacing;

        if(nextHeight >= menu->baseViewInfo.Height)
        {
            end = true;
        }

        // we have reached the last menuitem
        if(end && overrideItems.find(MENU_LAST) != overrideItems.end())
        {
            component = overrideItems[MENU_LAST];

            buildViewInfo(component, *viewInfo, itemDefaults);
            xml_attribute<> *itemSpacingXml = component->first_attribute("spacing");
            int itemSpacing = itemSpacingXml ? Utils::convertInt(itemSpacingXml->value()) : 0;
            nextHeight = height + viewInfo->Height + itemSpacing;
        }

        viewInfo->Y = menu->baseViewInfo.Y + (float)height;
        points->push_back(viewInfo);
        tweenPoints->push_back(createTweenInstance(component));
        index++;
        height = nextHeight;
    }

    //menu end
    if(overrideItems.find(MENU_END) != overrideItems.end())
    {
        xml_node<> *component = overrideItems[MENU_END];
        ViewInfo *viewInfo = createMenuItemInfo(component, itemDefaults, menu->baseViewInfo.Y + height);
        points->push_back(viewInfo);
        tweenPoints->push_back(createTweenInstance(component));
    }

    if(selectedIndex >= ((int)points->size()))
    {

        std::stringstream ss;

        ss << "Design error! Selected menu item was set to " << selectedIndex 
           << " although there are only " << points->size()
           << " menu points that can be displayed";

        Logger::write(Logger::ZONE_ERROR, "Layout", "Design error! \"duration\" attribute");

        selectedIndex = 0;
    }


    menu->setSelectedIndex(selectedIndex);
    menu->setPoints(points, tweenPoints);
}

ViewInfo *PageBuilder::createMenuItemInfo(xml_node<> *component, xml_node<> *defaults, float y)
{
    ViewInfo *viewInfo = new ViewInfo();
    buildViewInfo(component, *viewInfo, defaults);
    viewInfo->Y = y;
    return viewInfo;
}

int PageBuilder::parseMenuPosition(std::string strIndex)
{
    int index = MENU_FIRST;

    if(strIndex == "end")
    {
        index = MENU_END;
    }
    else if(strIndex == "last")
    {
        index = MENU_LAST;
    }
    else if(strIndex == "start")
    {
        index = MENU_START;
    }
    else if(strIndex == "first")
    {
        index = MENU_FIRST;
    }
    else
    {
        index = Utils::convertInt(strIndex);
    }
    return index;
}

xml_attribute<> *PageBuilder::findAttribute(xml_node<> *componentXml, std::string attribute, xml_node<> *defaultXml = NULL)
{
    xml_attribute<> *attributeXml = componentXml->first_attribute(attribute.c_str());

    if(!attributeXml && defaultXml)
    {
        attributeXml = defaultXml->first_attribute(attribute.c_str());
    }

    return attributeXml;
}

void PageBuilder::buildViewInfo(xml_node<> *componentXml, ViewInfo &info, xml_node<> *defaultXml)
{
    xml_attribute<> *x                  = findAttribute(componentXml, "x", defaultXml);
    xml_attribute<> *y                  = findAttribute(componentXml, "y", defaultXml);
    xml_attribute<> *xOffset            = findAttribute(componentXml, "xOffset", defaultXml);
    xml_attribute<> *yOffset            = findAttribute(componentXml, "yOffset", defaultXml);
    xml_attribute<> *xOrigin            = findAttribute(componentXml, "xOrigin", defaultXml);
    xml_attribute<> *yOrigin            = findAttribute(componentXml, "yOrigin", defaultXml);
    xml_attribute<> *height             = findAttribute(componentXml, "height", defaultXml);
    xml_attribute<> *width              = findAttribute(componentXml, "width", defaultXml);
    xml_attribute<> *fontSize           = findAttribute(componentXml, "fontSize", defaultXml);
    xml_attribute<> *fontColor          = findAttribute(componentXml, "fontColor", defaultXml);
    xml_attribute<> *minHeight          = findAttribute(componentXml, "minHeight", defaultXml);
    xml_attribute<> *minWidth           = findAttribute(componentXml, "minWidth", defaultXml);
    xml_attribute<> *maxHeight          = findAttribute(componentXml, "maxHeight", defaultXml);
    xml_attribute<> *maxWidth           = findAttribute(componentXml, "maxWidth", defaultXml);
    xml_attribute<> *alpha              = findAttribute(componentXml, "alpha", defaultXml);
    xml_attribute<> *angle              = findAttribute(componentXml, "angle", defaultXml);
    xml_attribute<> *layer              = findAttribute(componentXml, "layer", defaultXml);
    xml_attribute<> *backgroundColor    = findAttribute(componentXml, "backgroundColor", defaultXml);
    xml_attribute<> *backgroundAlpha    = findAttribute(componentXml, "backgroundAlpha", defaultXml);
    xml_attribute<> *reflection         = findAttribute(componentXml, "reflection", defaultXml);
    xml_attribute<> *reflectionDistance = findAttribute(componentXml, "reflectionDistance", defaultXml);
    xml_attribute<> *reflectionScale    = findAttribute(componentXml, "reflectionScale", defaultXml);
    xml_attribute<> *reflectionAlpha    = findAttribute(componentXml, "reflectionAlpha", defaultXml);
    xml_attribute<> *containerX         = findAttribute(componentXml, "containerX", defaultXml);
    xml_attribute<> *containerY         = findAttribute(componentXml, "containerY", defaultXml);
    xml_attribute<> *containerWidth     = findAttribute(componentXml, "containerWidth", defaultXml);
    xml_attribute<> *containerHeight    = findAttribute(componentXml, "containerHeight", defaultXml);
    xml_attribute<> *monitor            = findAttribute(componentXml, "monitor", defaultXml);
    xml_attribute<> *volume             = findAttribute(componentXml, "volume", defaultXml);

    info.X = getHorizontalAlignment(x, 0);
    info.Y = getVerticalAlignment(y, 0);

    info.XOffset =  getHorizontalAlignment(xOffset, 0);
    info.YOffset =  getVerticalAlignment(yOffset, 0);
    float xOriginRelative = getHorizontalAlignment(xOrigin, 0);
    float yOriginRelative = getVerticalAlignment(yOrigin, 0);

    // the origins need to be saved as a percent since the heights and widths can be scaled
    info.XOrigin = xOriginRelative / layoutWidth_;
    info.YOrigin = yOriginRelative / layoutHeight_;


    if(!height && !width)
    {
        info.Height = -1;
        info.Width = -1;
    }
    else
    {
        info.Height = getVerticalAlignment(height, -1);
        info.Width = getHorizontalAlignment(width, -1);
    }
    info.FontSize           = getVerticalAlignment(fontSize, -1);
    info.MinHeight          = getVerticalAlignment(minHeight, 0);
    info.MinWidth           = getHorizontalAlignment(minWidth, 0);
    info.MaxHeight          = getVerticalAlignment(maxHeight, FLT_MAX);
    info.MaxWidth           = getVerticalAlignment(maxWidth, FLT_MAX);
    info.Alpha              = alpha              ? Utils::convertFloat(alpha->value())             : 1.f;
    info.Angle              = angle              ? Utils::convertFloat(angle->value())             : 0.f;
    info.Layer              = layer              ? Utils::convertInt(layer->value())               : 0;
    info.Reflection         = reflection         ? reflection->value()                             : "";
    info.ReflectionDistance = reflectionDistance ? Utils::convertInt(reflectionDistance->value())  : 0;
    info.ReflectionScale    = reflectionScale    ? Utils::convertFloat(reflectionScale->value())   : 0.25f;
    info.ReflectionAlpha    = reflectionAlpha    ? Utils::convertFloat(reflectionAlpha->value())   : 1.f;
    info.ContainerX         = containerX         ? Utils::convertFloat(containerX->value())        : 0.f;
    info.ContainerY         = containerY         ? Utils::convertFloat(containerY->value())        : 0.f;
    info.ContainerWidth     = containerWidth     ? Utils::convertFloat(containerWidth->value())    : -1.f;
    info.ContainerHeight    = containerHeight    ? Utils::convertFloat(containerHeight->value())   : -1.f;
    info.Monitor            = monitor            ? Utils::convertInt(monitor->value())             : 0;
    info.Volume             = volume             ? Utils::convertFloat(volume->value())            : 1.f;

    if(fontColor)
    {
      Font *font = addFont(componentXml, defaultXml);
      info.font = font;
    }

    if(backgroundColor)
    {
        std::stringstream ss(backgroundColor->value());
        int num;
        ss >> std::hex >> num;
        int red = num / 0x10000;
        int green = (num / 0x100) % 0x100;
        int blue = num % 0x100;

        info.BackgroundRed = static_cast<float>(red/255);
        info.BackgroundGreen = static_cast<float>(green/255);
        info.BackgroundBlue = static_cast<float>(blue/255);
    }

    if(backgroundAlpha)
    {
        info.BackgroundAlpha =  backgroundAlpha ? Utils::convertFloat(backgroundAlpha->value()) : 1.f;
    }
}

void PageBuilder::getTweenSet(xml_node<> *node, Animation *animation)
{
    if(node)
    {
        for(xml_node<> *set = node->first_node("set"); set; set = set->next_sibling("set"))
        {
            TweenSet *ts = new TweenSet();
            getAnimationEvents(set, *ts);
            animation->Push(ts);
        }
    }
}

void PageBuilder::getAnimationEvents(xml_node<> *node, TweenSet &tweens)
{
    xml_attribute<> *durationXml = node->first_attribute("duration");

    if(!durationXml)
    {
        Logger::write(Logger::ZONE_ERROR, "Layout", "Animation set tag missing \"duration\" attribute");
    }
    else
    {
        for(xml_node<> *animate = node->first_node("animate"); animate; animate = animate->next_sibling("animate"))
        {
            xml_attribute<> *type = animate->first_attribute("type");
            xml_attribute<> *from = animate->first_attribute("from");
            xml_attribute<> *to = animate->first_attribute("to");
            xml_attribute<> *algorithmXml = animate->first_attribute("algorithm");

            std::string animateType;
            if (type)
            {
                animateType = type->value();
            }


            if(!type)
            {
                Logger::write(Logger::ZONE_ERROR, "Layout", "Animate tag missing \"type\" attribute");
            }
            else if(!to && animateType != "nop")
            {
                Logger::write(Logger::ZONE_ERROR, "Layout", "Animate tag missing \"to\" attribute");
            }
            else
            {
                float fromValue = 0.0f;
                bool  fromDefined = true;
                if (from)
                {
                    fromValue = Utils::convertFloat(from->value());
                }
                else
                {
                   fromDefined = false;
                }
                float toValue = 0.0f;
                if (to)
                {
                    toValue = Utils::convertFloat(to->value());
                }
                float durationValue = Utils::convertFloat(durationXml->value());

                TweenAlgorithm algorithm = LINEAR;
                TweenProperty property;

                if(algorithmXml)
                {
                    algorithm = Tween::getTweenType(algorithmXml->value());

                }

                if(Tween::getTweenProperty(type->value(), property))
                {
                    switch(property)
                    {
                    case TWEEN_PROPERTY_WIDTH:
                    case TWEEN_PROPERTY_X:
                    case TWEEN_PROPERTY_X_OFFSET:
                    case TWEEN_PROPERTY_CONTAINER_X:
                    case TWEEN_PROPERTY_CONTAINER_WIDTH:
                        fromValue = getHorizontalAlignment(from, 0);
                        toValue = getHorizontalAlignment(to, 0);
                        break;

                        // x origin gets translated to a percent
                    case TWEEN_PROPERTY_X_ORIGIN:
                        fromValue = getHorizontalAlignment(from, 0) / layoutWidth_;
                        toValue = getHorizontalAlignment(to, 0) / layoutWidth_;
                        break;

                    case TWEEN_PROPERTY_HEIGHT:
                    case TWEEN_PROPERTY_Y:
                    case TWEEN_PROPERTY_Y_OFFSET:
                    case TWEEN_PROPERTY_FONT_SIZE:
                    case TWEEN_PROPERTY_CONTAINER_Y:
                    case TWEEN_PROPERTY_CONTAINER_HEIGHT:
                        fromValue = getVerticalAlignment(from, 0);
                        toValue = getVerticalAlignment(to, 0);
                        break;

                        // y origin gets translated to a percent
                    case TWEEN_PROPERTY_Y_ORIGIN:
                        fromValue = getVerticalAlignment(from, 0) / layoutHeight_;
                        toValue = getVerticalAlignment(to, 0) / layoutHeight_;
                        break;

                    case TWEEN_PROPERTY_MAX_WIDTH:
                    case TWEEN_PROPERTY_MAX_HEIGHT:
                      fromValue = getVerticalAlignment(from, FLT_MAX);
                      toValue   = getVerticalAlignment(to,   FLT_MAX);

                    default:
                        break;
                    }

                    Tween *t = new Tween(property, algorithm, fromValue, toValue, durationValue);
                    if (!fromDefined)
                      t->startDefined = false;
                    tweens.push(t);
                }
                else
                {
                    std::stringstream ss;
                    ss << "Unsupported tween type attribute \"" << type->value() << "\"";
                    Logger::write(Logger::ZONE_ERROR, "Layout", ss.str());
                }
            }
        }
    }
}
