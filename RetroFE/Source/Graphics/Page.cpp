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

#include "Page.h"
#include "ComponentItemBinding.h"
#include "Component/Component.h"
#include "../Collection/CollectionInfo.h"
#include "Component/Text.h"
#include "../Utility/Log.h"
#include "Component/ScrollingList.h"
#include "../Sound/Sound.h"
#include "ComponentItemBindingBuilder.h"
#include "PageBuilder.h"
#include <algorithm>
#include <sstream>


Page::Page(Configuration &config, int layoutWidth, int layoutHeight)
    : config_(config)
    , menuDepth_(0)
    , scrollActive_(false)
    , selectedItem_(NULL)
    , textStatusComponent_(NULL)
    , loadSoundChunk_(NULL)
    , unloadSoundChunk_(NULL)
    , highlightSoundChunk_(NULL)
    , selectSoundChunk_(NULL)
    , minShowTime_(0)
    , jukebox_(false)
    , playlistMenu_(NULL)
    , elapsedTime_(0)
{
    for (int i = 0; i < SDL::getNumScreens(); i++)
    {
        layoutWidth_.push_back(layoutWidth);
        layoutHeight_.push_back(layoutHeight);
    }
}


Page::~Page()
{
}


void Page::deInitialize()
{

    cleanup( );

    MenuVector_T::iterator it = menus_.begin();
    while(it != menus_.end())
    {
        std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin();
        while(it2 != menus_[std::distance(menus_.begin(), it)].end())
        {
            ScrollingList *menu = *it2;
            menus_[std::distance(menus_.begin(), it)].erase(it2);
            delete menu;
            it2 = menus_[std::distance(menus_.begin(), it)].begin();
        }
        menus_.erase(it);
        it = menus_.begin();
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        delete *it;
    }
    LayerComponents.clear();


    if(loadSoundChunk_)
    {
        delete loadSoundChunk_;
        loadSoundChunk_ = NULL;
    }

    if(unloadSoundChunk_)
    {
        delete unloadSoundChunk_;
        unloadSoundChunk_ = NULL;
    }


    if(highlightSoundChunk_)
    {
        delete highlightSoundChunk_;
        highlightSoundChunk_ = NULL;
    }

    if(selectSoundChunk_)
    {
        delete selectSoundChunk_;
        selectSoundChunk_ = NULL;
    }
    CollectionVector_T::iterator itc = collections_.begin();

    while(itc != collections_.end())
    {
        itc->collection->Save();

        if(itc->collection)
        {
            delete itc->collection;
        }
        collections_.erase(itc);
        itc = collections_.begin();
    }
}


bool Page::isMenusFull()
{
  return (menuDepth_ > menus_.size());
}


void Page::setLoadSound(Sound *chunk)
{
  loadSoundChunk_ = chunk;
}


void Page::setUnloadSound(Sound *chunk)
{
  unloadSoundChunk_ = chunk;
}


void Page::setHighlightSound(Sound *chunk)
{
  highlightSoundChunk_ = chunk;
}


void Page::setSelectSound(Sound *chunk)
{
  selectSoundChunk_ = chunk;
}

void Page::setSelectedItem()
{
    ScrollingList* amenu = getAnActiveMenu();
    if (!amenu) return;

    selectedItem_ = amenu->getSelectedItem();
}

ScrollingList* Page::getAnActiveMenu() {
    if (activeMenu_.size()) {
        for (unsigned int i = 0; i < activeMenu_.size(); i++)
        {
            if (!activeMenu_[i]->playlistType_) {
                return activeMenu_[i];
            }
        }
    }
    
    return NULL;
}

void Page::setActiveMenuItemsFromPlaylist(MenuInfo_S info, ScrollingList* menu)
{
    // keep playlist menu
    if (menu->playlistType_ && info.collection->playlistItems.size()) {
        menu->setItems(&info.collection->playlistItems);
    }
    else {
        menu->setItems(playlist_->second);
    }
}

void Page::onNewItemSelected()
{
    if(!getAnActiveMenu()) return;

    setSelectedItem();

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menu) menu->setNewItemSelected();
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->setNewItemSelected();
    }

}


