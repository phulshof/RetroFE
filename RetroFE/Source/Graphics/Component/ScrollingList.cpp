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


#include "ScrollingList.h"
#include "../Animate/Tween.h"
#include "../Animate/TweenSet.h"
#include "../Animate/Animation.h"
#include "../Animate/AnimationEvents.h"
#include "../Animate/TweenTypes.h"
#include "../Font.h"
#include "ImageBuilder.h"
#include "VideoBuilder.h"
#include "VideoComponent.h"
#include "ReloadableMedia.h"
#include "Text.h"
#include "../../Database/Configuration.h"
#include "../../Collection/Item.h"
#include "../../Utility/Utils.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../ViewInfo.h"
#include <math.h>
#include <SDL2/SDL_image.h>
#include <sstream>
#include <cctype>
#include <iomanip>


ScrollingList::ScrollingList( Configuration &c,
                              Page          &p,
                              bool           layoutMode,
                              bool           commonMode,
                              bool          playlistType,
                              Font          *font,
                              std::string    layoutKey,
                              std::string    imageType,
                              std::string    videoType)
    : Component( p )
    , horizontalScroll( false )
    , layoutMode_( layoutMode )
    , commonMode_( commonMode )
    , playlistType_( playlistType )
    , spriteList_( NULL )
    , scrollPoints_( NULL )
    , tweenPoints_( NULL )
    , itemIndex_( 0 )
    , selectedOffsetIndex_( 0 )
    , scrollAcceleration_( 0 )
    , startScrollTime_( 0.500 )
    , minScrollTime_( 0.500 )
    , scrollPeriod_( 0 )
    , config_( c )
    , fontInst_( font )
    , layoutKey_( layoutKey )
    , imageType_( imageType )
    , videoType_( videoType )
    , items_( NULL )
{
}


ScrollingList::ScrollingList( const ScrollingList &copy )
    : Component( copy )
    , horizontalScroll( copy.horizontalScroll )
    , layoutMode_( copy.layoutMode_ )
    , commonMode_( copy.commonMode_ )
    , playlistType_(copy.playlistType_)
    , spriteList_( NULL )
    , itemIndex_( 0 )
    , selectedOffsetIndex_( copy.selectedOffsetIndex_ )
    , scrollAcceleration_( copy.scrollAcceleration_ )
    , startScrollTime_( copy.startScrollTime_ )
    , minScrollTime_( copy.minScrollTime_ )
    , scrollPeriod_( copy.startScrollTime_ )
    , config_( copy.config_ )
    , fontInst_( copy.fontInst_ )
    , layoutKey_( copy.layoutKey_ )
    , imageType_( copy.imageType_ )
    , items_( NULL )
{
    scrollPoints_ = NULL;
    tweenPoints_  = NULL;

    setPoints( copy.scrollPoints_, copy.tweenPoints_ );

}


ScrollingList::~ScrollingList( )
{
    destroyItems( );
    while( scrollPoints_->size( ) > 0 )
    {
        ViewInfo *scrollPoint = scrollPoints_->back( );
        delete scrollPoint;
        scrollPoints_->pop_back( );
    }
}

std::vector<Item*> ScrollingList::getItems()
{
    return *items_;
}

void ScrollingList::setItems( std::vector<Item *> *items )
{
    items_ = items;
    if (items_)
    {
        itemIndex_ = loopDecrement(0, selectedOffsetIndex_, items_->size());
    }
}


unsigned int ScrollingList::loopIncrement( unsigned int offset, unsigned int i, unsigned int size )
{
    if ( size == 0 ) return 0;
    return (offset + i ) % size;
}


unsigned int ScrollingList::loopDecrement( unsigned int offset, unsigned int i, unsigned int size )
{
    if ( size == 0 ) return 0;
    return ((offset % size ) - (i % size ) + size ) % size; 
}


void ScrollingList::setScrollAcceleration( float value )
{
    scrollAcceleration_ = value;
}


void ScrollingList::setStartScrollTime( float value )
{
    startScrollTime_ = value;
}


void ScrollingList::setMinScrollTime( float value )
{
    minScrollTime_ = value;
}


void ScrollingList::deallocateSpritePoints( )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        deallocateTexture( i );
    }
}


