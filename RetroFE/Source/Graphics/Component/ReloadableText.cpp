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

#include "ReloadableText.h"
#include "../ViewInfo.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <time.h>
#include <ctime>
#include <algorithm>
#include "../../Utility/Utils.h"

ReloadableText::ReloadableText(std::string type, Page &page, Configuration &config, bool systemMode, Font *font, std::string layoutKey, std::string timeFormat, std::string textFormat, std::string singlePrefix, std::string singlePostfix, std::string pluralPrefix, std::string pluralPostfix)
    : Component(page)
    , config_(config)
    , systemMode_(systemMode)
    , imageInst_(NULL)
    , type_(type)
    , layoutKey_(layoutKey)
    , fontInst_(font)
    , timeFormat_(timeFormat)
    , textFormat_(textFormat)
    , singlePrefix_(singlePrefix)
    , singlePostfix_(singlePostfix)
    , pluralPrefix_(pluralPrefix)
    , pluralPostfix_(pluralPostfix)
{
    allocateGraphicsMemory();
}



ReloadableText::~ReloadableText()
{
    if (imageInst_ != NULL)
    {
        delete imageInst_;
    }
}

void ReloadableText::update(float dt)
{
    if (newItemSelected ||
       (newScrollItemSelected && getMenuScrollReload()) ||
        type_ == "time" || type_ == "current" || type_ == "duration" || type_ == "isPaused")
    {
        ReloadTexture();
        newItemSelected = false;
    }

    // needs to be ran at the end to prevent the NewItemSelected flag from being detected
    Component::update(dt);

}

void ReloadableText::allocateGraphicsMemory()
{
    ReloadTexture();

    // NOTICE! needs to be done last to prevent flags from being missed
    Component::allocateGraphicsMemory();
}


void ReloadableText::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();

    if (imageInst_ != NULL)
    {
        delete imageInst_;
        imageInst_ = NULL;
    }
}


void ReloadableText::initializeFonts()
{
    fontInst_->initialize();
}


void ReloadableText::deInitializeFonts()
{
    fontInst_->deInitialize();
}


