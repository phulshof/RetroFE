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

#include "VideoBuilder.h"
#include "../../Utility/Utils.h"
#include "../../Video/VideoFactory.h"
#include <fstream>


VideoComponent * VideoBuilder::createVideo(std::string path, Page &page, std::string name, int monitor, bool isTypeVideo, int numLoops)
{
    VideoComponent *component = NULL;
    
    // Declare the extensions vector as static so it's only initialized once.
    static std::vector<std::string> extensions = {
        "mp4", "MP4", "avi", "AVI", "mkv", "MKV",
        "mp3", "MP3", "wav", "WAV", "flac", "FLAC"
    };

    std::string prefix = Utils::combinePath(path, name);
    std::string file;

    if(Utils::findMatchingFile(prefix, extensions, file))
    {
        IVideo *video = factory_.createVideo( monitor, isTypeVideo, numLoops );

        if(video)
        {
            component = new VideoComponent(video, page, file);
        }
    }

    return component;
}

