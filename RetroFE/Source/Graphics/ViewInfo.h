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

#include "Animate/TweenTypes.h"
#include <string>
#include <map>

class Font;

class ViewInfo
{
public:

    ViewInfo();
    virtual ~ViewInfo();

    float XRelativeToOrigin() const;
    float YRelativeToOrigin() const;

    float ScaledHeight() const;
    float ScaledWidth() const;

    static const int AlignCenter = -1;
    static const int AlignLeft = -2;
    static const int AlignTop = -3;
    static const int AlignRight = -4;
    static const int AlignBottom = -5;

    float        X;
    float        Y;
    float        XOrigin;
    float        YOrigin;
    float        XOffset;
    float        YOffset;
    float        Width;
    float        MinWidth;
    float        MaxWidth;
    float        Height;
    float        MinHeight;
    float        MaxHeight;
    float        ImageWidth;
    float        ImageHeight;
    float        FontSize;
    Font        *font;
    float        Angle;
    float        Alpha;
    unsigned int Layer;
    float        BackgroundRed;
    float        BackgroundGreen;
    float        BackgroundBlue;
    float        BackgroundAlpha;
    std::string  Reflection;
    unsigned int ReflectionDistance;
    float        ReflectionScale;
    float        ReflectionAlpha;
    float        ContainerX;
    float        ContainerY;
    float        ContainerWidth;
    float        ContainerHeight;
    int          Monitor;
    float        Volume;

private:
    float AbsoluteHeight() const;
    float AbsoluteWidth() const;
};
