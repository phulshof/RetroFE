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

#include <vector>

#include "../../SDL.h"
#include "../Page.h"
#include "../ViewInfo.h"
#include "../Animate/Tween.h"
#include "../Animate/AnimationEvents.h"
#include "../../Collection/Item.h"
class Component
{
public:
    Component(Page &p);
    Component(const Component &copy);
    virtual ~Component();
    virtual void freeGraphicsMemory();
    virtual void allocateGraphicsMemory();
    virtual void deInitializeFonts();
    virtual void initializeFonts();
    void triggerEvent(std::string event, int menuIndex = -1);
    void setPlaylist(std::string name );
    void setNewItemSelected();
    void setNewScrollItemSelected();
    bool isIdle();
    bool isAttractIdle();
    bool isMenuScrolling();
    bool newItemSelected;
    bool newScrollItemSelected;
    void setId( int id );

    virtual bool update(float dt);
    virtual void draw();
    void setTweens(AnimationEvents *set);
    virtual bool isPlaying();
    virtual bool isJukeboxPlaying();
    virtual void skipForward( ) {};
    virtual void skipBackward( ) {};
    virtual void skipForwardp( ) {};
    virtual void skipBackwardp( ) {};
    virtual void pause( ) {};
    virtual void restart( ) {};
    virtual unsigned long long getCurrent( ) {return 0;};
    virtual unsigned long long getDuration( ) {return 0;};
    virtual bool isPaused( ) {return false;};
    ViewInfo baseViewInfo;
    std::string collectionName;
    void setMenuScrollReload(bool menuScrollReload);
    bool getMenuScrollReload();
    void setAnimationDoneRemove(bool value);
    bool getAnimationDoneRemove();
    void setPauseOnScroll(bool value);
    bool getPauseOnScroll() const;
    virtual void setText(std::string text, int id = -1) {};
    virtual void setImage(std::string filePath, int id = -1) {};
    int getId( );
    std::string playlistName;
    

protected:
    Page &page;

private:

    bool animate();
    bool tweenSequencingComplete();

    AnimationEvents *tweens_;
    Animation *currentTweens_;
    SDL_Texture *backgroundTexture_;
    bool         pauseOnScroll_;
    ViewInfo     storeViewInfo_;
    unsigned int currentTweenIndex_;
    bool         currentTweenComplete_;
    float        elapsedTweenTime_;
    std::string  animationRequestedType_;
    std::string  animationType_;
    bool         animationRequested_;
    bool         menuScrollReload_;
    bool         animationDoneRemove_;
    int          menuIndex_;
    int          id_;

};
