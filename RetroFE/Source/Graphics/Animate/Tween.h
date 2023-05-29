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

#include "TweenTypes.h"
#include <string>
#include <map>

class ViewInfo;

class Tween
{
public:

    Tween(TweenProperty name, TweenAlgorithm type, double start, double end, double duration, std::string playlistFilter = "");
    float animate(double elapsedTime);
    float animate(double elapsedTime, double startValue);
    static float animateSingle(TweenAlgorithm type, double start, double end, double duration, double elapsedTime);
    static TweenAlgorithm getTweenType(std::string name);
    static bool getTweenProperty(std::string name, TweenProperty &property);
    TweenProperty property;
    double duration;
    bool   startDefined;
    std::string playlistFilter;

private:
    static double easeInQuadratic(double elapsedTime, double duration, double b, double c);
    static double easeOutQuadratic(double elapsedTime, double duration, double b, double c);
    static double easeInOutQuadratic(double elapsedTime, double duration, double b, double c);
    static double easeInCubic(double elapsedTime, double duration, double b, double c);
    static double easeOutCubic(double elapsedTime, double duration, double b, double c);
    static double easeInOutCubic(double elapsedTime, double duration, double b, double c);
    static double easeInQuartic(double elapsedTime, double duration, double b, double c);
    static double easeOutQuartic(double elapsedTime, double duration, double b, double c);
    static double easeInOutQuartic(double elapsedTime, double duration, double b, double c);
    static double easeInQuintic(double elapsedTime, double duration, double b, double c);
    static double easeOutQuintic(double elapsedTime, double duration, double b, double c);
    static double easeInOutQuintic(double elapsedTime, double duration, double b, double c);
    static double easeInSine(double elapsedTime, double duration, double b, double c);
    static double easeOutSine(double elapsedTime, double duration, double b, double c);
    static double easeInOutSine(double elapsedTime, double duration, double b, double c);
    static double easeInExponential(double elapsedTime, double duration, double b, double c);
    static double easeOutExponential(double elapsedTime, double duration, double b, double c);
    static double easeInOutExponential(double elapsedTime, double duration, double b, double c);
    static double easeInCircular(double elapsedTime, double duration, double b, double c);
    static double easeOutCircular(double elapsedTime, double duration, double b, double c);
    static double easeInOutCircular(double elapsedTime, double duration, double b, double c);
    static double linear(double elapsedTime, double duration, double b, double c);

    static std::map<std::string, TweenAlgorithm> tweenTypeMap_;
    static std::map<std::string, TweenProperty> tweenPropertyMap_;
    TweenAlgorithm type;
    double start;
    double end;
};
