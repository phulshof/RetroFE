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


#include "Text.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../Font.h"
#include <sstream>


Text::Text( std::string text, Page &p, Font *font, int monitor )
    : Component(p)
    , textData_(text)
    , fontInst_(font)
{
    allocateGraphicsMemory( );
    baseViewInfo.Monitor = monitor;
}

Text::~Text( )
{
    freeGraphicsMemory( );
}

void Text::freeGraphicsMemory( )
{
    Component::freeGraphicsMemory( );
}

void Text::allocateGraphicsMemory( )
{
    Component::allocateGraphicsMemory( );
}

void Text::deInitializeFonts( )
{
    fontInst_->deInitialize( );
}

void Text::initializeFonts( )
{
    fontInst_->initialize( );
}

void Text::setText( std::string text, int id )
{
    if ( getId( ) == id )
        textData_ = text;
}

void Text::draw( )
{
    Component::draw( );

    Font *font;
    if ( baseViewInfo.font ) // Use font of this specific item if available
      font = baseViewInfo.font;
    else                     // If not, use the general font settings
      font = fontInst_;

    SDL_Texture *t = font->getTexture( );

    float imageHeight = 0;
    float imageWidth = 0;
    float imageMaxWidth = 0;
    if (baseViewInfo.Width < baseViewInfo.MaxWidth && baseViewInfo.Width > 0)
    {
        imageMaxWidth = baseViewInfo.Width;
    }
    else
    {
        imageMaxWidth = baseViewInfo.MaxWidth;
    }

    imageHeight = (float)font->getHeight( );
    float scale = (float)baseViewInfo.FontSize / (float)imageHeight;

    unsigned int textIndexMax = 0;

    // determine image width
    for ( unsigned int i = 0; i < textData_.size( ); ++i )
    {
        Font::GlyphInfo glyph;
        if ( font->getRect( textData_[i], glyph ) )
        {
            if ( glyph.minX < 0 )
            {
                imageWidth += glyph.minX;
            }

            if ( (imageWidth + glyph.advance)*scale > imageMaxWidth )
            {
                break;
            }

            textIndexMax = i;
            imageWidth  += glyph.advance;
        }

    }

    float oldWidth       = baseViewInfo.Width;
    float oldHeight      = baseViewInfo.Height;
    float oldImageWidth  = baseViewInfo.ImageHeight;
    float oldImageHeight = baseViewInfo.ImageWidth;

    baseViewInfo.Width       = imageWidth*scale;
    baseViewInfo.Height      = baseViewInfo.FontSize;
    baseViewInfo.ImageWidth  = imageWidth;
    baseViewInfo.ImageHeight = imageHeight;

    float xOrigin = baseViewInfo.XRelativeToOrigin( );
    float yOrigin = baseViewInfo.YRelativeToOrigin( );

    baseViewInfo.Width       = oldWidth;
    baseViewInfo.Height      = oldHeight;
    baseViewInfo.ImageWidth  = oldImageWidth;
    baseViewInfo.ImageHeight = oldImageHeight;


    SDL_Rect rect;
    rect.x = static_cast<int>( xOrigin );

    for ( unsigned int i = 0; i <= textIndexMax; ++i )
    {
        Font::GlyphInfo glyph;

        if ( font->getRect(textData_[i], glyph) && glyph.rect.h > 0 )
        {
            SDL_Rect charRect = glyph.rect;
            float h = static_cast<float>( charRect.h * scale );
            float w = static_cast<float>( charRect.w * scale );
            rect.h = static_cast<int>( h );
            rect.w = static_cast<int>( w );
            rect.y = static_cast<int>( yOrigin );

            if(glyph.minX < 0)
            {
                rect.x += static_cast<int>( (float)(glyph.minX) * scale );
            }
            if ( font->getAscent( ) < glyph.maxY )
            {
                rect.y += static_cast<int>( (font->getAscent( ) - glyph.maxY)*scale );
            }


            SDL::renderCopy( t, baseViewInfo.Alpha, &charRect, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor) );

            rect.x += static_cast<int>( glyph.advance * scale );

        }
    }
}
