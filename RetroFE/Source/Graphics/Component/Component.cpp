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
#include "Component.h"
#include "../Animate/Tween.h"
#include "../../Graphics/ViewInfo.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../PageBuilder.h"

Component::Component(Page &p)
: page(p)
{
    tweens_                   = NULL;
    backgroundTexture_        = NULL;
    menuScrollReload_         = false;
    freeGraphicsMemory();
    id_                       = -1;
}

Component::Component(const Component &copy)
    : page(copy.page)
{
    tweens_ = NULL;
    backgroundTexture_ = NULL;
    freeGraphicsMemory();

    if ( copy.tweens_ )
    {
        AnimationEvents *tweens = new AnimationEvents(*copy.tweens_);
        setTweens(tweens);
    }


}

Component::~Component()
{
    freeGraphicsMemory();
}

void Component::freeGraphicsMemory()
{
    animationRequestedType_ = "";
    animationType_          = "";
    animationRequested_     = false;
    newItemSelected         = false;
    newScrollItemSelected   = false;
    menuIndex_              = -1;

    currentTweens_        = NULL;
    currentTweenIndex_    = 0;
    currentTweenComplete_ = true;
    elapsedTweenTime_     = 0;

    if ( backgroundTexture_ )
    {
        SDL_LockMutex(SDL::getMutex());
        SDL_DestroyTexture(backgroundTexture_);
        SDL_UnlockMutex(SDL::getMutex());

        backgroundTexture_ = NULL;
    }
}
void Component::allocateGraphicsMemory()
{
    if ( !backgroundTexture_ )
    {
        // make a 4x4 pixel wide surface to be stretched during rendering, make it a white background so we can use
        // color  later
        SDL_Surface *surface = SDL_CreateRGBSurface(0, 4, 4, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

        SDL_LockMutex(SDL::getMutex());
        backgroundTexture_ = SDL_CreateTextureFromSurface(SDL::getRenderer(baseViewInfo.Monitor), surface);
        SDL_UnlockMutex(SDL::getMutex());

        SDL_FreeSurface(surface);
        SDL_SetTextureBlendMode(backgroundTexture_, SDL_BLENDMODE_BLEND);
    }
}


void Component::deInitializeFonts()
{
}


void Component::initializeFonts()
{
}


void Component::triggerEvent(std::string event, int menuIndex)
{
    animationRequestedType_ = event;
    animationRequested_     = true;
    menuIndex_              = (menuIndex > 0 ? menuIndex : 0);
}

void Component::setPlaylist(std::string name)
{
    this->playlistName = name;
}

void Component::setNewItemSelected()
{
    newItemSelected = true;
}

void Component::setNewScrollItemSelected()
{
    newScrollItemSelected = true;
}

void Component::setId( int id )
{
    id_ = id;
}

bool Component::isIdle()
{
    return (currentTweenComplete_ || animationType_ == "idle" || animationType_ == "menuIdle" || animationType_ == "attract");
}

bool Component::isAttractIdle()
{
    return (currentTweenComplete_ || animationType_ == "idle" || animationType_ == "menuIdle");
}

bool Component::isMenuScrolling()
{
    return (!currentTweenComplete_ && animationType_ == "menuScroll");
}

void Component::setTweens(AnimationEvents *set)
{
    tweens_ = set;
}

void Component::update(float dt)
{
    elapsedTweenTime_ += dt;

    if ( animationRequested_ && animationRequestedType_ != "" )
    {
      Animation *newTweens;
      // Check if this component is part of an active scrolling list
      if ( menuIndex_ >= MENU_INDEX_HIGH )
      {
          // Check for animation at index i
          newTweens = tweens_->getAnimation( animationRequestedType_, MENU_INDEX_HIGH );
          if ( !(newTweens && newTweens->size() > 0) )
          {
              // Check for animation at the current menuIndex
              newTweens = tweens_->getAnimation( animationRequestedType_, menuIndex_ - MENU_INDEX_HIGH);
          }
      }
      else
      {
          // Check for animation at the current menuIndex
          newTweens = tweens_->getAnimation( animationRequestedType_, menuIndex_ );
      }
      if (newTweens && newTweens->size() > 0)
      {
        animationType_        = animationRequestedType_;
        currentTweens_        = newTweens;
        currentTweenIndex_    = 0;
        elapsedTweenTime_     = 0;
        storeViewInfo_        = baseViewInfo;
        currentTweenComplete_ = false;
      }
      animationRequested_   = false;
    }

    if (tweens_ && currentTweenComplete_)
    {
        animationType_        = "idle";
        currentTweens_        = tweens_->getAnimation( "idle", menuIndex_ );
        if ( currentTweens_ && currentTweens_->size( ) == 0 && !page.isMenuScrolling( ) )
        {
            currentTweens_    = tweens_->getAnimation( "menuIdle", menuIndex_ );
            if ( currentTweens_ && currentTweens_->size( ) > 0 )
            {
                currentTweens_ = currentTweens_;
            }
        }
        currentTweenIndex_    = 0;
        elapsedTweenTime_     = 0;
        storeViewInfo_        = baseViewInfo;
        currentTweenComplete_ = false;
        animationRequested_   = false;
    }

    currentTweenComplete_ = animate();
    if ( currentTweenComplete_ )
    {
      currentTweens_     = NULL;
      currentTweenIndex_ = 0;
    }
}

void Component::draw()
{

    if ( backgroundTexture_ )
    {
        SDL_Rect rect;
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());
        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());


        SDL_SetTextureColorMod(backgroundTexture_,
                               static_cast<char>(baseViewInfo.BackgroundRed*255),
                               static_cast<char>(baseViewInfo.BackgroundGreen*255),
                               static_cast<char>(baseViewInfo.BackgroundBlue*255));

        SDL::renderCopy(backgroundTexture_, baseViewInfo.BackgroundAlpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));
    }
}

