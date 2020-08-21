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
#include "TweenSet.h"

TweenSet::TweenSet()
{
}

TweenSet::TweenSet(TweenSet &copy)
{
    for(std::vector<Tween *>::iterator it = copy.set_.begin(); it != copy.set_.end(); it++)
    {
        Tween *t = new Tween(**it);
        set_.push_back(t);
    }
}

TweenSet::~TweenSet()
{
    clear();
}

void TweenSet::push(Tween *tween)
{
    set_.push_back(tween);
}

void TweenSet::clear()
{
    std::vector<Tween *>::iterator it = set_.begin();
    while(it != set_.end())
    {
        delete *it;
        set_.erase(it);
        it = set_.begin();
    }
}
std::vector<Tween *> *TweenSet::tweens()
{
    return &set_;
}

Tween *TweenSet::getTween(unsigned int index)
{
    return set_[index];
}


unsigned int TweenSet::size()
{
    return set_.size();
}