void Page::onNewScrollItemSelected()
{
    if(!getAnActiveMenu()) return;

    setSelectedItem();

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->setNewScrollItemSelected();
    }

}


void Page::highlightLoadArt()
{
    if(!getAnActiveMenu()) return;

    setSelectedItem();

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->setNewItemSelected();
    }

}


void Page::pushMenu(ScrollingList *s, int index)
{
    // If index < 0 then append to the menus_ vector
    if(index < 0)
    {
        index = menus_.size();
    }

    // Increase menus_ as needed
    while(index >= static_cast<int>(menus_.size()))
    {
        std::vector<ScrollingList *> menus;
        menus_.push_back(menus);
    }

    menus_[index].push_back(s);
}


unsigned int Page::getMenuDepth()
{
    return menuDepth_;
}


void Page::setStatusTextComponent(Text *t)
{
    textStatusComponent_ = t;
}


bool Page::addComponent(Component *c)
{
    bool retVal = false;

    if(c->baseViewInfo.Layer < NUM_LAYERS)
    {
        LayerComponents.push_back(c);
        retVal = true;
    }
    else
    {
        std::stringstream ss;
        ss << "Component layer too large Layer: " << c->baseViewInfo.Layer;
        Logger::write(Logger::ZONE_ERROR, "Page", ss.str());
    }

    return retVal;
}


bool Page::isMenuIdle()
{
    bool idle = true;

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;

            if(!menu->isIdle())
            {
                idle = false;
                break;
            }
        }
    }
    return idle;
}


bool Page::isIdle()
{
    bool idle = isMenuIdle();

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end() && idle; ++it)
    {
        idle = (*it)->isIdle();
    }

    return idle;
}


bool Page::isAttractIdle()
{
    bool idle = true;

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;

            if(!menu->isAttractIdle())
            {
                idle = false;
                break;
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end() && idle; ++it)
    {
        idle = (*it)->isAttractIdle();
    }

    return idle;
}


bool Page::isGraphicsIdle()
{
    bool idle = true;

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end() && idle; ++it)
    {
        idle = (*it)->isIdle();
    }

    return idle;
}


void Page::start()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            menu->triggerEvent( "enter" );
            menu->triggerEnterEvent();
        }
    }

    if(loadSoundChunk_)
    {
        loadSoundChunk_->play();
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "enter" );
    }
}


void Page::stop()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            menu->triggerEvent( "exit" );
            menu->triggerExitEvent();
        }
    }

    if(unloadSoundChunk_)
    {
        unloadSoundChunk_->play();
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "exit" );
    }
}


Item *Page::getSelectedItem()
{
    return selectedItem_;
}


Item *Page::getSelectedItem(int offset)
{
    ScrollingList* amenu = getAnActiveMenu();
    if (!amenu) return NULL;

    return amenu->getItemByOffset(offset);
}


void Page::removeSelectedItem()
{
    /*
    //todo: change method to RemoveItem() and pass in SelectedItem
    if(Menu)
    {
        Menu->RemoveSelectedItem();
    }
    */
    selectedItem_ = NULL;

}


void Page::setScrollOffsetIndex(unsigned int i)
{
    if (!getAnActiveMenu()) return;

    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->setScrollOffsetIndex(i);
    }
}


unsigned int Page::getScrollOffsetIndex()
{
    ScrollingList* amenu = getAnActiveMenu();
    if (!amenu) return -1;

    return amenu->getScrollOffsetIndex();
}


void Page::setMinShowTime(float value)
{
    minShowTime_ = value;
}


float Page::getMinShowTime()
{
    return minShowTime_;
}


void Page::playlistChange()
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu) menu->setPlaylist(playlist_->first);
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->setPlaylist(playlist_->first);
    }

    updatePlaylistMenuPosition();
}


void Page::menuScroll()
{
    Item *item = selectedItem_;

    if(!item) return;

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "menuScroll", menuDepth_ - 1 );
    }
}


void Page::highlightEnter()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "highlightEnter", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerHighlightEnterEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "highlightEnter", menuDepth_ - 1 );
                menu->triggerHighlightEnterEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "highlightEnter", menuDepth_ - 1 );
    }
}


