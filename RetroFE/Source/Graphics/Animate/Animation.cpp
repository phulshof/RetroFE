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

#include "Animation.h"
#include <string>

Animation::Animation()
{
}

Animation::Animation(Animation &copy)
{
    for(std::vector<TweenSet *>::iterator it = copy.animationVector_.begin(); it != copy.animationVector_.end(); it++)
    {
        Push(new TweenSet(**it));
    }
}
Animation::~Animation()
{
    Clear();
}

void Animation::Push(TweenSet *set)
{
    animationVector_.push_back(set);
}

void Animation::Clear()
{
    std::vector<TweenSet *>::iterator it = animationVector_.begin();
    while(it != animationVector_.end())
    {
        delete *it;
        animationVector_.erase(it);
        it = animationVector_.begin();
    }

    animationVector_.clear();
}

std::vector<TweenSet *> *Animation::tweenSets()
{
    return &animationVector_;
}

TweenSet *Animation::tweenSet(unsigned int index)
{
    return animationVector_[index];
}


unsigned int Animation::size()
{
    return animationVector_.size();
}
