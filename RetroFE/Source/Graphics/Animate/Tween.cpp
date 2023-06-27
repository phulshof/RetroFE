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
#include "Tween.h"
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

std::map<std::string, TweenAlgorithm> Tween::tweenTypeMap_;
std::map<std::string, TweenProperty> Tween::tweenPropertyMap_;

Tween::Tween(TweenProperty property, TweenAlgorithm type, double start, double end, double duration, std::string playlistFilter)
    : property(property)
    , duration(duration)
    , startDefined(true)
    , type(type)
    , start(start)
    , end(end)
    , playlistFilter(playlistFilter)
{
}


bool Tween::getTweenProperty(std::string name, TweenProperty &property)
{
    bool retVal = false;

    if(tweenPropertyMap_.size() == 0)
    {
        tweenPropertyMap_["x"]               = TWEEN_PROPERTY_X;
        tweenPropertyMap_["y"]               = TWEEN_PROPERTY_Y;
        tweenPropertyMap_["angle"]           = TWEEN_PROPERTY_ANGLE;
        tweenPropertyMap_["alpha"]           = TWEEN_PROPERTY_ALPHA;
        tweenPropertyMap_["width"]           = TWEEN_PROPERTY_WIDTH;
        tweenPropertyMap_["height"]          = TWEEN_PROPERTY_HEIGHT;
        tweenPropertyMap_["xorigin"]         = TWEEN_PROPERTY_X_ORIGIN;
        tweenPropertyMap_["yorigin"]         = TWEEN_PROPERTY_Y_ORIGIN;
        tweenPropertyMap_["xoffset"]         = TWEEN_PROPERTY_X_OFFSET;
        tweenPropertyMap_["yoffset"]         = TWEEN_PROPERTY_Y_OFFSET;
        tweenPropertyMap_["fontSize"]        = TWEEN_PROPERTY_FONT_SIZE;
        tweenPropertyMap_["backgroundalpha"] = TWEEN_PROPERTY_BACKGROUND_ALPHA;
        tweenPropertyMap_["maxwidth"]        = TWEEN_PROPERTY_MAX_WIDTH;
        tweenPropertyMap_["maxheight"]       = TWEEN_PROPERTY_MAX_HEIGHT;
        tweenPropertyMap_["layer"]           = TWEEN_PROPERTY_LAYER;
        tweenPropertyMap_["containerx"]      = TWEEN_PROPERTY_CONTAINER_X;
        tweenPropertyMap_["containery"]      = TWEEN_PROPERTY_CONTAINER_Y;
        tweenPropertyMap_["containerwidth"]  = TWEEN_PROPERTY_CONTAINER_WIDTH;
        tweenPropertyMap_["containerheight"] = TWEEN_PROPERTY_CONTAINER_HEIGHT;
        tweenPropertyMap_["volume"]          = TWEEN_PROPERTY_VOLUME;
        tweenPropertyMap_["nop"]             = TWEEN_PROPERTY_NOP;
        tweenPropertyMap_["restart"] = TWEEN_PROPERTY_RESTART;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if(tweenPropertyMap_.find(name) != tweenPropertyMap_.end())
    {
        property = tweenPropertyMap_[name];
        retVal = true;
    }

    return retVal;
}


TweenAlgorithm Tween::getTweenType(std::string name)
{
    if(tweenTypeMap_.size() == 0)
    {
        tweenTypeMap_["easeinquadratic"] = EASE_IN_QUADRATIC;
        tweenTypeMap_["easeoutquadratic"] = EASE_OUT_QUADRATIC;
        tweenTypeMap_["easeinoutquadratic"] = EASE_INOUT_QUADRATIC;
        tweenTypeMap_["easeincubic"] = EASE_IN_CUBIC;
        tweenTypeMap_["easeoutcubic"] = EASE_OUT_CUBIC;
        tweenTypeMap_["easeinoutcubic"] = EASE_INOUT_CUBIC;
        tweenTypeMap_["easeinquartic"] = EASE_IN_QUARTIC;
        tweenTypeMap_["easeoutquartic"] = EASE_OUT_QUARTIC;
        tweenTypeMap_["easeinoutquartic"] = EASE_INOUT_QUARTIC;
        tweenTypeMap_["easeinquintic"] = EASE_IN_QUINTIC;
        tweenTypeMap_["easeoutquintic"] = EASE_OUT_QUINTIC;
        tweenTypeMap_["easeonoutquintic"] = EASE_INOUT_QUINTIC;
        tweenTypeMap_["easeinsine"] = EASE_IN_SINE;
        tweenTypeMap_["easeoutsine"] = EASE_OUT_SINE;
        tweenTypeMap_["easeinoutsine"] = EASE_INOUT_SINE;
        tweenTypeMap_["easeinexponential"] = EASE_IN_EXPONENTIAL;
        tweenTypeMap_["easeoutexponential"] = EASE_OUT_EXPONENTIAL;
        tweenTypeMap_["easeinoutexponential"] = EASE_INOUT_EXPONENTIAL;
        tweenTypeMap_["easeincircular"] = EASE_IN_CIRCULAR;
        tweenTypeMap_["easeoutcircular"] = EASE_OUT_CIRCULAR;
        tweenTypeMap_["easeinoutcircular"] = EASE_INOUT_CIRCULAR;
        tweenTypeMap_["linear"] = LINEAR;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if(tweenTypeMap_.find(name) != tweenTypeMap_.end())
    {
        return tweenTypeMap_[name];
    }
    else
    {
        return tweenTypeMap_["linear"];
    }
}


float Tween::animate(double elapsedTime)
{
    return animateSingle(type, start, end, duration, elapsedTime);
}

float Tween::animate(double elapsedTime, double startValue)
{
    return animateSingle(type, startValue, end, duration, elapsedTime);
}

//todo: SDL likes floats, consider having casting being performed elsewhere
float Tween::animateSingle(TweenAlgorithm type, double start, double end, double duration, double elapsedTime)
{
    double a = start;
    double b = end - start;
    double result = 0;

    switch(type)
    {
    case EASE_IN_QUADRATIC:
        result = easeInQuadratic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_QUADRATIC:
        result = easeOutQuadratic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_QUADRATIC:
        result = easeInOutQuadratic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_CUBIC:
        result = easeInCubic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_CUBIC:
        result = easeOutCubic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_CUBIC:
        result = easeInOutCubic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_QUARTIC:
        result = easeInQuartic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_QUARTIC:
        result = easeOutQuartic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_QUARTIC:
        result = easeInOutQuartic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_QUINTIC:
        result = easeInQuintic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_QUINTIC:
        result = easeOutQuintic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_QUINTIC:
        result = easeInOutQuintic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_SINE:
        result = easeInSine(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_SINE:
        result = easeOutSine(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_SINE:
        result = easeInOutSine(elapsedTime, duration, a, b);
        break;

    case EASE_IN_EXPONENTIAL:
        result = easeInExponential(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_EXPONENTIAL:
        result = easeOutExponential(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_EXPONENTIAL:
        result = easeInOutExponential(elapsedTime, duration, a, b);
        break;

    case EASE_IN_CIRCULAR:
        result = easeInCircular(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_CIRCULAR:
        result = easeOutCircular(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_CIRCULAR:
        result = easeInOutCircular(elapsedTime, duration, a, b);
        break;

    case LINEAR:
    default:
        result = linear(elapsedTime, duration, a, b);
        break;
    }

    return static_cast<float>(result);

}

double Tween::linear(double t, double d, double b, double c)
{
    if(d == 0) return b;
    return c*t/d + b;
};

double Tween::easeInQuadratic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t + b;
};

double Tween::easeOutQuadratic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return -c * t*(t-2) + b;
};

double Tween::easeInOutQuadratic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t + b;
    t--;
    return -c/2 * (t*(t-2) - 1) + b;
};

double Tween::easeInCubic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t*t + b;
};

double Tween::easeOutCubic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    t--;
    return c*(t*t*t + 1) + b;
};

double Tween::easeInOutCubic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t*t + b;
    t -= 2;
    return c/2*(t*t*t + 2) + b;
};

double Tween::easeInQuartic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t*t*t + b;
};

double Tween::easeOutQuartic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    t--;
    return -c * (t*t*t*t - 1) + b;
};