void Page::highlightExit()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "highlightExit", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerHighlightExitEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "highlightExit", menuDepth_ - 1 );
                menu->triggerHighlightExitEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "highlightExit", menuDepth_ - 1 );
    }
}


void Page::playlistEnter()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "playlistEnter", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerPlaylistEnterEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "playlistEnter", menuDepth_ - 1 );
                menu->triggerPlaylistEnterEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "playlistEnter", menuDepth_ - 1 );
    }
}


void Page::playlistExit()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "playlistExit", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerPlaylistExitEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "playlistExit", menuDepth_ - 1 );
                menu->triggerPlaylistExitEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "playlistExit", menuDepth_ - 1 );
    }
}


void Page::menuJumpEnter()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "menuJumpEnter", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerMenuJumpEnterEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "menuJumpEnter", menuDepth_ - 1 );
                menu->triggerMenuJumpEnterEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "menuJumpEnter", menuDepth_ - 1 );
    }
}


void Page::menuJumpExit()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "menuJumpExit", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerMenuJumpExitEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "menuJumpExit", menuDepth_ - 1 );
                menu->triggerMenuJumpExitEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "menuJumpExit", menuDepth_ - 1 );
    }
}


void Page::attractEnter()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "attractEnter", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerAttractEnterEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "attractEnter", menuDepth_ - 1 );
                menu->triggerAttractEnterEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "attractEnter", menuDepth_ - 1 );
    }
}


void Page::attract()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "attract", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerAttractEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "attract", menuDepth_ - 1 );
                menu->triggerAttractEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "attract", menuDepth_ - 1 );
    }
}


void Page::attractExit()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "attractExit", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerAttractExitEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "attractExit", menuDepth_ - 1 );
                menu->triggerAttractExitEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "attractExit", menuDepth_ - 1 );
    }
}


void Page::jukeboxJump()
{
    Item *item = selectedItem_;

    if(!item) return;
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "jukeboxJump", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerJukeboxJumpEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "jukeboxJump", menuDepth_ - 1 );
                menu->triggerJukeboxJumpEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "jukeboxJump", menuDepth_ - 1 );
    }
}


void Page::triggerEvent( std::string action )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( action );
    }
}


void Page::setText( std::string text, int id )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->setText( text, id );
    }
}


void Page::setScrolling(ScrollDirection direction)
{
    switch(direction)
    {
    case ScrollDirectionForward:
        if(!scrollActive_)
        {
            menuScroll();
        }
        scrollActive_ = true;
        break;
    case ScrollDirectionBack:
        if(!scrollActive_)
        {
            menuScroll();
        }
        scrollActive_ = true;
        break;
    case ScrollDirectionIdle:
    default:
        scrollActive_ = false;
        break;
    }

}


bool Page::isHorizontalScroll()
{
    ScrollingList* amenu = getAnActiveMenu();
    if(!amenu) return false;

    return amenu->horizontalScroll;
}


void Page::pageScroll(ScrollDirection direction)
{
    ScrollingList* amenu = getAnActiveMenu();
    if (!amenu) return;

    if(direction == ScrollDirectionForward)
    {
        amenu->pageDown();
    } else if(direction == ScrollDirectionBack)
    {
        amenu->pageUp();
    }

    unsigned int index = amenu->getScrollOffsetIndex();
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if (menu)
            menu->setScrollOffsetIndex(index);
    }
}

void Page::selectRandom()
{
    ScrollingList* amenu = getAnActiveMenu();
    if (!amenu) return;

    amenu->random();
    unsigned int index = amenu->getScrollOffsetIndex();
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        menu->setScrollOffsetIndex(index);
    }
}


void Page::letterScroll(ScrollDirection direction)
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu)
        {
            if(direction == ScrollDirectionForward)
            {
                menu->letterDown();
            }
            if(direction == ScrollDirectionBack)
            {
                menu->letterUp();
            }
        }
    }
}