void ScrollingList::allocateSpritePoints( )
{
    if ( !items_ || items_->size( ) == 0 ) return;
    if ( !scrollPoints_ ) return;
    if ( components_.size( ) == 0 ) return;

    for ( unsigned int i = 0; i < scrollPoints_->size( ); ++i )
    {
        unsigned int index  = loopIncrement( itemIndex_, i, items_->size( ) );
        Item *item = items_->at( index );

        Component *old = components_.at( i );

        allocateTexture( i, item );

        Component *c = components_.at( i );
        c->allocateGraphicsMemory( );

        ViewInfo *view = scrollPoints_->at( i );

        resetTweens( c, tweenPoints_->at( i ), view, view, 0 );

        if ( old && !newItemSelected )
        {
            c->baseViewInfo = old->baseViewInfo;
            delete old;
        }

    }
}


void ScrollingList::destroyItems( )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        if ( components_.at( i ) )
        {
            components_.at( i )->freeGraphicsMemory( );
            delete components_.at( i );
        }
        components_.at( i ) = NULL;
    }
}


void ScrollingList::setPoints( std::vector<ViewInfo *> *scrollPoints, std::vector<AnimationEvents *> *tweenPoints )
{
    scrollPoints_ = scrollPoints;
    tweenPoints_  = tweenPoints;

    // empty out the list as we will resize it
    components_.clear( );

    int size = 0;

    if ( scrollPoints )
    {
        size = scrollPoints_->size( );
    }
    components_.resize( size );

    if ( items_ )
    {
        itemIndex_ = loopDecrement( 0, selectedOffsetIndex_, items_->size( ) );
    }
}


unsigned int ScrollingList::getScrollOffsetIndex( )
{
    return loopIncrement( itemIndex_, selectedOffsetIndex_, items_->size( ) );
}


void ScrollingList::setScrollOffsetIndex( unsigned int index )
{
    itemIndex_ = loopDecrement( index, selectedOffsetIndex_, items_->size( ) );
}


void ScrollingList::setSelectedIndex( int selectedIndex )
{
    selectedOffsetIndex_ = selectedIndex;
}


Item *ScrollingList::getItemByOffset( int offset )
{

    if ( !items_ || items_->size( ) == 0 ) return NULL;

    unsigned int index = getSelectedIndex( );
    if ( offset >= 0 )
    {
        index = loopIncrement( index, offset, items_->size( ) );
    }
    else
    {
        index = loopDecrement( index, offset*-1, items_->size( ) );
    }
    
    return items_->at( index );

}


Item *ScrollingList::getSelectedItem( )
{
    if ( !items_ || items_->size( ) == 0 ) return NULL;
    return items_->at( loopIncrement( itemIndex_, selectedOffsetIndex_, items_->size( ) ) );
}


void ScrollingList::pageUp( )
{
    if ( components_.size( ) == 0 ) return;
    itemIndex_ = loopDecrement( itemIndex_, components_.size( ), items_->size( ) );
}


void ScrollingList::pageDown( )
{
    if ( components_.size( ) == 0 ) return;
    itemIndex_ = loopIncrement( itemIndex_, components_.size( ), items_->size( ) );
}


void ScrollingList::random( )
{
    if ( !items_ || items_->size( ) == 0 ) return;
    itemIndex_ = rand( ) % items_->size( );
}


void ScrollingList::letterUp( )
{
    letterChange( true );
}


void ScrollingList::letterDown( )
{
    letterChange( false );
}


void ScrollingList::letterChange( bool increment )
{

    if ( !items_ || items_->size( ) == 0 ) return;

    Item       *startItem = items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) );
    std::string startname = items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->lowercaseFullTitle( );

    for ( unsigned int i = 0; i < items_->size( ); ++i )
    {
        unsigned int index = 0;
        if ( increment )
        {
            index = loopIncrement( itemIndex_, i, items_->size( ) );
        }
        else
        {
            index = loopDecrement( itemIndex_, i, items_->size( ) );
        }

        std::string endname = items_->at( (index+selectedOffsetIndex_ ) % items_->size( ) )->lowercaseFullTitle( );

        // check if we are changing characters from a-z, or changing from alpha character to non-alpha character
        if ((isalpha(startname[0] ) ^ isalpha(endname[0] ) ) ||
            (isalpha(startname[0] ) && isalpha(endname[0] ) && startname[0] != endname[0] ) )
        {
            itemIndex_ = index;
            break;
        }
    }

    if ( !increment ) // For decrement, find the first game of the new letter
    {
        bool prevLetterSubToCurrent = false;
        config_.getProperty( "prevLetterSubToCurrent", prevLetterSubToCurrent );
        if ( !prevLetterSubToCurrent || items_->at( (itemIndex_+1+selectedOffsetIndex_ ) % items_->size( ) ) == startItem )
        {
            startname = items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->lowercaseFullTitle( );

            for ( unsigned int i = 0; i < items_->size( ); ++i )
            {
                unsigned int index = loopDecrement( itemIndex_, i, items_->size( ) );

                std::string endname = items_->at( (index+selectedOffsetIndex_ ) % items_->size( ) )->lowercaseFullTitle( );

                // check if we are changing characters from a-z, or changing from alpha character to non-alpha character
                if ((isalpha(startname[0] ) ^ isalpha(endname[0] ) ) ||
                    (isalpha(startname[0] ) && isalpha(endname[0] ) && startname[0] != endname[0] ) )
                {
                    itemIndex_ = loopIncrement( index,1,items_->size( ) );
                    break;
                }
            }
        }
        else
        {
            itemIndex_ = loopIncrement( itemIndex_,1,items_->size( ) );
        }
    }
}