double Tween::easeInOutQuartic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t*t*t + b;
    t -= 2;
    return -c/2 * (t*t*t*t - 2) + b;
};

double Tween::easeInQuintic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t*t*t*t + b;
};


double Tween::easeOutQuintic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    t--;
    return c*(t*t*t*t*t + 1) + b;
};

double Tween::easeInOutQuintic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t*t*t*t + b;
    t -= 2;
    return c/2*(t*t*t*t*t + 2) + b;
};

double Tween::easeInSine(double t, double d, double b, double c)
{
    return -c * cos(t/d * (M_PI/2)) + c + b;
};

double Tween::easeOutSine(double t, double d, double b, double c)
{
    return c * sin(t/d * (M_PI/2)) + b;
};

double Tween::easeInOutSine(double t, double d, double b, double c)
{
    return -c/2 * (cos( M_PI*t/d) - 1) + b;
};

double Tween::easeInExponential(double t, double d, double b, double c)
{
    return c * pow( 2, 10 * (t/d - 1) ) + b;
};

double Tween::easeOutExponential(double t, double d, double b, double c)
{
    return c * ( - pow( 2, -10 * t/d ) + 1 ) + b;
};

double Tween::easeInOutExponential(double t, double d, double b, double c)
{
    t /= d/2;
    if (t < 1) return c/2 * pow( 2, 10 * (t - 1) ) + b;
    t--;
    return c/2 * ( -1* pow( 2, -10 * t) + 2 ) + b;
};

double Tween::easeInCircular(double t, double d, double b, double c)
{
    t /= d;
    return -c * (sqrt(1 - t*t) - 1) + b;
};


double Tween::easeOutCircular(double t, double d, double b, double c)
{
    t /= d;
    t--;
    return c * sqrt(1 - t*t) + b;
};

double Tween::easeInOutCircular(double t, double d, double b, double c)
{
    t /= d/2;
    if (t < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b;
    t -= 2;
    return c/2 * (sqrt(1 - t*t) + 1) + b;
}