void Page::subScroll(ScrollDirection direction)
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu)
        {
            if(direction == ScrollDirectionForward)
            {
                menu->subDown();
            }
            if(direction == ScrollDirectionBack)
            {
                menu->subUp();
            }
        }
    }
}


void Page::cfwLetterSubScroll(ScrollDirection direction)
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu)
        {
            if(direction == ScrollDirectionForward)
            {
                menu->cfwLetterSubDown();
            }
            if(direction == ScrollDirectionBack)
            {
                menu->cfwLetterSubUp();
            }
        }
    }
}


unsigned int Page::getCollectionSize()
{
    ScrollingList* amenu = getAnActiveMenu();
    if (!amenu) return 0;

    return amenu->getSize();
}


unsigned int Page::getSelectedIndex()
{
    ScrollingList* amenu = getAnActiveMenu();
    if (!amenu) return 0;

    return amenu->getSelectedIndex();
}


bool Page::pushCollection(CollectionInfo *collection)
{

    // grow the menu as needed
    if(menus_.size() <= menuDepth_ && getAnActiveMenu())
    {
        for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
        {
            ScrollingList *menu    = *it;
            ScrollingList *newMenu = new ScrollingList(*menu);
            if (&newMenu->playlistType_) {
                playlistMenu_ = newMenu;
            }
            pushMenu(newMenu, menuDepth_);
        }
    }
    if (menus_.size()) {
        activeMenu_ = menus_[menuDepth_];
        for (std::vector<ScrollingList*>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
        {
            ScrollingList* menu = *it;
            menu->collectionName = collection->name;
            // add playlist menu items
            if (menu->playlistType_ && collection->playlistItems.size()) {
                menu->setItems(&collection->playlistItems);
            }
            else {
                // add item collection menu
                menu->setItems(&collection->items);
            }
        }
    }
    else {
        Logger::write(Logger::ZONE_WARNING, "RetroFE", "layout.xml doesn't have any menus");
    }

    // build the collection info instance
    MenuInfo_S info;
    info.collection = collection;
    info.playlist = collection->playlists.begin();
    info.queueDelete = false;
    collections_.push_back(info);

    playlist_ = info.playlist;
    playlistChange();

    if(menuDepth_ < menus_.size())
    {
        menuDepth_++;
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->collectionName = collection->name;
    }

    return true;
}


bool Page::popCollection()
{
    if (!getAnActiveMenu()) return false;
    if(menuDepth_ <= 1) return false;
    if(collections_.size() <= 1) return false;

    // queue the collection for deletion
    MenuInfo_S *info = &collections_.back();
    info->queueDelete = true;
    deleteCollections_.push_back(*info);

    // get the next collection off of the stack
    collections_.pop_back();
    info = &collections_.back();

    // build playlist menu
    if (playlistMenu_ && info->collection->playlistItems.size()) {
        playlistMenu_->setItems(&info->collection->playlistItems);
    }

    playlist_ = info->playlist;
    playlistChange();

    menuDepth_--;
    activeMenu_ = menus_[menuDepth_ - 1];

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->collectionName = info->collection->name;
    }

    return true;
}


void Page::enterMenu()
{

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "menuEnter", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerMenuEnterEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "menuEnter", menuDepth_ - 1 );
                menu->triggerMenuEnterEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "menuEnter", menuDepth_ - 1 );
    }

    return;
}


void Page::exitMenu()
{

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "menuExit", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerMenuExitEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "menuExit", menuDepth_ - 1 );
                menu->triggerMenuExitEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "menuExit", menuDepth_ - 1 );
    }

    return;
}


void Page::enterGame()
{

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "gameEnter", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerGameEnterEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "gameEnter", menuDepth_ - 1 );
                menu->triggerGameEnterEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "gameEnter", menuDepth_ - 1 );
    }

    return;
}


void Page::exitGame()
{

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            if(menuDepth_-1 == static_cast<unsigned int>(distance(menus_.begin(), it)))
            {
                // Also trigger animations for index i for active menu
                menu->triggerEvent( "gameExit", MENU_INDEX_HIGH + menuDepth_ - 1 );
                menu->triggerGameExitEvent( MENU_INDEX_HIGH + menuDepth_ - 1 );
            }
            else
            {
                menu->triggerEvent( "gameExit", menuDepth_ - 1 );
                menu->triggerGameExitEvent( menuDepth_ - 1 );
            }
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->triggerEvent( "gameExit", menuDepth_ - 1 );
    }

    return;
}