void ScrollingList::subUp( )
{
    subChange( true );
}


void ScrollingList::subDown( )
{
    subChange( false );
}


void ScrollingList::subChange( bool increment )
{

    if ( !items_ || items_->size( ) == 0 ) return;

    Item       *startItem = items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) );
    std::string startname = items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( );

    for ( unsigned int i = 0; i < items_->size( ); ++i )
    {
        unsigned int index = 0;
        if ( increment )
        {
            index = loopIncrement( itemIndex_, i, items_->size( ) );
        }
        else
        {
            index = loopDecrement( itemIndex_, i, items_->size( ) );
        }

        std::string endname = items_->at( (index+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( );

        if (startname != endname)
        {
            itemIndex_ = index;
            break;
        }
    }

    if ( !increment ) // For decrement, find the first game of the new sub
    {
        bool prevLetterSubToCurrent = false;
        config_.getProperty( "prevLetterSubToCurrent", prevLetterSubToCurrent );
        if ( !prevLetterSubToCurrent || items_->at( (itemIndex_+1+selectedOffsetIndex_ ) % items_->size( ) ) == startItem )
        {
            startname = items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( );

            for ( unsigned int i = 0; i < items_->size( ); ++i )
            {
                unsigned int index = loopDecrement( itemIndex_, i, items_->size( ) );

                std::string endname = items_->at( (index+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( );

                if (startname != endname)
                {
                    itemIndex_ = loopIncrement( index,1,items_->size( ) );
                    break;
                }
            }
        }
        else
        {
            itemIndex_ = loopIncrement( itemIndex_,1,items_->size( ) );
        }
    }
}


void ScrollingList::cfwLetterSubUp( )
{
    if (Utils::toLower( collectionName ) != items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( ))
        subChange( true );
    else
        letterChange( true );
}


void ScrollingList::cfwLetterSubDown( )
{
    if (Utils::toLower( collectionName ) != items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( ))
    {
        subChange( false );
        if (Utils::toLower( collectionName ) == items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( ))
        {
            subChange( true );
            letterChange( false );
        }
    }
    else
    {
        letterChange( false );
        if (Utils::toLower( collectionName ) != items_->at( (itemIndex_+selectedOffsetIndex_ ) % items_->size( ) )->collectionInfo->lowercaseName( ))
        {
            letterChange( true );
            subChange( false );
        }
    }
}


void ScrollingList::allocateGraphicsMemory( )
{
    Component::allocateGraphicsMemory( );
    scrollPeriod_ = startScrollTime_;

    allocateSpritePoints( );
}


void ScrollingList::freeGraphicsMemory( )
{
    Component::freeGraphicsMemory( );
    scrollPeriod_ = 0;
    
    deallocateSpritePoints( );
}

void ScrollingList::triggerEnterEvent( )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "enter" );
    }
}

void ScrollingList::triggerExitEvent( )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at(i );
        if ( c ) c->triggerEvent( "exit" );
    }
}

void ScrollingList::triggerMenuEnterEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "menuEnter", menuIndex );
    }
}

void ScrollingList::triggerMenuExitEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "menuExit", menuIndex );
    }
}

void ScrollingList::triggerGameEnterEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "gameEnter", menuIndex );
    }
}

void ScrollingList::triggerGameExitEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "gameExit", menuIndex );
    }
}

