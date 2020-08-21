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
#include "ComponentItemBindingBuilder.h"
#include "ComponentItemBinding.h"
#include "../Collection/Item.h"
#include "../Utility/Log.h"

ComponentItemBindingBuilder::ComponentItemBindingBuilder()
{
}

ComponentItemBindingBuilder::~ComponentItemBindingBuilder()
{
}

std::vector<ComponentItemBinding *> *ComponentItemBindingBuilder::buildCollectionItems(std::vector<Item *> *infoList)
{
    std::vector<ComponentItemBinding *> *sprites = new std::vector<ComponentItemBinding *>();
    std::vector<Item *>::iterator it;

    for(it = infoList->begin(); it != infoList->end(); ++it)
    {
        ComponentItemBinding *s = new ComponentItemBinding(*it);
        sprites->push_back(s);
    }

    return sprites;
}