std::string Page::getPlaylistName()
{
   return playlist_->first;
}


void Page::favPlaylist()
{
    if(playlist_->first == "favorites")
    {
        selectPlaylist("all");
    }
    else
    {
        selectPlaylist("favorites");
    }
    return;
}


void Page::nextPlaylist()
{
    MenuInfo_S &info = collections_.back();
    unsigned int numlists = info.collection->playlists.size();

    for(unsigned int i = 0; i <= numlists; ++i)
    {
        playlist_++;
        // wrap
        if(playlist_ == info.collection->playlists.end()) 
            playlist_ = info.collection->playlists.begin();

        // find the first playlist
        if(playlist_->second->size() != 0) 
            break;
    }

    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        setActiveMenuItemsFromPlaylist(info, *it);
    }
    playlistChange();
}


void Page::prevPlaylist()
{
    MenuInfo_S &info = collections_.back();
    unsigned int numlists = info.collection->playlists.size();

    for(unsigned int i = 0; i <= numlists; ++i)
    {
        // wrap
        if(playlist_ == info.collection->playlists.begin())
        {
            playlist_ = info.collection->playlists.end();
        }
        playlist_--;

        // find the first playlist
        if(playlist_->second->size() != 0) break;
    }

    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        setActiveMenuItemsFromPlaylist(info, *it);
    }
    playlistChange();
}


void Page::selectPlaylist(std::string playlist)
{
    MenuInfo_S &info = collections_.back();
    info.collection->Save();
    unsigned int numlists = info.collection->playlists.size();

    // Store current playlist
    CollectionInfo::Playlists_T::iterator playlist_store = playlist_;

    for(unsigned int i = 0; i <= numlists; ++i)
    {
        playlist_++;
        // wrap
        if(playlist_ == info.collection->playlists.end()) playlist_ = info.collection->playlists.begin();

        // find the first playlist
        if(playlist_->second->size() != 0 && playlist_->first == playlist) break;
    }

    // Do not change playlist if it does not exist or if it's empty
    if ( playlist_->second->size() == 0 || playlist_->first != playlist)
      playlist_ = playlist_store;

    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        setActiveMenuItemsFromPlaylist(info, *it);
    }
    playlistChange();
}

void Page::updatePlaylistMenuPosition()
{
    if (playlistMenu_) {
        unsigned int i = 0;
        std::string name = getPlaylistName();
        std::vector<Item*> items = playlistMenu_->getItems();
        for (std::vector<Item *>::iterator it = items.begin(); it != items.end(); ++it) {
            Item* item = *it;
            if (item->name == name) {
                playlistMenu_->setScrollOffsetIndex(i);

                return;
            }
            i++;
        }
    }
}

void Page::nextCyclePlaylist(std::vector<std::string> list)
{

    // Empty list
    if (list.size() == 0)
        return;

    // Find the current playlist in the list
    std::vector<std::string>::iterator it = list.begin();
    while (*it != getPlaylistName() && it != list.end())
        ++it;

    // If current playlist not found, switch to the first found cycle playlist in the playlist list
    if (it == list.end())
    {
        for (std::vector<std::string>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
        {
            selectPlaylist( *it2 );
            if (*it2 == getPlaylistName())
                break;
        }
    }
    // Current playlist found; switch to the next found playlist in the list
    else
    {
        for(;;)
        {
            ++it;
            if (it == list.end()) it = list.begin(); // wrap
            selectPlaylist( *it );
            if (*it == getPlaylistName())
                break;
        }
    }
    
}


void Page::prevCyclePlaylist(std::vector<std::string> list)
{

    // Empty list
    if (list.size() == 0)
        return;

    // Find the current playlist in the list
    std::vector<std::string>::iterator it = list.begin();
    while (*it != getPlaylistName() && it != list.end())
        ++it;

    // If current playlist not found, switch to the first found cycle playlist in the playlist list
    if (it == list.end())
    {
        for (std::vector<std::string>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
        {
            selectPlaylist( *it2 );
            if (*it2 == getPlaylistName())
                break;
        }
    }
    // Current playlist found; switch to the previous found playlist in the list
    else
    {
        for(;;)
        {
            if (it == list.begin()) it = list.end(); // wrap
            --it;
            selectPlaylist( *it );
            if (*it == getPlaylistName())
                break;
        }
    }
    
}


void Page::update(float dt)
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            menu->update(dt);
        }
    }

    if(textStatusComponent_)
    {
        std::string status;
        config_.setProperty("status", status);
        textStatusComponent_->setText(status);
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        if(*it) (*it)->update(dt);
    }

}