void ScrollingList::triggerHighlightEnterEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "highlightEnter", menuIndex );
    }
}

void ScrollingList::triggerHighlightExitEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "highlightExit", menuIndex );
    }
}

void ScrollingList::triggerPlaylistEnterEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "playlistEnter", menuIndex );
    }
}

void ScrollingList::triggerPlaylistExitEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "playlistExit", menuIndex );
    }
}

void ScrollingList::triggerMenuJumpEnterEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "menuJumpEnter", menuIndex );
    }
}

void ScrollingList::triggerMenuJumpExitEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "menuJumpExit", menuIndex );
    }
}

void ScrollingList::triggerAttractEnterEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "attractEnter", menuIndex );
    }
}

void ScrollingList::triggerAttractEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "attract", menuIndex );
    }
}

void ScrollingList::triggerAttractExitEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "attractExit", menuIndex );
    }
}

void ScrollingList::triggerJukeboxJumpEvent( int menuIndex )
{
    for ( unsigned int i = 0; i < components_.size( ); ++i )
    {
        Component *c = components_.at( i );
        if ( c ) c->triggerEvent( "jukeboxJump", menuIndex );
    }
}

void ScrollingList::update( float dt )
{

    Component::update( dt );

    if (components_.size( ) == 0 ) return;
    if (!items_ ) return;

    for ( unsigned int i = 0; i < scrollPoints_->size( ); i++ )
    {
        Component *c = components_.at( i );
        if ( c ) c->update(dt );
    }
}


unsigned int ScrollingList::getSelectedIndex( )
{
    if ( !items_ ) return 0;
    return loopIncrement( itemIndex_, selectedOffsetIndex_, items_->size( ) );
}


void ScrollingList::setSelectedIndex( unsigned int index )
{
     if ( !items_ ) return;
     itemIndex_ = loopDecrement( index, selectedOffsetIndex_, items_->size( ) );
}


unsigned int ScrollingList::getSize( )
{
    if ( !items_ ) return 0;
    return items_->size( );
}


void ScrollingList::resetTweens( Component *c, AnimationEvents *sets, ViewInfo *currentViewInfo, ViewInfo *nextViewInfo, double scrollTime )
{
    if ( !c ) return;
    if ( !sets ) return;
    if ( !currentViewInfo ) return;
    if ( !nextViewInfo ) return;

    currentViewInfo->ImageHeight  = c->baseViewInfo.ImageHeight;
    currentViewInfo->ImageWidth   = c->baseViewInfo.ImageWidth;
    nextViewInfo->ImageHeight     = c->baseViewInfo.ImageHeight;
    nextViewInfo->ImageWidth      = c->baseViewInfo.ImageWidth;
    nextViewInfo->BackgroundAlpha = c->baseViewInfo.BackgroundAlpha;

    c->setTweens(sets );

    Animation *scrollTween = sets->getAnimation("menuScroll" );
    scrollTween->Clear( );
    c->baseViewInfo = *currentViewInfo;

    TweenSet *set = new TweenSet( );
    set->push(new Tween(TWEEN_PROPERTY_HEIGHT, LINEAR, currentViewInfo->Height, nextViewInfo->Height, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_WIDTH, LINEAR, currentViewInfo->Width, nextViewInfo->Width, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_ANGLE, LINEAR, currentViewInfo->Angle, nextViewInfo->Angle, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_ALPHA, LINEAR, currentViewInfo->Alpha, nextViewInfo->Alpha, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_X, LINEAR, currentViewInfo->X, nextViewInfo->X, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_Y, LINEAR, currentViewInfo->Y, nextViewInfo->Y, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_X_ORIGIN, LINEAR, currentViewInfo->XOrigin, nextViewInfo->XOrigin, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_Y_ORIGIN, LINEAR, currentViewInfo->YOrigin, nextViewInfo->YOrigin, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_X_OFFSET, LINEAR, currentViewInfo->XOffset, nextViewInfo->XOffset, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_Y_OFFSET, LINEAR, currentViewInfo->YOffset, nextViewInfo->YOffset, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_FONT_SIZE, LINEAR, currentViewInfo->FontSize, nextViewInfo->FontSize, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_BACKGROUND_ALPHA, LINEAR, currentViewInfo->BackgroundAlpha, nextViewInfo->BackgroundAlpha, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_MAX_WIDTH, LINEAR, currentViewInfo->MaxWidth, nextViewInfo->MaxWidth, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_MAX_HEIGHT, LINEAR, currentViewInfo->MaxHeight, nextViewInfo->MaxHeight, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_LAYER, LINEAR, currentViewInfo->Layer, nextViewInfo->Layer, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_VOLUME, LINEAR, currentViewInfo->Volume, nextViewInfo->Volume, scrollTime ) );
    set->push(new Tween(TWEEN_PROPERTY_MONITOR, LINEAR, currentViewInfo->Monitor, nextViewInfo->Monitor, scrollTime ) );
    scrollTween->Push( set );
}


