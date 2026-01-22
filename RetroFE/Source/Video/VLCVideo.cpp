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
#include "VLCVideo.h"
#include "../Database/Configuration.h"
#include "../Utility/Log.h"
#include "../SDL.h"
#include <cstring>

bool VLCVideo::initialized_ = false;
libvlc_instance_t* VLCVideo::vlcInstance_ = nullptr;

VLCVideo::VLCVideo(int monitor)
    : mediaPlayer_(nullptr)
    , media_(nullptr)
    , texture_(nullptr)
    , videoBuffer_(nullptr)
    , frameReady_(false)
    , width_(0)
    , height_(0)
    , pitch_(0)
    , isPlaying_(false)
    , paused_(false)
    , playCount_(0)
    , numLoops_(0)
    , volume_(1.0f)
    , monitor_(monitor)
{
}

VLCVideo::~VLCVideo()
{
    stop();
    deInitialize();
}

void VLCVideo::setNumLoops(int n)
{
    if (n > 0)
        numLoops_ = n;
}

SDL_Texture* VLCVideo::getTexture() const
{
    return texture_;
}

bool VLCVideo::initialize()
{
    if (!initialized_)
    {
        // Initialize libVLC with optimized arguments
        const char* const vlcArgs[] = {
            "--no-xlib",          // Don't use X11 on Linux
            "--quiet",            // Suppress console output
            "--audio-desync=100", // Audio desync compensation
            "--no-video-title-show", // Don't show video title
            "--avcodec-hw=none"   // Disable hardware acceleration (can cause issues)
        };

        vlcInstance_ = libvlc_new(sizeof(vlcArgs) / sizeof(vlcArgs[0]), vlcArgs);

        if (!vlcInstance_)
        {
            Logger::write(Logger::ZONE_ERROR, "Video", "Failed to initialize libVLC");
            return false;
        }

        initialized_ = true;
        Logger::write(Logger::ZONE_INFO, "Video", "libVLC initialized successfully");
    }

    return true;
}

bool VLCVideo::deInitialize()
{
    if (mediaPlayer_)
    {
        libvlc_media_player_release(mediaPlayer_);
        mediaPlayer_ = nullptr;
    }

    // Don't release the shared vlcInstance_ here - it should persist
    // across all video instances for the lifetime of the application

    if (videoBuffer_)
    {
        delete[] videoBuffer_;
        videoBuffer_ = nullptr;
    }

    if (texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }

    return true;
}

// Video callback implementations
void* VLCVideo::lockCallback(void* opaque, void** planes)
{
    VLCVideo* video = static_cast<VLCVideo*>(opaque);
    video->bufferMutex_.lock();
    *planes = video->videoBuffer_;
    return nullptr;
}

void VLCVideo::unlockCallback(void* opaque, void* picture, void* const* planes)
{
    VLCVideo* video = static_cast<VLCVideo*>(opaque);
    video->frameReady_ = true;
    video->bufferMutex_.unlock();
}

void VLCVideo::displayCallback(void* opaque, void* picture)
{
    // We handle display in update() method
}

void VLCVideo::eventCallback(const struct libvlc_event_t* event, void* opaque)
{
    VLCVideo* video = static_cast<VLCVideo*>(opaque);

    switch (event->type)
    {
        case libvlc_MediaPlayerEndReached:
            video->playCount_++;
            if (video->numLoops_ == 0 || video->playCount_ < video->numLoops_)
            {
                // Loop the video
                libvlc_media_player_set_position(video->mediaPlayer_, 0.0f);
                libvlc_media_player_play(video->mediaPlayer_);
            }
            else
            {
                video->isPlaying_ = false;
            }
            break;

        case libvlc_MediaPlayerStopped:
            video->isPlaying_ = false;
            break;

        default:
            break;
    }
}

bool VLCVideo::play(std::string file)
{
    if (!vlcInstance_)
    {
        Logger::write(Logger::ZONE_ERROR, "Video", "libVLC not initialized");
        return false;
    }

    // Stop any currently playing media
    if (isPlaying_)
    {
        stop();
    }

    // Create media from file
    media_ = libvlc_media_new_path(vlcInstance_, file.c_str());
    if (!media_)
    {
        Logger::write(Logger::ZONE_ERROR, "Video", "Failed to create media from file: " + file);
        return false;
    }

    // Create media player if it doesn't exist
    if (!mediaPlayer_)
    {
        mediaPlayer_ = libvlc_media_player_new_from_media(media_);
        if (!mediaPlayer_)
        {
            Logger::write(Logger::ZONE_ERROR, "Video", "Failed to create media player");
            libvlc_media_release(media_);
            media_ = nullptr;
            return false;
        }

        // Attach event manager
        libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(mediaPlayer_);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, eventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, eventCallback, this);
    }
    else
    {
        libvlc_media_player_set_media(mediaPlayer_, media_);
    }

    // Parse media to get video dimensions
    libvlc_media_parse(media_);

    // Get video track info
    libvlc_media_track_t** tracks = nullptr;
    unsigned int trackCount = libvlc_media_tracks_get(media_, &tracks);

    for (unsigned int i = 0; i < trackCount; i++)
    {
        if (tracks[i]->i_type == libvlc_track_video)
        {
            width_ = tracks[i]->video->i_width;
            height_ = tracks[i]->video->i_height;
            break;
        }
    }

    libvlc_media_tracks_release(tracks, trackCount);

    // Use default dimensions if we couldn't get them from media
    if (width_ == 0 || height_ == 0)
    {
        width_ = 1920;
        height_ = 1080;
        Logger::write(Logger::ZONE_WARNING, "Video", "Could not determine video dimensions, using defaults");
    }

    // Calculate pitch (bytes per row) for RGBA format
    pitch_ = width_ * 4;

    // Allocate video buffer
    if (videoBuffer_)
    {
        delete[] videoBuffer_;
    }
    videoBuffer_ = new unsigned char[pitch_ * height_];
    memset(videoBuffer_, 0, pitch_ * height_);

    // Create SDL texture
    if (texture_)
    {
        SDL_DestroyTexture(texture_);
    }

    SDL_Renderer* renderer = SDL::getRenderer(monitor_);
    texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING, width_, height_);

    if (!texture_)
    {
        Logger::write(Logger::ZONE_ERROR, "Video", "Failed to create SDL texture");
        return false;
    }

    // Set blend mode for proper rendering
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);

    // Set video callbacks
    libvlc_video_set_callbacks(mediaPlayer_, lockCallback, unlockCallback, displayCallback, this);
    libvlc_video_set_format(mediaPlayer_, "RV32", width_, height_, pitch_);

    // Set volume
    libvlc_audio_set_volume(mediaPlayer_, static_cast<int>(volume_ * 100.0f));

    // Start playback
    int result = libvlc_media_player_play(mediaPlayer_);
    if (result == -1)
    {
        Logger::write(Logger::ZONE_ERROR, "Video", "Failed to start playback");
        return false;
    }

    currentFile_ = file;
    isPlaying_ = true;
    playCount_ = 0;
    paused_ = false;
    frameReady_ = false;

    Logger::write(Logger::ZONE_INFO, "Video", "Playing: " + file);
    return true;
}