bool Component::animate()
{
    bool completeDone = false;
    if ( !currentTweens_ || currentTweenIndex_ >= currentTweens_->size() )
    {
        completeDone = true;
    }
    else if ( currentTweens_ )
    {
        bool currentDone = true;
        TweenSet *tweens = currentTweens_->tweenSet(currentTweenIndex_);

        for(unsigned int i = 0; i < tweens->size(); i++)
        {
            Tween *tween = tweens->tweens()->at(i);
            double elapsedTime = elapsedTweenTime_;

            //todo: too many levels of nesting
            if ( elapsedTime < tween->duration )
                currentDone = false;
            else
                elapsedTime = static_cast<float>(tween->duration);

            switch(tween->property)
            {
            case TWEEN_PROPERTY_X:
                if (tween->startDefined)
                    baseViewInfo.X = tween->animate(elapsedTime);
                else
                    baseViewInfo.X = tween->animate(elapsedTime, storeViewInfo_.X);
                break;

            case TWEEN_PROPERTY_Y:
                if (tween->startDefined)
                    baseViewInfo.Y = tween->animate(elapsedTime);
                else
                    baseViewInfo.Y = tween->animate(elapsedTime, storeViewInfo_.Y);
                break;

            case TWEEN_PROPERTY_HEIGHT:
                if (tween->startDefined)
                    baseViewInfo.Height = tween->animate(elapsedTime);
                else
                    baseViewInfo.Height = tween->animate(elapsedTime, storeViewInfo_.Height);
                break;

            case TWEEN_PROPERTY_WIDTH:
                if (tween->startDefined)
                    baseViewInfo.Width = tween->animate(elapsedTime);
                else
                    baseViewInfo.Width = tween->animate(elapsedTime, storeViewInfo_.Width);
                break;

            case TWEEN_PROPERTY_ANGLE:
                if (tween->startDefined)
                    baseViewInfo.Angle = tween->animate(elapsedTime);
                else
                    baseViewInfo.Angle = tween->animate(elapsedTime, storeViewInfo_.Angle);
                break;

            case TWEEN_PROPERTY_ALPHA:
                if (tween->startDefined)
                    baseViewInfo.Alpha = tween->animate(elapsedTime);
                else
                    baseViewInfo.Alpha = tween->animate(elapsedTime, storeViewInfo_.Alpha);
                break;

            case TWEEN_PROPERTY_X_ORIGIN:
                if (tween->startDefined)
                    baseViewInfo.XOrigin = tween->animate(elapsedTime);
                else
                    baseViewInfo.XOrigin = tween->animate(elapsedTime, storeViewInfo_.XOrigin);
                break;

            case TWEEN_PROPERTY_Y_ORIGIN:
                if (tween->startDefined)
                    baseViewInfo.YOrigin = tween->animate(elapsedTime);
                else
                    baseViewInfo.YOrigin = tween->animate(elapsedTime, storeViewInfo_.YOrigin);
                break;

            case TWEEN_PROPERTY_X_OFFSET:
                if (tween->startDefined)
                    baseViewInfo.XOffset = tween->animate(elapsedTime);
                else
                    baseViewInfo.XOffset = tween->animate(elapsedTime, storeViewInfo_.XOffset);
                break;

            case TWEEN_PROPERTY_Y_OFFSET:
                if (tween->startDefined)
                    baseViewInfo.YOffset = tween->animate(elapsedTime);
                else
                    baseViewInfo.YOffset = tween->animate(elapsedTime, storeViewInfo_.YOffset);
                break;

            case TWEEN_PROPERTY_FONT_SIZE:
                if (tween->startDefined)
                    baseViewInfo.FontSize = tween->animate(elapsedTime);
                else
                    baseViewInfo.FontSize = tween->animate(elapsedTime, storeViewInfo_.FontSize);
                break;

            case TWEEN_PROPERTY_BACKGROUND_ALPHA:
                if (tween->startDefined)
                    baseViewInfo.BackgroundAlpha = tween->animate(elapsedTime);
                else
                    baseViewInfo.BackgroundAlpha = tween->animate(elapsedTime, storeViewInfo_.BackgroundAlpha);
                break;

            case TWEEN_PROPERTY_MAX_WIDTH:
                if (tween->startDefined)
                    baseViewInfo.MaxWidth = tween->animate(elapsedTime);
                else
                    baseViewInfo.MaxWidth = tween->animate(elapsedTime, storeViewInfo_.MaxWidth);
                break;

            case TWEEN_PROPERTY_MAX_HEIGHT:
                if (tween->startDefined)
                    baseViewInfo.MaxHeight = tween->animate(elapsedTime);
                else
                    baseViewInfo.MaxHeight = tween->animate(elapsedTime, storeViewInfo_.MaxHeight);
                break;

            case TWEEN_PROPERTY_LAYER:
                if (tween->startDefined)
                    baseViewInfo.Layer = static_cast<unsigned int>(tween->animate(elapsedTime));
                else
                    baseViewInfo.Layer = static_cast<unsigned int>(tween->animate(elapsedTime, storeViewInfo_.Layer));
                break;

            case TWEEN_PROPERTY_CONTAINER_X:
                if (tween->startDefined)
                    baseViewInfo.ContainerX = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerX = tween->animate(elapsedTime, storeViewInfo_.ContainerX);
                break;

            case TWEEN_PROPERTY_CONTAINER_Y:
                if (tween->startDefined)
                    baseViewInfo.ContainerY = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerY = tween->animate(elapsedTime, storeViewInfo_.ContainerY);
                break;

            case TWEEN_PROPERTY_CONTAINER_WIDTH:
                if (tween->startDefined)
                    baseViewInfo.ContainerWidth = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerWidth = tween->animate(elapsedTime, storeViewInfo_.ContainerWidth);
                break;

            case TWEEN_PROPERTY_CONTAINER_HEIGHT:
                if (tween->startDefined)
                    baseViewInfo.ContainerHeight = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerHeight = tween->animate(elapsedTime, storeViewInfo_.ContainerHeight);
                break;

            case TWEEN_PROPERTY_VOLUME:
                if (tween->startDefined)
                    baseViewInfo.Volume = tween->animate(elapsedTime);
                else
                    baseViewInfo.Volume = tween->animate(elapsedTime, storeViewInfo_.Volume);
                break;

            case TWEEN_PROPERTY_MONITOR:
                if (tween->startDefined)
                    baseViewInfo.Monitor = static_cast<unsigned int>(tween->animate(elapsedTime));
                else
                    baseViewInfo.Monitor = static_cast<unsigned int>(tween->animate(elapsedTime, storeViewInfo_.Monitor));
                break;

            case TWEEN_PROPERTY_NOP:
                break;
            }
        }

        if ( currentDone )
        {
            currentTweenIndex_++;
            elapsedTweenTime_ = 0;
            storeViewInfo_    = baseViewInfo;
        }
    }

    if ( !currentTweens_ || currentTweenIndex_ >= currentTweens_->tweenSets()->size() )
    {
        completeDone = true;
    }

    return completeDone;
}


bool Component::isPlaying()
{
    return false;
}




bool Component::isJukeboxPlaying()
{
    return false;
}


void Component::setMenuScrollReload(bool menuScrollReload)
{
    menuScrollReload_ = menuScrollReload;
}


bool Component::getMenuScrollReload()
{
    return menuScrollReload_;
}

int Component::getId( )
{
    return id_;
}
