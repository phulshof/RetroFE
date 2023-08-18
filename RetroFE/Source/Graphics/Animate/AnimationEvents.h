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

#include "Tween.h"
#include "Animation.h"
#include <string>
#include <vector>
#include <map>

class AnimationEvents
{
public:
    AnimationEvents();
    AnimationEvents(AnimationEvents &copy);
    ~AnimationEvents();

    Animation *getAnimation(const std::string& tween);
    Animation *getAnimation(const std::string& tween, int index);
    void setAnimation(const std::string& tween, int index, Animation *animation);
    void clear();

private:
    std::map<std::string, std::map<int, Animation *> > animationMap_;
};