bool VLCVideo::stop()
{
    if (mediaPlayer_)
    {
        libvlc_media_player_stop(mediaPlayer_);
        isPlaying_ = false;
        paused_ = false;
        frameReady_ = false;
    }

    if (media_)
    {
        libvlc_media_release(media_);
        media_ = nullptr;
    }

    return true;
}

void VLCVideo::update(float dt)
{
    if (!isPlaying_ || !mediaPlayer_ || !texture_)
        return;

    // Update texture if we have a new frame
    if (frameReady_)
    {
        bufferMutex_.lock();

        void* pixels;
        int pitch;
        if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch) == 0)
        {
            memcpy(pixels, videoBuffer_, pitch_ * height_);
            SDL_UnlockTexture(texture_);
        }

        frameReady_ = false;
        bufferMutex_.unlock();
    }
}

void VLCVideo::draw()
{
    // Drawing is handled by the component system via getTexture()
}

int VLCVideo::getHeight()
{
    return height_;
}

int VLCVideo::getWidth()
{
    return width_;
}

bool VLCVideo::isPlaying()
{
    return isPlaying_;
}

void VLCVideo::setVolume(float volume)
{
    volume_ = volume;
    if (mediaPlayer_)
    {
        libvlc_audio_set_volume(mediaPlayer_, static_cast<int>(volume * 100.0f));
    }
}

void VLCVideo::skipForward()
{
    if (!mediaPlayer_)
        return;

    libvlc_time_t current = libvlc_media_player_get_time(mediaPlayer_);
    libvlc_media_player_set_time(mediaPlayer_, current + 10000); // Skip 10 seconds
}

void VLCVideo::skipBackward()
{
    if (!mediaPlayer_)
        return;

    libvlc_time_t current = libvlc_media_player_get_time(mediaPlayer_);
    libvlc_time_t newTime = (current > 10000) ? (current - 10000) : 0;
    libvlc_media_player_set_time(mediaPlayer_, newTime);
}

void VLCVideo::skipForwardp()
{
    if (!mediaPlayer_)
        return;

    libvlc_time_t duration = libvlc_media_player_get_length(mediaPlayer_);
    libvlc_time_t current = libvlc_media_player_get_time(mediaPlayer_);
    libvlc_time_t skip = duration / 20; // 5% of duration
    libvlc_media_player_set_time(mediaPlayer_, current + skip);
}

void VLCVideo::skipBackwardp()
{
    if (!mediaPlayer_)
        return;

    libvlc_time_t duration = libvlc_media_player_get_length(mediaPlayer_);
    libvlc_time_t current = libvlc_media_player_get_time(mediaPlayer_);
    libvlc_time_t skip = duration / 20; // 5% of duration
    libvlc_time_t newTime = (current > skip) ? (current - skip) : 0;
    libvlc_media_player_set_time(mediaPlayer_, newTime);
}

void VLCVideo::pause()
{
    if (!mediaPlayer_)
        return;

    if (paused_)
    {
        libvlc_media_player_play(mediaPlayer_);
        paused_ = false;
    }
    else
    {
        libvlc_media_player_pause(mediaPlayer_);
        paused_ = true;
    }
}

void VLCVideo::restart()
{
    if (!mediaPlayer_)
        return;

    libvlc_media_player_set_position(mediaPlayer_, 0.0f);
    if (paused_)
    {
        libvlc_media_player_play(mediaPlayer_);
        paused_ = false;
    }
}

unsigned long long VLCVideo::getCurrent()
{
    if (!mediaPlayer_)
        return 0;

    return static_cast<unsigned long long>(libvlc_media_player_get_time(mediaPlayer_));
}

unsigned long long VLCVideo::getDuration()
{
    if (!mediaPlayer_)
        return 0;

    return static_cast<unsigned long long>(libvlc_media_player_get_length(mediaPlayer_));
}

bool VLCVideo::isPaused()
{
    return paused_;
}
