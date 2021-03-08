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

#include "ReloadableMedia.h"
#include "ImageBuilder.h"
#include "VideoBuilder.h"
#include "ReloadableText.h"
#include "../ViewInfo.h"
#include "../../Video/VideoFactory.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../Utility/Utils.h"
#include "../../SDL.h"
#include <fstream>
#include <vector>
#include <iostream>

ReloadableMedia::ReloadableMedia(Configuration &config, bool systemMode, bool layoutMode, bool commonMode, bool menuMode, std::string type, std::string imageType, Page &p, int displayOffset, bool isVideo, Font *font, bool jukebox, int jukeboxNumLoops)
    : Component(p)
    , config_(config)
    , systemMode_(systemMode)
    , layoutMode_(layoutMode)
    , commonMode_(commonMode)
    , menuMode_(menuMode)
    , loadedComponent_(NULL)
    , videoInst_(NULL)
    , isVideo_(isVideo)
    , FfntInst_(font)
    , textFallback_(false)
    , type_(type)
    , displayOffset_(displayOffset)
    , imageType_(imageType)
    , jukebox_(jukebox)
    , jukeboxNumLoops_(jukeboxNumLoops)
{
    allocateGraphicsMemory();
}

ReloadableMedia::~ReloadableMedia()
{
    if (loadedComponent_ != NULL)
    {
        delete loadedComponent_;
    }
}

void ReloadableMedia::enableTextFallback_(bool value)
{
    textFallback_ = value;
}

void ReloadableMedia::update(float dt)
{
    if (newItemSelected ||
       (newScrollItemSelected && getMenuScrollReload()))
    {

        reloadTexture();
        newItemSelected       = false;
        newScrollItemSelected = false;
    }

    if(loadedComponent_)
    {

        // video needs to run a frame to start getting size info
        if(baseViewInfo.ImageHeight == 0 && baseViewInfo.ImageWidth == 0)
        {
            baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
            baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
        }

        loadedComponent_->update(dt);
    }

    // needs to be ran at the end to prevent the NewItemSelected flag from being detected
    Component::update(dt);

}

void ReloadableMedia::allocateGraphicsMemory()
{
    if(loadedComponent_)
    {
        loadedComponent_->allocateGraphicsMemory();
    }

    // NOTICE! needs to be done last to prevent flags from being missed
    Component::allocateGraphicsMemory();
}


void ReloadableMedia::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();

    if(loadedComponent_)
    {
        loadedComponent_->freeGraphicsMemory();
    }
}


