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

enum TweenAlgorithm
{
    LINEAR,
    EASE_IN_QUADRATIC,
    EASE_OUT_QUADRATIC,
    EASE_INOUT_QUADRATIC,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_INOUT_CUBIC,
    EASE_IN_QUARTIC,
    EASE_OUT_QUARTIC,
    EASE_INOUT_QUARTIC,
    EASE_IN_QUINTIC,
    EASE_OUT_QUINTIC,
    EASE_INOUT_QUINTIC,
    EASE_IN_SINE,
    EASE_OUT_SINE,
    EASE_INOUT_SINE,
    EASE_IN_EXPONENTIAL,
    EASE_OUT_EXPONENTIAL,
    EASE_INOUT_EXPONENTIAL,
    EASE_IN_CIRCULAR,
    EASE_OUT_CIRCULAR,
    EASE_INOUT_CIRCULAR,
};

enum TweenProperty
{
    TWEEN_PROPERTY_HEIGHT,
    TWEEN_PROPERTY_WIDTH,
    TWEEN_PROPERTY_ANGLE,
    TWEEN_PROPERTY_ALPHA,
    TWEEN_PROPERTY_X,
    TWEEN_PROPERTY_Y,
    TWEEN_PROPERTY_X_ORIGIN,
    TWEEN_PROPERTY_Y_ORIGIN,
    TWEEN_PROPERTY_X_OFFSET,
    TWEEN_PROPERTY_Y_OFFSET,
    TWEEN_PROPERTY_FONT_SIZE,
    TWEEN_PROPERTY_BACKGROUND_ALPHA,
    TWEEN_PROPERTY_MAX_WIDTH,
    TWEEN_PROPERTY_MAX_HEIGHT,
    TWEEN_PROPERTY_LAYER,
    TWEEN_PROPERTY_CONTAINER_X,
    TWEEN_PROPERTY_CONTAINER_Y,
    TWEEN_PROPERTY_CONTAINER_WIDTH,
    TWEEN_PROPERTY_CONTAINER_HEIGHT,
    TWEEN_PROPERTY_VOLUME,
    TWEEN_PROPERTY_NOP,
};
