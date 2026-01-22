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

#include "IVideo.h"
#include <vlc/vlc.h>
#include <mutex>

class VLCVideo : public IVideo
{
public:
    VLCVideo(int monitor);
    ~VLCVideo();
    bool initialize();
    bool play(std::string file);
    bool stop();
    bool deInitialize();
    SDL_Texture *getTexture() const;
    void update(float dt);
    void draw();
    void setNumLoops(int n);
    int getHeight();
    int getWidth();
    bool isPlaying();
    void setVolume(float volume);
    void skipForward();
    void skipBackward();
    void skipForwardp();
    void skipBackwardp();
    void pause();
    void restart();
    unsigned long long getCurrent();
    unsigned long long getDuration();
    bool isPaused();

private:
    // libVLC video callbacks
    static void* lockCallback(void* opaque, void** planes);
    static void unlockCallback(void* opaque, void* picture, void* const* planes);
    static void displayCallback(void* opaque, void* picture);

    // libVLC event callback
    static void eventCallback(const struct libvlc_event_t* event, void* opaque);

    // libVLC objects
    static libvlc_instance_t* vlcInstance_;  // Shared across all instances
    libvlc_media_player_t* mediaPlayer_;
    libvlc_media_t* media_;

    // Video buffer and texture
    SDL_Texture* texture_;
    unsigned char* videoBuffer_;
    std::mutex bufferMutex_;
    bool frameReady_;

    // Video properties
    int width_;
    int height_;
    int pitch_;

    // Playback state
    bool isPlaying_;
    bool paused_;
    int playCount_;
    int numLoops_;
    std::string currentFile_;

    // Audio
    float volume_;

    // Monitor
    int monitor_;

    // Initialization state
    static bool initialized_;
};