void Page::cleanup()
{
    std::list<MenuInfo_S>::iterator del = deleteCollections_.begin();

    while(del != deleteCollections_.end())
    {
        MenuInfo_S &info = *del;
        if(info.queueDelete)
        {
            std::list<MenuInfo_S>::iterator next = del;
            ++next;

            if(info.collection)
            {
                info.collection->Save();
                delete info.collection;
            }
            deleteCollections_.erase(del);
            del = next;
        }
        else
        {
            ++del;
        }
    }
}


void Page::draw()
{
    for(unsigned int i = 0; i < NUM_LAYERS; ++i)
    {
        for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
        {
            if(*it && (*it)->baseViewInfo.Layer == i) (*it)->draw();
        }

        for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
        {
            for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
            {
                ScrollingList *menu = *it2;
                menu->draw(i);
            }
        }
    }

}


void Page::removePlaylist()
{
    if(!selectedItem_) return;

    MenuInfo_S &info = collections_.back();
    CollectionInfo *collection = info.collection;

    std::vector<Item *> *items = collection->playlists["favorites"];
    std::vector<Item *>::iterator it = std::find(items->begin(), items->end(), selectedItem_);

    if(it != items->end())
    {
        items->erase(it);
        selectedItem_->isFavorite = false;
        collection->sortPlaylists();
        collection->saveRequest = true;
    }
    collection->Save();
}


void Page::addPlaylist()
{
    if(!selectedItem_) return;

    MenuInfo_S &info = collections_.back();
    CollectionInfo *collection = info.collection;

    std::vector<Item *> *items = collection->playlists["favorites"];
    if(playlist_->first != "favorites" && std::find(items->begin(), items->end(), selectedItem_) == items->end())
    {
        items->push_back(selectedItem_);
        selectedItem_->isFavorite = true;
        collection->sortPlaylists();
        collection->saveRequest = true;
    }
    collection->Save();
}


void Page::togglePlaylist()
{
    if (!selectedItem_) return;

    if (selectedItem_->isFavorite)
        removePlaylist();
    else
        addPlaylist();
}


std::string Page::getCollectionName()
{
    if(collections_.size() == 0) return "";

    MenuInfo_S &info = collections_.back();
    return info.collection->name;

}


CollectionInfo *Page::getCollection()
{
    return collections_.back().collection;
}


void Page::freeGraphicsMemory()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            menu->freeGraphicsMemory();
        }
    }

    if(loadSoundChunk_) loadSoundChunk_->free();
    if(unloadSoundChunk_) unloadSoundChunk_->free();
    if(highlightSoundChunk_) highlightSoundChunk_->free();
    if(selectSoundChunk_) selectSoundChunk_->free();

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->freeGraphicsMemory();
    }
}


void Page::allocateGraphicsMemory()
{
    Logger::write(Logger::ZONE_DEBUG, "Page", "Allocating graphics memory");

    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        if ( std::distance(menus_.begin(), it) < (signed int)menuDepth_ )
        {
            for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
            {
                ScrollingList *menu = *it2;
                menu->allocateGraphicsMemory();
            }
        }
    }

    if(loadSoundChunk_) loadSoundChunk_->allocate();
    if(unloadSoundChunk_) unloadSoundChunk_->allocate();
    if(highlightSoundChunk_) highlightSoundChunk_->allocate();
    if(selectSoundChunk_) selectSoundChunk_->allocate();

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->allocateGraphicsMemory();
    }
    Logger::write(Logger::ZONE_DEBUG, "Page", "Allocate graphics memory complete");
}