void ReloadableText::ReloadTexture()
{
    if (imageInst_ != NULL)
    {
        delete imageInst_;
        imageInst_ = NULL;
    }

    Item *selectedItem = page.getSelectedItem();

    if (selectedItem != NULL)
    {
        std::stringstream ss;
        std::string text = "";
        if (type_ == "time")
        {
          time_t    now = time(0);
          struct tm tstruct;
          char      buf[80];
          tstruct = *localtime(&now);
          strftime(buf, sizeof(buf), timeFormat_.c_str(), &tstruct);
          ss << buf;
        }
        if (type_ == "numberButtons")
        {
            text = selectedItem->numberButtons;
        }
        else if (type_ == "numberPlayers")
        {
            text = selectedItem->numberPlayers;
        }
        else if (type_ == "ctrlType")
        {
            text = selectedItem->ctrlType;
        }
        else if (type_ == "numberJoyWays")
        {
            text = selectedItem->joyWays;
        }
        else if (type_ == "rating")
        {
            text = selectedItem->rating;
        }
        else if (type_ == "score")
        {
            text = selectedItem->score;
        }
        else if (type_ == "year")
        {
              text = selectedItem->year;
        }
        else if (type_ == "title")
        {
            text = selectedItem->title;
        }
        else if(type_ == "developer")
        {
            text = selectedItem->developer;
            // Overwrite in case developer has not been specified
            if (text == "")
            {
                text = selectedItem->manufacturer;
            }
        }
        else if (type_ == "manufacturer")
        {
            text = selectedItem->manufacturer;
        }
        else if (type_ == "genre")
        {
            text = selectedItem->genre;
        }
        else if (type_ == "playCount")
        {
            text = std::to_string(selectedItem->playCount);
        }
        else if (type_ == "lastPlayed")
        {
            if (selectedItem->lastPlayed != "0") {
                text = selectedItem->lastPlayed;// getTimeSince(selectedItem->lastPlayed);
            }
        }
        else if (type_.rfind( "playlist", 0 ) == 0)
        {
            text = playlistName;
        }
        else if (type_ == "firstLetter")
        {
          text = selectedItem->fullTitle.at(0);
        }
        else if (type_ == "collectionName")
        {
            text = page.getCollectionName();
        }
        else if (type_ == "collectionSize")
        {
            if (page.getCollectionSize() == 0)
            {
                ss << singlePrefix_ << page.getCollectionSize() << pluralPostfix_;
            }
            else if (page.getCollectionSize() == 1)
            {
                ss << singlePrefix_ << page.getCollectionSize() << singlePostfix_;
            }
            else
            {
                ss << pluralPrefix_ << page.getCollectionSize() << pluralPostfix_;
            }
        }
        else if (type_ == "collectionIndex")
        {
            if (page.getSelectedIndex() == 0)
            {
                ss << singlePrefix_ << (page.getSelectedIndex()+1) << pluralPostfix_;
            }
            else if (page.getSelectedIndex() == 1)
            {
                ss << singlePrefix_ << (page.getSelectedIndex()+1) << singlePostfix_;
            }
            else
            {
                ss << pluralPrefix_ << (page.getSelectedIndex()+1) << pluralPostfix_;
            }
        }
        else if (type_ == "collectionIndexSize")
        {
            if (page.getSelectedIndex() == 0)
            {
                ss << singlePrefix_ << (page.getSelectedIndex()+1) << "/" << page.getCollectionSize() << pluralPostfix_;
            }
            else if (page.getSelectedIndex() == 1)
            {
                ss << singlePrefix_ << (page.getSelectedIndex()+1) << "/" << page.getCollectionSize() << singlePostfix_;
            }
            else
            {
                ss << pluralPrefix_ << (page.getSelectedIndex()+1) << "/" << page.getCollectionSize() << pluralPostfix_;
            }
        }
        else if (type_ == "isFavorite")
        {
            if (selectedItem->isFavorite)
                text = "yes";
            else
                text = "no";
        }
        else if (type_ == "isPaused")
        {
            if (page.isPaused( ))
                text = "Paused";
            else
                text = "";
        }
        else if (type_ == "current")
        {
            unsigned long long current = 0;
            current     = page.getCurrent( );
            current    /= 1000000000;
            int seconds = current%60;
            int minutes = (current/60)%60;
            int hours   = int( current/3600 );
            text        = std::to_string( hours ) + ":";
            if ( minutes < 10 )
                text   += "0" + std::to_string( minutes ) + ":";
            else
                text   += std::to_string( minutes ) + ":";
            if ( seconds < 10 )
                text   +=  "0" + std::to_string( seconds );
            else
                text   += std::to_string( seconds );
			if ( page.getDuration( ) == 0 )
				text    = "--:--:--";
        }
        else if (type_ == "duration")
        {
            unsigned long long duration = 0;
            duration    = page.getDuration( );
            duration   /= 1000000000;
            int seconds = duration%60;
            int minutes = (duration/60)%60;
            int hours   = int( duration/3600 );
            text        = std::to_string( hours ) + ":";
            if ( minutes < 10 )
                text   += "0" + std::to_string( minutes ) + ":";
            else
                text   += std::to_string( minutes ) + ":";
            if ( seconds < 10 )
                text   +=  "0" + std::to_string( seconds );
            else
                text   += std::to_string( seconds );
			if ( page.getDuration( ) == 0 )
				text    = "--:--:--";
        }

        if (!selectedItem->leaf || systemMode_) // item is not a leaf
        {
            (void)config_.getProperty("collections." + selectedItem->name + "." + type_, text );
        }

        if (systemMode_) // Get the system information in stead
        {
            text = "";
            (void)config_.getProperty("collections." + page.getCollectionName() + "." + type_, text );
        }

        bool overwriteXML = false;
        config_.getProperty( "overwriteXML", overwriteXML );
        if ( text == "" || overwriteXML ) // No text was found yet; check the info in stead
        {
            std::string text_tmp;
            selectedItem->getInfo( type_, text_tmp );
            if ( text_tmp != "" )
            {
                text = text_tmp;
            }
        }

        if (text == "0")
        {
            text = singlePrefix_ + text + pluralPostfix_;
        }
        else if (text == "1")
        {
            text = singlePrefix_ + text + singlePostfix_;
        }
        else if (text != "")
        {
            text = pluralPrefix_ + text + pluralPostfix_;
        }

        if (text != "")
        {
            if (textFormat_ == "uppercase")
            {
                std::transform(text.begin(), text.end(), text.begin(), ::toupper);
            }
            if (textFormat_ == "lowercase")
            {
                std::transform(text.begin(), text.end(), text.begin(), ::tolower);
            }
            ss << text;
        }

        imageInst_ = new Text(ss.str(), page, fontInst_, baseViewInfo.Monitor);
    }
}


