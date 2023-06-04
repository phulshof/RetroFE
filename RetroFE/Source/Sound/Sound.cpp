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

#include "Sound.h"

#include "../Utility/Log.h"

Sound::Sound(std::string file, std::string altfile)
    : file_(file)
    , chunk_(NULL)
    , channel_(-1)
{
    if(file_ != "" && !allocate())
    {
        file_ = altfile;
        if (file_ != "" && !allocate())
        {
            Logger::write(Logger::ZONE_WARNING, "Sound", "Cannot load " + file_);
        }
    }
}

Sound::~Sound()
{
    if(chunk_)
    {
        Mix_FreeChunk(chunk_);
        chunk_ = NULL;
    }
}

void Sound::play()
{
    if(chunk_)
    {
        channel_ = Mix_PlayChannel(-1, chunk_, 0);
    }
}

bool Sound::free()
{
    if(chunk_)
    {
        Mix_FreeChunk(chunk_);
        chunk_   = NULL;
        channel_ = -1;
    }

    return true;
}

bool Sound::allocate()
{
    if(!chunk_)
    {
        chunk_ = Mix_LoadWAV(file_.c_str());
    }

    return (chunk_ != NULL);
}


bool Sound::isPlaying()
{
    return (channel_ != -1) && Mix_Playing(channel_);
}