void ReloadableMedia::reloadTexture()
{
    if(loadedComponent_)
    {
        delete loadedComponent_;
        loadedComponent_ = NULL;
    }

    Item *selectedItem = page.getSelectedItem(displayOffset_);
    if(!selectedItem) return;

    config_.getProperty("currentCollection", currentCollection_);

    // build clone list
    std::vector<std::string> names;

    names.push_back(selectedItem->name);
    names.push_back(selectedItem->fullTitle);
    if(selectedItem->cloneof.length() > 0)
    {
        names.push_back(selectedItem->cloneof);
    }

    std::string typeLC   = Utils::toLower(type_);
    if (typeLC == "isfavorite")
    {
        if (selectedItem->isFavorite)
        {
            names.push_back("yes");
        }
        else
        {
            names.push_back("no");
        }
    }

    names.push_back("default");

    if(isVideo_)
    {
        for(unsigned int n = 0; n < names.size() && !loadedComponent_; ++n)
        {
            std::string basename = names[n];
            if(systemMode_)
            {

                // check the master collection for the system artifact 
                loadedComponent_ = findComponent(collectionName, type_, type_, "", true, true);

                // check the collection for the system artifact
                if(!loadedComponent_)
                {
                  loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, type_, "", true, true);
                }

            }
            else
            {

                // are we looking at a leaf or a submenu
                if (selectedItem->leaf) // item is a leaf
                {

                  // check the master collection for the artifact 
                  loadedComponent_ = findComponent(collectionName, type_, basename, "", false, true);

                  // check the collection for the artifact
                  if(!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, "", false, true);
                  }

                  // check the rom directory for the artifact
                  if(!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, type_, selectedItem->filepath, false, true);
                  }

                }
                else // item is a submenu
                {

                  // check the master collection for the artifact 
                  loadedComponent_ = findComponent(collectionName, type_, basename, "", false, true);

                  // check the collection for the artifact
                  if(!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, "", false, true);
                  }

                  // check the submenu collection for the system artifact
                  if (!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->name, type_, type_, "", true, true);
                  } 

                }

            }

            if(loadedComponent_)
            {
                loadedComponent_->allocateGraphicsMemory();
                baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
                baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
            }
        }
    }

    // check for images, also if video could not be found (and was specified)
    for(unsigned int n = 0; n < names.size() && !loadedComponent_; ++n)
    {
        std::string basename = names[n];
        bool        defined  = false;
        if ( isVideo_ )
            typeLC = Utils::toLower(imageType_);

        if(basename == "default")
        {
            basename = "default";
            defined  = true;
        }
        else if(typeLC == "numberbuttons")
        {
            basename = selectedItem->numberButtons;
            defined  = true;
        }
        else if(typeLC == "numberplayers")
        {
            basename = selectedItem->numberPlayers;
            defined  = true;
        }
        else if(typeLC == "year")
        {
            basename = selectedItem->year;
            defined  = true;
        }
        else if(typeLC == "title")
        {
            basename = selectedItem->title;
            defined  = true;
        }
        else if(typeLC == "developer")
        {
            basename = selectedItem->developer;
            defined  = true;
            // Overwrite in case developer has not been specified
            if (basename == "")
            {
                basename = selectedItem->manufacturer;
            }
        }
        else if(typeLC == "manufacturer")
        {
            basename = selectedItem->manufacturer;
            defined  = true;
        }
        else if(typeLC == "genre")
        {
            basename = selectedItem->genre;
            defined  = true;
        }
        else if(typeLC == "ctrltype")
        {
            basename = selectedItem->ctrlType;
            defined  = true;
        }
        else if(typeLC == "joyways")
        {
            basename = selectedItem->joyWays;
            defined  = true;
        }
        else if(typeLC == "rating")
        {
            basename = selectedItem->rating;
            defined  = true;
        }
        else if(typeLC == "score")
        {
            basename = selectedItem->score;
            defined  = true;
        }
        else if(typeLC.rfind( "playlist", 0 ) == 0)
        {
            basename = page.getPlaylistName();
            defined  = true;
        }
        else if (typeLC == "firstletter")
        {
            basename = selectedItem->fullTitle.at(0);
            defined  = true;
        }

        if (!selectedItem->leaf) // item is not a leaf
        {
            (void)config_.getProperty("collections." + selectedItem->name + "." + type_, basename );
        }

        bool overwriteXML = false;
        config_.getProperty( "overwriteXML", overwriteXML );
        if ( !defined || overwriteXML ) // No basename was found yet; check the info in stead
        {
            std::string basename_tmp;
            selectedItem->getInfo( type_, basename_tmp );
            if ( basename_tmp != "" )
            {
                basename = basename_tmp;
            }
        }

        Utils::replaceSlashesWithUnderscores(basename);

        if(systemMode_)
        {

            // check the master collection for the system artifact 
            loadedComponent_ = findComponent(collectionName, type_, type_, "", true, false);

            // check collection for the system artifact
            if(!loadedComponent_)
            {
              loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, type_, "", true, false);
            }

        }
        else
        {

            // are we looking at a leaf or a submenu
            if (selectedItem->leaf) // item is a leaf
            {

              // check the master collection for the artifact 
              loadedComponent_ = findComponent(collectionName, type_, basename, "", false, false);

              // check the collection for the artifact
              if(!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, "", false, false);
              }

              // check the rom directory for the artifact
              if(!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, type_, selectedItem->filepath, false, false);
              }

            }
            else // item is a submenu
            {

              // check the master collection for the artifact 
              loadedComponent_ = findComponent(collectionName, type_, basename, "", false, false);

              // check the collection for the artifact
              if(!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, "", false, false);
              }

              // check the submenu collection for the system artifact
              if (!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->name, type_, type_, "", true, false);
              } 

            }

        }

        if (loadedComponent_ != NULL)
        {
             loadedComponent_->allocateGraphicsMemory();
             baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
             baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
        }
    }

    // if image and artwork was not specified, fall back to displaying text
    if(!loadedComponent_ && textFallback_)
    {
        loadedComponent_ = new Text(selectedItem->fullTitle, page, FfntInst_, baseViewInfo.Monitor);
        baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
        baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
    }
}


Component *ReloadableMedia::findComponent(std::string collection, std::string type, std::string basename, std::string filepath, bool systemMode, bool isVideo)
{
    std::string imagePath;
    Component *component = NULL;
    VideoBuilder videoBuild;
    ImageBuilder imageBuild;

    // check the system folder
    if (layoutMode_)
    {
        std::string layoutName;
        config_.getProperty("layout", layoutName);
        if (commonMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", "_common");
        }
        else
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", collection);
        }
        if (systemMode)
            imagePath = Utils::combinePath(imagePath, "system_artwork");
        else
            imagePath = Utils::combinePath(imagePath, "medium_artwork", type);
    }
    else
    {
        if (commonMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "collections", "_common" );
            if (systemMode)
                imagePath = Utils::combinePath(imagePath, "system_artwork");
            else
                imagePath = Utils::combinePath(imagePath, "medium_artwork", type);
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(collection, type, systemMode, imagePath);
        }
    }
    if ( filepath != "" )
        imagePath = filepath;

    if(isVideo)
    {
        if ( jukebox_ )
            component = videoBuild.createVideo(imagePath, page, basename, baseViewInfo.Monitor, type=="video", jukeboxNumLoops_);
        else
            component = videoBuild.createVideo(imagePath, page, basename, baseViewInfo.Monitor, type=="video");
    }
    else
    {
        component = imageBuild.CreateImage(imagePath, page, basename, baseViewInfo.Monitor);
    }

    return component;

}

void ReloadableMedia::draw()
{
    Component::draw();

    if(loadedComponent_)
    {
    	baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
    	baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
        loadedComponent_->baseViewInfo = baseViewInfo;
        loadedComponent_->draw();
    }
}


bool ReloadableMedia::isJukeboxPlaying()
{
    if ( jukebox_ && loadedComponent_ )
        return loadedComponent_->isPlaying();
    else
        return false;
}
