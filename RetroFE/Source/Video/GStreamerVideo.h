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

extern "C"
{
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
}


class GStreamerVideo : public IVideo
{
public:
    GStreamerVideo( int monitor );
    ~GStreamerVideo();
    bool initialize();
    bool play(std::string file);
    bool stop();
    bool deInitialize();
    SDL_Texture *getTexture() const;
    void update(float dt);
    void volumeUpdate();
    void draw();
    void setNumLoops(int n);
    void freeElements();
    int getHeight();
    int getWidth();
    bool isPlaying();
    void setVolume(float volume);
    void skipForward( );
    void skipBackward( );
    void skipForwardp( );
    void skipBackwardp( );
    void pause( );
    void restart( );
    unsigned long long getCurrent( );
    unsigned long long getDuration( );
    bool isPaused( );

private:
        
    enum BufferLayout {
        UNKNOWN,        // Initial state
        CONTIGUOUS,     // Contiguous buffer layout
        NON_CONTIGUOUS  // Non-contiguous buffer layout
    };
    
    BufferLayout bufferLayout_ = UNKNOWN;
    
    static void processNewBuffer (GstElement *fakesink, GstBuffer *buf, GstPad *pad, gpointer data);
    static void elementSetupCallback(GstElement *playbin, GstElement *element, GStreamerVideo *video);
    static gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer data);
    bool initializeGstElements(std::string file);
    bool createAndLinkGstElements();
    void onEndOfStream();
    GstElement *playbin_;
    GstElement *videoBin_;
    GstElement *videoSink_;
    GstElement *videoConvert_;
    GstElement *capsFilter_;
    GstCaps *videoConvertCaps_;
    GstBus *videoBus_;
    SDL_Texture* texture_;
    gulong elementSetupHandlerId_;
    gulong handoffHandlerId_;
    gint height_;
    gint width_;
    GstBuffer *videoBuffer_;
    bool frameReady_;
    bool isPlaying_;
    static bool initialized_;
    int playCount_;
    std::string currentFile_;
    int numLoops_;
    float volume_;
    double currentVolume_;
    int monitor_;
    bool paused_;
    double lastSetVolume_;
    bool lastSetMuteState_;
    guint busWatchId_;
};