bool ScrollingList::allocateTexture( unsigned int index, Item *item )
{

    if ( index >= components_.size( ) ) return false;

    std::string imagePath;
    std::string videoPath;

    Component *t = NULL;

    ImageBuilder imageBuild;
    VideoBuilder videoBuild;

    std::string layoutName;
    config_.getProperty( "layout", layoutName );

    std::string typeLC = Utils::toLower( imageType_ );

    std::vector<std::string> names;
    names.push_back( item->name );
    names.push_back( item->fullTitle );
    if ( item->cloneof != "" )
        names.push_back( item->cloneof );
    if ( typeLC == "numberbuttons" )
        names.push_back( item->numberButtons );
    if ( typeLC == "numberplayers" )
        names.push_back( item->numberPlayers );
    if ( typeLC == "year" )
        names.push_back( item->year );
    if ( typeLC == "title" )
        names.push_back( item->title );
    if ( typeLC == "developer" )
    {
        if ( item->developer == "" )
        {
            names.push_back( item->manufacturer );
        }
        else
        {
            names.push_back( item->developer );
        }
    }
    if ( typeLC == "manufacturer" )
        names.push_back( item->manufacturer );
    if ( typeLC == "genre" )
        names.push_back( item->genre );
    if ( typeLC == "ctrltype" )
        names.push_back( item->ctrlType );
    if ( typeLC == "joyways" )
        names.push_back( item->joyWays );
    if ( typeLC == "rating" )
        names.push_back( item->rating );
    if ( typeLC == "score" )
        names.push_back( item->score );
    if (typeLC.rfind("playlist", 0) == 0)
        names.push_back(item->name);
    names.push_back("default");

    for ( unsigned int n = 0; n < names.size() && !t; ++n )
    {
        // check collection path for art
        if ( layoutMode_ )
        {
            if ( commonMode_ )
                imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", "_common");
            else
                imagePath = Utils::combinePath( Configuration::absolutePath, "layouts", layoutName, "collections", collectionName );
            imagePath = Utils::combinePath( imagePath, "medium_artwork", imageType_ );
            videoPath = Utils::combinePath( imagePath, "medium_artwork", videoType_ );
        }
        else
        {
            if ( commonMode_ )
            {
                imagePath = Utils::combinePath(Configuration::absolutePath, "collections", "_common" );
                imagePath = Utils::combinePath( imagePath, "medium_artwork", imageType_ );
                videoPath = Utils::combinePath( imagePath, "medium_artwork", videoType_ );
            }
            else
            {
                config_.getMediaPropertyAbsolutePath( collectionName, imageType_, false, imagePath );
                config_.getMediaPropertyAbsolutePath( collectionName, videoType_, false, videoPath );
            }
        }
        if ( !t )
        {
            if ( videoType_ != "null" )
            {
                t = videoBuild.createVideo( videoPath, page, names[n], baseViewInfo.Monitor, false );
            }
            else
            {
                t = imageBuild.CreateImage( imagePath, page, names[n], baseViewInfo.Monitor );
            }
        }

        // check sub-collection path for art
        if ( !t && !commonMode_ )
        {
            if ( layoutMode_ )
            {
                imagePath = Utils::combinePath( Configuration::absolutePath, "layouts", layoutName, "collections", item->collectionInfo->name );
                imagePath = Utils::combinePath( imagePath, "medium_artwork", imageType_ );
                videoPath = Utils::combinePath( imagePath, "medium_artwork", videoType_ );
            }
            else
            {
                config_.getMediaPropertyAbsolutePath( item->collectionInfo->name, imageType_, false, imagePath );
                config_.getMediaPropertyAbsolutePath( item->collectionInfo->name, videoType_, false, videoPath );
            }
            if ( videoType_ != "null" )
            {
                t = videoBuild.createVideo( videoPath, page, names[n], baseViewInfo.Monitor, false );
            }
            else
            {
                t = imageBuild.CreateImage( imagePath, page, names[n], baseViewInfo.Monitor );
            }
        }
    }

    // check collection path for art based on system name
    if ( !t )
    {
        if ( layoutMode_ )
        {
            if ( commonMode_ )
                imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", "_common");
            else
                imagePath = Utils::combinePath( Configuration::absolutePath, "layouts", layoutName, "collections", item->name );
            imagePath = Utils::combinePath( imagePath, "system_artwork" );
            videoPath = imagePath;
        }
        else
        {
            if ( commonMode_ )
            {
                imagePath = Utils::combinePath(Configuration::absolutePath, "collections", "_common" );
                imagePath = Utils::combinePath( imagePath, "system_artwork" );
                videoPath = imagePath;
            }
            else
            {
                config_.getMediaPropertyAbsolutePath( item->name, imageType_, true, imagePath );
                config_.getMediaPropertyAbsolutePath( item->name, videoType_, true, videoPath );
            }
        }
        if ( videoType_ != "null" )
        {
            t = videoBuild.createVideo( videoPath, page, videoType_, baseViewInfo.Monitor, false );
        }
        else
        {
            t = imageBuild.CreateImage( imagePath, page, imageType_, baseViewInfo.Monitor );
        }
    }

    // check rom directory path for art
    if ( !t )
    {
        if ( videoType_ != "null" )
        {
            t = videoBuild.createVideo( item->filepath, page, videoType_, baseViewInfo.Monitor, false );
        }
        else
        {
            t = imageBuild.CreateImage( item->filepath, page, imageType_, baseViewInfo.Monitor );
        }
    }

    // Check for fallback art in case no video could be found
    if ( videoType_ != "null" && !t)
    {
        for ( unsigned int n = 0; n < names.size() && !t; ++n )
        {
            // check collection path for art
            if ( layoutMode_ )
            {
                if ( commonMode_ )
                    imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", "_common");
                else
                    imagePath = Utils::combinePath( Configuration::absolutePath, "layouts", layoutName, "collections", collectionName );
               imagePath = Utils::combinePath( imagePath, "medium_artwork", imageType_ );
            }
            else
            {
                if ( commonMode_ )
                {
                    imagePath = Utils::combinePath(Configuration::absolutePath, "collections", "_common" );
                    imagePath = Utils::combinePath( imagePath, "medium_artwork", imageType_ );
                }
                else
                {
                    config_.getMediaPropertyAbsolutePath( collectionName, imageType_, false, imagePath );
                }
            }

            t = imageBuild.CreateImage( imagePath, page, names[n], baseViewInfo.Monitor );

            // check sub-collection path for art
            if ( !t && !commonMode_ )
            {
                if ( layoutMode_ )
                {
                    imagePath = Utils::combinePath( Configuration::absolutePath, "layouts", layoutName, "collections", item->collectionInfo->name );
                    imagePath = Utils::combinePath( imagePath, "medium_artwork", imageType_ );
                }
                else
                {
                    config_.getMediaPropertyAbsolutePath( item->collectionInfo->name, imageType_, false, imagePath );
                }
                t = imageBuild.CreateImage( imagePath, page, names[n], baseViewInfo.Monitor );
            }
        }

        // check collection path for art based on system name
        if ( !t )
        {
            if ( layoutMode_ )
            {
                if ( commonMode_ )
                    imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", "_common");
                else
                    imagePath = Utils::combinePath( Configuration::absolutePath, "layouts", layoutName, "collections", item->name );
                imagePath = Utils::combinePath( imagePath, "system_artwork" );
            }
            else
            {
                if ( commonMode_ )
                {
                    imagePath = Utils::combinePath(Configuration::absolutePath, "collections", "_common" );
                    imagePath = Utils::combinePath( imagePath, "system_artwork" );
                }
                else
                {
                    config_.getMediaPropertyAbsolutePath( item->name, imageType_, true, imagePath );
                }
            }
            if ( !t )
            {
                t = imageBuild.CreateImage( imagePath, page, imageType_, baseViewInfo.Monitor );
            }
        }
        // check rom directory path for art
        if ( !t )
        {
            t = imageBuild.CreateImage( item->filepath, page, imageType_, baseViewInfo.Monitor );
        }

    }

    if ( !t )
    {
        t = new Text(item->title, page, fontInst_, baseViewInfo.Monitor );
    }

    if ( t )
    {
        components_.at( index ) = t;
    }

    return true;
}