void ReloadableText::draw()
{
    if(imageInst_)
    {
        imageInst_->baseViewInfo = baseViewInfo;
        imageInst_->draw();
    }
}

// thank you chatgpt
std::string ReloadableText::getTimeSince(std::string sinceTimestamp)
{
    const char* timestamp = sinceTimestamp.c_str();
    std::time_t t2 = (time_t)strtol(timestamp, NULL, 10);

    // error checking, make sure timestamp is valid
    if (t2 == 0 && errno == EINVAL) {
        return "";
    }

    // error checking, make sure the timestamp is not in the future
    std::time_t t1 = std::time(nullptr);
    if (t2 > t1) {
        return "";
    }

    // Convert time_t to struct tm
    std::tm tm1 = *std::localtime(&t1);
    std::tm tm2 = *std::localtime(&t2);

    // Calculate the difference in years, months, and days
    int yearsDiff = tm1.tm_year - tm2.tm_year;
    int monthsDiff = tm1.tm_mon - tm2.tm_mon;
    int daysDiff = tm1.tm_mday - tm2.tm_mday;

    // Adjust the difference in case of negative values
    if (daysDiff < 0) {
        monthsDiff--;
        std::time_t tempT2 = t2;
        std::tm tempTm2 = tm2;

        while (tempTm2.tm_mon != tm1.tm_mon) {
            tempT2 += 24 * 60 * 60;  // Add 1 day
            tempTm2 = *std::localtime(&tempT2);
            daysDiff += tempTm2.tm_mday;  // Add the number of days in the current month
        }

        if (daysDiff < 0) {
            std::time_t tempT2 = t2;
            std::tm tempTm2 = tm2;
            int totalDaysDiff = 0;

            while (tempTm2.tm_year != tm1.tm_year || tempTm2.tm_mon != tm1.tm_mon) {
                tempT2 += 24 * 60 * 60;  // Add 1 day
                tempTm2 = *std::localtime(&tempT2);
                totalDaysDiff++;
            }

            monthsDiff--;
            daysDiff = totalDaysDiff - daysDiff;
        }
    }

    // Adjust the difference in case of negative months
    if (monthsDiff < 0) {
        yearsDiff--;
        monthsDiff += 12;
    }

    // Calculate the number of days in each month
    const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    int totalDays = 0;
    for (int i = tm2.tm_mon; i < 12; ++i) {
        totalDays += daysInMonth[i];
    }
    totalDays -= tm2.tm_mday - 1;

    for (int i = 0; i < tm1.tm_mon; ++i) {
        totalDays += daysInMonth[i];
    }
    totalDays += tm1.tm_mday;

    // Adjust the difference by the total number of days
    if (totalDays >= daysDiff) {
        totalDays -= daysDiff;
    }
    else {
        monthsDiff--;
        totalDays = daysInMonth[(tm1.tm_mon + 11) % 12] - (daysDiff - totalDays - 1);
    }

    // construct the result string
    std::string result = "";
    if (yearsDiff > 0) {
        result += std::to_string(yearsDiff) + (yearsDiff == 1 ? " year " : " years ");
    }
    if (monthsDiff > 0) {
        result += std::to_string(monthsDiff) + (monthsDiff == 1 ? " month " : " months ");
    }
    if (daysDiff > 0) {
        result += std::to_string(daysDiff) + (daysDiff == 1 ? " day " : " days ");
    }

    if (result == "") {
        result = "today";
    }
    else {
        result += "ago";
    }

    return result;
}