void Page::deInitializeFonts()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            menu->deInitializeFonts( );
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->deInitializeFonts();
    }
}


void Page::initializeFonts()
{
    for(MenuVector_T::iterator it = menus_.begin(); it != menus_.end(); it++)
    {
        for(std::vector<ScrollingList *>::iterator it2 = menus_[std::distance(menus_.begin(), it)].begin(); it2 != menus_[std::distance(menus_.begin(), it)].end(); it2++)
        {
            ScrollingList *menu = *it2;
            menu->initializeFonts( );
        }
    }

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->initializeFonts( );
    }
}


void Page::playSelect()
{
    if(selectSoundChunk_)
    {
        selectSoundChunk_->play();
    }
}


bool Page::isSelectPlaying()
{
    if ( selectSoundChunk_ )
    {
        return selectSoundChunk_->isPlaying();
    }
    return false;
}


void Page::reallocateMenuSpritePoints()
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu)
        {
            menu->deallocateSpritePoints();
            menu->allocateSpritePoints();
        }
    }
}


bool Page::isMenuScrolling()
{
    return scrollActive_;
}


bool Page::isPlaying()
{

    bool retVal = false;

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        retVal |= (*it)->isPlaying();
    }

    return retVal;

}


void Page::resetScrollPeriod()
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu) menu->resetScrollPeriod();
    }
    return;
}


void Page::updateScrollPeriod()
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu) menu->updateScrollPeriod();
    }
    return;
}


void Page::scroll(bool forward)
{
    for(std::vector<ScrollingList *>::iterator it = activeMenu_.begin(); it != activeMenu_.end(); it++)
    {
        ScrollingList *menu = *it;
        if(menu)
        {
            menu->scroll(forward);
        }
    }
    onNewScrollItemSelected();
    if(highlightSoundChunk_)
    {
        highlightSoundChunk_->play();
    }
    return;
}


bool Page::hasSubs()
{
    return collections_.back().collection->hasSubs;
}


int Page::getLayoutWidth(int monitor)
{
    if ( monitor < SDL::getNumScreens( ) )
        return layoutWidth_[monitor];
    else
        return 0;
}


int Page::getLayoutHeight(int monitor)
{
    if ( monitor < SDL::getNumScreens( ) )
        return layoutHeight_[monitor];
    else
        return 0;
}


void Page::setLayoutWidth(int monitor, int width)
{
    if ( monitor < SDL::getNumScreens( ) )
        layoutWidth_[monitor] = width;
}


void Page::setLayoutHeight(int monitor, int height)
{
    if ( monitor < SDL::getNumScreens( ) )
        layoutHeight_[monitor] = height;
}


void Page::setJukebox()
{
    jukebox_ = true;
    return;
}


bool Page::isJukebox()
{
    return jukebox_;
}


bool Page::isJukeboxPlaying()
{

    bool retVal = false;

    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        retVal |= (*it)->isJukeboxPlaying();
    }

    return retVal;

}


void Page::skipForward( )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->skipForward( );
    }
}


void Page::skipBackward( )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->skipBackward( );
    }
}


void Page::skipForwardp( )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->skipForwardp( );
    }
}


void Page::skipBackwardp( )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->skipBackwardp( );
    }
}


void Page::pause( )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->pause( );
    }
}


void Page::restart( )
{
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        (*it)->restart( );
    }
}


unsigned long long Page::getCurrent( )
{
    unsigned long long ret = 0;
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        ret += (*it)->getCurrent( );
    }
    return ret;
}


unsigned long long Page::getDuration( )
{
    unsigned long long ret = 0;
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        ret += (*it)->getDuration( );
    }
    return ret;
}


bool Page::isPaused( )
{
    bool ret = false;
    for(std::vector<Component *>::iterator it = LayerComponents.begin(); it != LayerComponents.end(); ++it)
    {
        ret |= (*it)->isPaused( );
    }
    return ret;
}