void ScrollingList::deallocateTexture( unsigned int index )
{
    if ( components_.size(  ) <= index ) return;

    Component *s = components_.at( index );

    if ( s )
        s->freeGraphicsMemory(  );
}

void ScrollingList::draw(  )
{
    //todo: Poor design implementation.
    // caller should instead call ScrollingList::Draw( unsigned int layer )
}


void ScrollingList::draw( unsigned int layer )
{
    
    if ( components_.size(  ) == 0 ) return;

    for ( unsigned int i = 0; i < components_.size(  ); ++i )
    {
        Component *c = components_.at( i );
        if ( c && c->baseViewInfo.Layer == layer ) c->draw(  );
    }
}


bool ScrollingList::isIdle(  )
{
    if ( !Component::isIdle(  ) ) return false;

    for ( unsigned int i = 0; i < components_.size(  ); ++i )
    {
        Component *c = components_.at( i );
        if ( c && !c->isIdle(  ) ) return false;
    }

    return true;
}


bool ScrollingList::isAttractIdle(  )
{
    if ( !Component::isAttractIdle(  ) ) return false;

    for ( unsigned int i = 0; i < components_.size(  ); ++i )
    {
        Component *c = components_.at( i );
        if ( c && !c->isAttractIdle(  ) ) return false;
    }

    return true;
}


