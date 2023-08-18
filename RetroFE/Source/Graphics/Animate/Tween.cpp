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
#include "../../Utility/Log.h"
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

std::map<std::string, TweenAlgorithm> Tween::tweenTypeMap_ = {
    {"easeinquadratic", EASE_IN_QUADRATIC},
    {"easeoutquadratic", EASE_OUT_QUADRATIC},
    {"easeinoutquadratic", EASE_INOUT_QUADRATIC},
    {"easeincubic", EASE_IN_CUBIC},
    {"easeoutcubic", EASE_OUT_CUBIC},
    {"easeinoutcubic", EASE_INOUT_CUBIC},
    {"easeinquartic", EASE_IN_QUARTIC},
    {"easeoutquartic", EASE_OUT_QUARTIC},
    {"easeinoutquartic", EASE_INOUT_QUARTIC},
    {"easeinquintic", EASE_IN_QUINTIC},
    {"easeoutquintic", EASE_OUT_QUINTIC},
    {"easeonoutquintic", EASE_INOUT_QUINTIC},
    {"easeinsine", EASE_IN_SINE},
    {"easeoutsine", EASE_OUT_SINE},
    {"easeinoutsine", EASE_INOUT_SINE},
    {"easeinexponential", EASE_IN_EXPONENTIAL},
    {"easeoutexponential", EASE_OUT_EXPONENTIAL},
    {"easeinoutexponential", EASE_INOUT_EXPONENTIAL},
    {"easeincircular", EASE_IN_CIRCULAR},
    {"easeoutcircular", EASE_OUT_CIRCULAR},
    {"easeinoutcircular", EASE_INOUT_CIRCULAR},
    {"linear", LINEAR}
};


std::map<std::string, TweenProperty> Tween::tweenPropertyMap_ = {
    {"x", TWEEN_PROPERTY_X},
    {"y", TWEEN_PROPERTY_Y},
    {"angle", TWEEN_PROPERTY_ANGLE},
    {"alpha", TWEEN_PROPERTY_ALPHA},
    {"width", TWEEN_PROPERTY_WIDTH},
    {"height", TWEEN_PROPERTY_HEIGHT},
    {"xorigin", TWEEN_PROPERTY_X_ORIGIN},
    {"yorigin", TWEEN_PROPERTY_Y_ORIGIN},
    {"xoffset", TWEEN_PROPERTY_X_OFFSET},
    {"yoffset", TWEEN_PROPERTY_Y_OFFSET},
    {"fontsize", TWEEN_PROPERTY_FONT_SIZE},
    {"backgroundalpha", TWEEN_PROPERTY_BACKGROUND_ALPHA},
    {"maxwidth", TWEEN_PROPERTY_MAX_WIDTH},
    {"maxheight", TWEEN_PROPERTY_MAX_HEIGHT},
    {"layer", TWEEN_PROPERTY_LAYER},
    {"containerx", TWEEN_PROPERTY_CONTAINER_X},
    {"containery", TWEEN_PROPERTY_CONTAINER_Y},
    {"containerwidth", TWEEN_PROPERTY_CONTAINER_WIDTH},
    {"containerheight", TWEEN_PROPERTY_CONTAINER_HEIGHT},
    {"volume", TWEEN_PROPERTY_VOLUME},
    {"nop", TWEEN_PROPERTY_NOP},
    {"restart", TWEEN_PROPERTY_RESTART}
};

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
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if(tweenPropertyMap_.find(name) != tweenPropertyMap_.end())
    {
        property = tweenPropertyMap_[name];
        return true;
    }

    return false;
}


TweenAlgorithm Tween::getTweenType(std::string name)
{
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
