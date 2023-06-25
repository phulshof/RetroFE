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

#include "../Collection/CollectionInfo.h"

#include <map>
#include <string>
#include <list>
#include <vector>

class Component;
class Configuration;
class ScrollingList;
class Text;
class Item;
class Sound;

class Page
{
public:
    enum ScrollDirection
    {
        ScrollDirectionForward,
        ScrollDirectionBack,
        ScrollDirectionIdle

    };

    Page(Configuration &c, int layoutWidth, int layoutHeight );
    virtual ~Page();
    void deInitialize();
    virtual void onNewItemSelected();
    virtual void onNewScrollItemSelected();
    void returnToRememberSelectedItem();
    void rememberSelectedItem();
    std::map<std::string, unsigned int> getLastPlaylistOffsets();
    void highlightLoadArt();
    bool pushCollection(CollectionInfo *collection);
    bool popCollection();
    void enterMenu();
    void exitMenu();
    void enterGame();
    void exitGame();
    std::string getPlaylistName();
    void favPlaylist();
    void nextPlaylist();
    void prevPlaylist();
    void selectPlaylist(std::string playlist);
    void nextCyclePlaylist(std::vector<std::string> list);
    void prevCyclePlaylist(std::vector<std::string> list);
    void pushMenu(ScrollingList *s, int index = -1);
    void updatePlaylistMenuPosition();
    bool isMenusFull();
    void setLoadSound(Sound *chunk);
    void setUnloadSound(Sound *chunk);
    void setHighlightSound(Sound *chunk);
    void setSelectSound(Sound *chunk);
    Item *getSelectedMenuItem();
    ScrollingList* getAnActiveMenu();
    bool addComponent(Component *c);
    void pageScroll(ScrollDirection direction);
    void letterScroll(ScrollDirection direction);
    void metaScroll(ScrollDirection direction, std::string attribute);
    void cfwLetterSubScroll(ScrollDirection direction);
    unsigned int getCollectionSize();
    unsigned int getSelectedIndex();
    void selectRandom();
    void start();
    void stop();
    void setScrolling(ScrollDirection direction);
    bool isHorizontalScroll();
    unsigned int getMenuDepth();
    Item *getSelectedItem();
    Item *getSelectedItem(int offset);
    void removeSelectedItem();
    void setScrollOffsetIndex(unsigned int i);
    unsigned int getScrollOffsetIndex();
    bool isIdle();
    bool isAttractIdle();
    bool isGraphicsIdle();
    bool isMenuIdle();
    void setStatusTextComponent(Text *t);
    void update(float dt);
    void updateReloadables(float dt);
    void cleanup();
    void draw();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void deInitializeFonts( );
    void initializeFonts( );
    void playSelect();
    bool isSelectPlaying();
    std::string getCollectionName();
    CollectionInfo *getCollection();
    void  setMinShowTime(float value);
    float getMinShowTime();
    std::string controlsType();
    void setControlsType(std::string type);
    void  menuScroll();
    void  highlightEnter();
    void  highlightExit();
    void  playlistEnter();
    void  playlistExit();
    void playlistNextEnter();
    void playlistNextExit();
    void playlistPrevEnter();
    void playlistPrevExit();
    void triggerEventOnAllMenus(std::string event);
    void  menuJumpEnter();
    void  menuJumpExit();
    void  attractEnter( );
    void  attract( );
    void  attractExit( );
    void  jukeboxJump( );
    void  triggerEvent( std::string action );
    void  setText( std::string text, int id );
    void  addPlaylist();
    void  removePlaylist();
    void  togglePlaylist();
    void  reallocateMenuSpritePoints(bool updatePlaylistMenu = true);
    bool  isMenuScrolling();
    bool  isPlaying();
    void  resetScrollPeriod();
    void  updateScrollPeriod();
    void  scroll(bool forward);
    bool  hasSubs();
    int   getLayoutWidth(int monitor);
    int   getLayoutHeight(int monitor);
    void  setLayoutWidth(int monitor, int width);
    void  setLayoutHeight(int monitor, int height);
    void  setJukebox();
    bool  isJukebox();
    bool  isJukeboxPlaying();
    void  skipForward( );
    void  skipBackward( );
    void  skipForwardp( );
    void  skipBackwardp( );
    void  pause( );
    void  restart( );
    unsigned long long getCurrent( );
    unsigned long long getDuration( );
    bool  isPaused( );
    void setLocked(bool locked);
    bool isLocked();
    ScrollingList* getPlaylistMenu();
    void setPlaylistMenu(ScrollingList*);
    bool playlistExists(std::string);
    void setSelectedItem();
    bool fromPreviousPlaylist = false;
    bool fromPlaylistNav = false;

private:
    void playlistChange();
    std::string collectionName_;
    Configuration &config_;
    std::string controlsType_;
    bool locked_;

    struct MenuInfo_S
    {
        CollectionInfo* collection;
        CollectionInfo::Playlists_T::iterator playlist;
        bool queueDelete;
    };
    typedef std::list<MenuInfo_S> CollectionVector_T;
    
    typedef std::vector< std::vector<ScrollingList *> > MenuVector_T;
    void setActiveMenuItemsFromPlaylist(MenuInfo_S info, ScrollingList* menu);

    std::vector<ScrollingList *> activeMenu_;
    ScrollingList* anActiveMenu_;
    ScrollingList* playlistMenu_;
    unsigned int menuDepth_;
    MenuVector_T menus_;
    CollectionVector_T collections_;
    CollectionVector_T deleteCollections_;

    static const unsigned int NUM_LAYERS = 20;
    std::vector<Component *> LayerComponents;
    std::list<ScrollingList *> deleteMenuList_;
    std::list<CollectionInfo *> deleteCollectionList_;
    std::map<std::string, unsigned int> lastPlaylistOffsets_;

    bool scrollActive_;

    Item *selectedItem_;
    Text *textStatusComponent_;
    Sound *loadSoundChunk_;
    Sound *unloadSoundChunk_;
    Sound *highlightSoundChunk_;
    Sound *selectSoundChunk_;
    float minShowTime_;
    float elapsedTime_;
    CollectionInfo::Playlists_T::iterator playlist_;
    std::vector<int> layoutWidth_;
    std::vector<int> layoutHeight_;
    bool jukebox_;

};