void ScrollingList::resetScrollPeriod(  )
{
    scrollPeriod_ = startScrollTime_;
    return;
}


void ScrollingList::updateScrollPeriod(  )
{
    scrollPeriod_ -= scrollAcceleration_;
    if ( scrollPeriod_ < minScrollTime_ )
    {
        scrollPeriod_ = minScrollTime_;
    }
}


void ScrollingList::scroll( bool forward )
{
    // playlist menus don't scroll
    if (playlistType_)
        return;

    if ( !items_ || items_->size(  ) == 0 ) return;
    if ( !scrollPoints_ || scrollPoints_->size(  ) == 0 ) return;

    if ( scrollPeriod_ < minScrollTime_ )
    {
        scrollPeriod_ = minScrollTime_;
    }

    // Replace the item that's scrolled out
    if ( forward )
    {
        Item *i    = items_->at( loopIncrement( itemIndex_, scrollPoints_->size(  ), items_->size(  ) ) );
        itemIndex_ = loopIncrement( itemIndex_, 1, items_->size(  ) );
        deallocateTexture( 0 );
        allocateTexture( 0, i );
    }
    else
    {
        Item *i    = items_->at( loopDecrement( itemIndex_, 1, items_->size(  ) ) );
        itemIndex_ = loopDecrement( itemIndex_, 1, items_->size(  ) );
        deallocateTexture( loopDecrement( 0, 1, components_.size(  ) ) );
        allocateTexture( loopDecrement( 0, 1, components_.size(  ) ), i );
    }

    // Set the animations
    for ( unsigned int i = 0; i < scrollPoints_->size(  ); i++ )
    {
        unsigned int nextI;
        if ( forward )
        {
            nextI = loopDecrement( i, 1, scrollPoints_->size(  ) );
        }
        else
        {
            nextI = loopIncrement( i, 1, scrollPoints_->size(  ) );
        }

        Component *c = components_.at( i );
        c->allocateGraphicsMemory( );

        resetTweens( c, tweenPoints_->at( nextI ), scrollPoints_->at( i ), scrollPoints_->at( nextI ), scrollPeriod_ );
        c->baseViewInfo.font = scrollPoints_->at( nextI )->font; // Use the font settings of the next index
        c->triggerEvent(  "menuScroll" );
    }

    // Reorder the components
    Component *c = components_.at( 0 );
    if ( forward )
    {
        for ( unsigned int i = scrollPoints_->size(  ); i > 0; i-- )
        {
            unsigned int prevI = loopDecrement( i, 1, scrollPoints_->size(  ) );
            Component *store   = components_.at( prevI );
            components_[prevI] = c;
            c                  = store;
            
        }
    }
    else
    {
        for ( unsigned int i = 0; i < scrollPoints_->size(  ); i++ )
        {
            unsigned int nextI = loopIncrement( i, 1, scrollPoints_->size(  ) );
            Component *store   = components_.at( nextI );
            components_[nextI] = c;
            c                  = store;
            
        }
    }

    return;
}
