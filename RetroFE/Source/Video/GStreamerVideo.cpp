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
#include "GStreamerVideo.h"
#include "../Graphics/ViewInfo.h"
#include "../Graphics/Component/Image.h"
#include "../Database/Configuration.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include "../SDL.h"
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <gst/app/gstappsink.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>

bool GStreamerVideo::initialized_ = false;

GStreamerVideo::GStreamerVideo( int monitor )
    : playbin_(NULL)
    , videoBin_(NULL)
    , videoSink_(NULL)
    , videoConvert_(NULL)
    , capsFilter_(NULL)
    , videoConvertCaps_(NULL)
    , videoBus_(NULL)
    , texture_(NULL)
    , elementSetupHandlerId_(0)
    , handoffHandlerId_(0)
    , height_(0)
    , width_(0)
    , videoBuffer_(NULL)
    , frameReady_(false)
    , isPlaying_(false)
    , playCount_(0)
    , currentFile_("")
    , numLoops_(0)
    , volume_(0.0)
    , currentVolume_(0.0)
    , monitor_(monitor)
    , lastSetVolume_(0.0)
    , lastSetMuteState_(false)
    , busWatchId_(0)
    , bufferLayout_(UNKNOWN)
{
    paused_ = false;
}

GStreamerVideo::~GStreamerVideo()
{
    GStreamerVideo::stop();
}

void GStreamerVideo::setNumLoops(int n)
{
    if ( n > 0 )
        numLoops_ = n;
}

SDL_Texture *GStreamerVideo::getTexture() const
{
    return texture_;
}


bool GStreamerVideo::initialize()
{
    if(initialized_)
    {
        return true;
    }

    std::string path = Utils::combinePath(Configuration::absolutePath, "retrofe");
    gst_init(NULL, NULL);

#ifdef WIN32
    GstRegistry *registry = gst_registry_get();
    gst_registry_scan_path(registry, path.c_str());
#endif

    initialized_ = true;
    paused_      = false;

    return true;
}

bool GStreamerVideo::deInitialize()
{
    gst_deinit();
    initialized_ = false;
    paused_      = false;
    return true;
}


bool GStreamerVideo::stop()
{
    paused_ = false;

    if(!initialized_)
    {
        return false;
    }

    // Disable handoffs for videoSink
    if (videoSink_) 
    {
        g_object_set(G_OBJECT(videoSink_), "signal-handoffs", FALSE, NULL);
    }

    // Disconnect associated signals
    if (playbin_ && elementSetupHandlerId_) {
        g_signal_handler_disconnect(playbin_, elementSetupHandlerId_);
        elementSetupHandlerId_ = 0;
    }

    if (videoSink_ && handoffHandlerId_) {
        g_signal_handler_disconnect(videoSink_, handoffHandlerId_);
        handoffHandlerId_ = 0;
    }

    // Set playbin state to GST_STATE_NULL
    if(playbin_)
    {
        GstStateChangeReturn ret = gst_element_set_state(playbin_, GST_STATE_NULL);
        if (ret == GST_STATE_CHANGE_FAILURE) 
        {
            Logger::write(Logger::ZONE_ERROR, "Video", "Failed to set playbin to NULL state");
            return false;
        }

        ret = gst_element_get_state(playbin_, NULL, NULL, GST_CLOCK_TIME_NONE);
        if (ret == GST_STATE_CHANGE_FAILURE) 
        {
            Logger::write(Logger::ZONE_ERROR, "Video", "Failed to wait for playbin to reach NULL state");
            return false;
        }
    }

    // Release SDL Texture
    if(texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = NULL;
    }

    // Unref the video buffer
    if(videoBuffer_)
    {
        gst_buffer_unref(videoBuffer_);
        videoBuffer_ = NULL;
    }

    // Free GStreamer elements and related resources
    freeElements();

    isPlaying_ = false;
    height_ = 0;
    width_ = 0;
    frameReady_ = false;

    return true;
}


void GStreamerVideo::freeElements()
{
    // Remove the bus watch
    if(busWatchId_)
    {
        g_source_remove(busWatchId_);
        busWatchId_ = 0;
    }

    // Unref the video bus
    if(videoBus_)
    {
        gst_object_unref(videoBus_);
        videoBus_ = NULL;
    }

    // Unref the playbin
    if(playbin_)
    {
        gst_object_unref(playbin_);
        playbin_ = NULL;
    }

    // Unref caps associated with video conversion
    if(videoConvertCaps_)
    {
        gst_caps_unref(videoConvertCaps_);
        videoConvertCaps_ = NULL;
    }

    // Nullify video elements
    videoSink_    = NULL;
    videoConvert_ = NULL;
    videoBin_     = NULL;
}


bool GStreamerVideo::play(std::string file)
{
    playCount_ = 0;

    if(!initialized_)
        return false;

    currentFile_ = file;

    if(!initializeGstElements(file))
        return false;

    // Start playing
    GstStateChangeReturn playState = gst_element_set_state(GST_ELEMENT(playbin_), GST_STATE_PLAYING);
    if (playState != GST_STATE_CHANGE_ASYNC)
    {
        isPlaying_ = false;
        Logger::write(Logger::ZONE_ERROR, "Video", "Unable to set the pipeline to the playing state.");
        stop();
        return false;
    }

    isPlaying_ = true;

    // Set the volume to zero and mute the video
    gst_stream_volume_set_volume(GST_STREAM_VOLUME(playbin_), GST_STREAM_VOLUME_FORMAT_LINEAR, 0.0);
    gst_stream_volume_set_mute(GST_STREAM_VOLUME(playbin_), true);

    return true;
}

bool GStreamerVideo::initializeGstElements(std::string file)
{
    gchar *uriFile = gst_filename_to_uri(file.c_str(), NULL);

    if(!uriFile)
        return false;

    if(!playbin_)
    {
        if(!createAndLinkGstElements())
        {
            stop();
            return false;
        }
    }

    // Set properties of playbin and videoSink
    const guint PLAYBIN_FLAGS = 0x00000001 | 0x00000002 | 0x00000010;
    g_object_set(G_OBJECT(playbin_), "uri", uriFile, "video-sink", videoBin_, "instant-uri", TRUE, "flags", PLAYBIN_FLAGS, NULL);
    g_free(uriFile);
    elementSetupHandlerId_ = g_signal_connect(playbin_, "element-setup", G_CALLBACK(elementSetupCallback), this);
    videoBus_ = gst_pipeline_get_bus(GST_PIPELINE(playbin_));
    busWatchId_ = gst_bus_add_watch(videoBus_, busCallback, this);
    g_object_set(G_OBJECT(videoSink_), "signal-handoffs", TRUE, NULL);
    handoffHandlerId_ = g_signal_connect(videoSink_, "handoff", G_CALLBACK(processNewBuffer), this);

    return true;
}

bool GStreamerVideo::createAndLinkGstElements()
{
    playbin_ = gst_element_factory_make("playbin3", "player");
    videoBin_ = gst_bin_new("SinkBin");
    videoSink_ = gst_element_factory_make("fakesink", "video_sink");
    videoConvert_ = gst_element_factory_make("videoconvert", "video_convert");
    capsFilter_ = gst_element_factory_make("capsfilter", "caps_filter");
    videoConvertCaps_ = gst_caps_from_string("video/x-raw,format=(string)NV12,pixel-aspect-ratio=(fraction)1/1");

    if(!playbin_ || !videoSink_ || !videoConvert_ || !capsFilter_ || !videoConvertCaps_)
    {
        Logger::write(Logger::ZONE_DEBUG, "Video", "Could not create elements");
        return false;
    }

    g_object_set(G_OBJECT(videoSink_), "sync", TRUE, "qos", FALSE, NULL);
    g_object_set(G_OBJECT(capsFilter_), "caps", videoConvertCaps_, NULL);

    gst_bin_add_many(GST_BIN(videoBin_), videoConvert_, capsFilter_, videoSink_, NULL);
    if (!gst_element_link_many(videoConvert_, capsFilter_, videoSink_, NULL))
    {
        Logger::write(Logger::ZONE_DEBUG, "Video", "Could not link video processing elements");
        return false;
    }

    GstPad *videoSinkPad = gst_element_get_static_pad(videoConvert_, "sink");
    GstPad *ghostPad = gst_ghost_pad_new("sink", videoSinkPad);
    gst_element_add_pad(videoBin_, ghostPad);
    gst_object_unref(videoSinkPad);

    return true;
}


void GStreamerVideo::elementSetupCallback(GstElement *playbin, GstElement *element, GStreamerVideo *video) {
    #ifdef WIN32
    bool hardwareVideoAccel = Configuration::HardwareVideoAccel;
    if (!hardwareVideoAccel) {
        std::vector<std::string> decoderPluginNames = {"d3d11h264dec", "d3d11h265dec"};
        for (const auto& pluginName : decoderPluginNames) {
            GstElementFactory *factory = gst_element_factory_find(pluginName.c_str());
            if (factory) {
                gst_plugin_feature_set_rank(GST_PLUGIN_FEATURE(factory), GST_RANK_NONE);
                g_object_unref(factory);
            }
        }
    }
    #endif

    gchar *elementName = gst_element_get_name(element);
    if (g_str_has_prefix(elementName, "avdec_h264") || g_str_has_prefix(elementName, "avdec_h265")) {
        #ifdef WIN32
        if (!hardwareVideoAccel) {
        #endif
            // Modify the properties of the avdec_h265 element here
            g_object_set(G_OBJECT(element), "thread-type", 2, "max-threads", Configuration::AvdecMaxThreads, "direct-rendering", false, NULL);
        #ifdef WIN32
        }
        #endif
    }
    g_free(elementName);
}

void GStreamerVideo::processNewBuffer(GstElement * /* fakesink */, GstBuffer *buf, GstPad *new_pad, gpointer userdata)
{
    GStreamerVideo *video = (GStreamerVideo *)userdata;
    if (video && video->isPlaying_)
    {
        if (!video->frameReady_)
        {
            if (!video->width_ || !video->height_)
            {
                GstCaps *caps = gst_pad_get_current_caps(new_pad);
                GstStructure *s = gst_caps_get_structure(caps, 0);
                gst_structure_get_int(s, "width", &video->width_);
                gst_structure_get_int(s, "height", &video->height_);
                gst_caps_unref(caps);  // Don't forget to unref the caps
            }
            if ((video->height_ || video->width_) && !video->videoBuffer_)
            {
                SDL_LockMutex(SDL::getMutex());
                video->videoBuffer_ = gst_buffer_ref(buf);
                video->frameReady_ = true;
                SDL_UnlockMutex(SDL::getMutex());
            }
        }
    }
}


void GStreamerVideo::onEndOfStream()
{
    playCount_++;

    // If the number of loops is 0 or greater than the current playCount_, seek the playback to the beginning.
    if(!numLoops_ || numLoops_ > playCount_)
    {
        gst_element_seek(playbin_,
                     1.0,
                     GST_FORMAT_TIME,
                     GST_SEEK_FLAG_FLUSH,
                     GST_SEEK_TYPE_SET,
                     0,
                     GST_SEEK_TYPE_NONE,
                     GST_CLOCK_TIME_NONE);
    }
    else
    {
        stop();
    }
}


gboolean GStreamerVideo::busCallback(GstBus *bus, GstMessage *msg, gpointer data)
{
    GStreamerVideo *video = static_cast<GStreamerVideo*>(data);
    switch(GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_EOS:
            video->onEndOfStream();
            break;
        default:
            break;
    }
    return TRUE;  // Keep the watch alive.
}


void GStreamerVideo::update(float /* dt */) 
{
	if(!playbin_)
	{
		return;
	}

    SDL_LockMutex(SDL::getMutex());

    if (!texture_ && width_ != 0) 
    {
        texture_ = SDL_CreateTexture(SDL::getRenderer(monitor_), SDL_PIXELFORMAT_NV12,
                                    SDL_TEXTUREACCESS_STREAMING, width_, height_);
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    }

    if (videoBuffer_) 
    {
        // Check buffer layout only once
        if (bufferLayout_ == UNKNOWN)
        {
            GstVideoMeta *meta = gst_buffer_get_video_meta(videoBuffer_);
            gint expected_y_stride = GST_ROUND_UP_4(width_);
            gint expected_uv_stride = GST_ROUND_UP_4(expected_y_stride);
            gsize expected_uv_offset = height_ * expected_y_stride;

            // Assume CONTIGUOUS is more likely.
            if (!meta || meta->offset[0] != 0 || meta->stride[0] != expected_y_stride || 
                meta->stride[1] != expected_uv_stride || meta->offset[1] != expected_uv_offset)
            {
                bufferLayout_ = NON_CONTIGUOUS;
                Logger::write(Logger::ZONE_DEBUG, "Video", "Buffer is Non-Contiguous");
            }
            else 
            {
                bufferLayout_ = CONTIGUOUS;
                Logger::write(Logger::ZONE_DEBUG, "Video", "Buffer is Contiguous");
            }
        }

        GstMapInfo bufInfo;
        if (!gst_buffer_map(videoBuffer_, &bufInfo, GST_MAP_READ))
        {
            SDL_UnlockMutex(SDL::getMutex());
            return; // Early exit if unable to map the buffer
        }

        switch (bufferLayout_)
        {
            case CONTIGUOUS:
            {
                // Directly lock the texture for the entire area
                Uint8 *texture_pixels;
                int texture_pitch;
                if (SDL_LockTexture(texture_, NULL, (void**)&texture_pixels, &texture_pitch) < 0) {
                    Logger::write(Logger::ZONE_ERROR, "Video", "Unable to lock texture");
                    break;
                }

                // Pointers to Y and UV planes in the source buffer
                const Uint8 *src_y = (const Uint8 *)bufInfo.data;
                const Uint8 *src_uv = src_y + GST_ROUND_UP_4(width_) * height_;

                // Pointers to Y and UV planes in the texture
                Uint8 *dst_y = texture_pixels;
                Uint8 *dst_uv = dst_y + texture_pitch * height_;

                // Y plane copying
                for (int i = height_; i--;) {
                    SDL_memcpy(dst_y, src_y, width_);
                    src_y += GST_ROUND_UP_4(width_);
                    dst_y += texture_pitch;
                }

                // UV plane copying (NV12 specific)
                int uv_height = (height_ + 1) / 2;
                int uv_width = ((width_ + 1) / 2) * 2;
                int uv_src_pitch = ((GST_ROUND_UP_4(width_) + 1) / 2) * 2;
                for (int i = uv_height; i--;) {
                    SDL_memcpy(dst_uv, src_uv, uv_width);
                    src_uv += uv_src_pitch;
                    dst_uv += texture_pitch;
                }

                SDL_UnlockTexture(texture_);
                break;
            }

            case NON_CONTIGUOUS:
            {
                GstVideoMeta *meta = gst_buffer_get_video_meta(videoBuffer_);
                void *y_plane = bufInfo.data + (meta ? meta->offset[0] : 0);
                void *uv_plane = bufInfo.data + (meta ? meta->offset[1] : width_ * height_);
                int y_stride = meta ? meta->stride[0] : GST_ROUND_UP_4(width_);
                int uv_stride = meta ? meta->stride[1] : GST_ROUND_UP_4(y_stride);
                SDL_UpdateNVTexture(texture_, NULL, (const Uint8*)y_plane, y_stride, (const Uint8*)uv_plane, uv_stride);
                break;
            }
            
            default:
                // Should not reach here.
                break;
        }

        gst_buffer_unmap(videoBuffer_, &bufInfo);
        gst_buffer_unref(videoBuffer_);
        videoBuffer_ = NULL;
    }
    SDL_UnlockMutex(SDL::getMutex());
    volumeUpdate();
}

void GStreamerVideo::volumeUpdate()
{    
    bool shouldMute = false;
    double targetVolume = 0.0;
    bool muteVideo = Configuration::MuteVideo;
    if (muteVideo)
    {
        shouldMute = true;
    }
    else
    {
        if (volume_ > 1.0)
            volume_ = 1.0;
        if (currentVolume_ > volume_ || currentVolume_ + 0.005 >= volume_)
            currentVolume_ = volume_;
        else
            currentVolume_ += 0.005;
        targetVolume = static_cast<double>(currentVolume_);
        if (currentVolume_ < 0.1)
            shouldMute = true;
    }

    // Only set the volume if it has changed since the last call.
    if (targetVolume != lastSetVolume_)
    {
        gst_stream_volume_set_volume(GST_STREAM_VOLUME(playbin_), GST_STREAM_VOLUME_FORMAT_LINEAR, targetVolume);
        lastSetVolume_ = targetVolume;
    }
    // Only set the mute state if it has changed since the last call.
    if (shouldMute != lastSetMuteState_)
    {
        gst_stream_volume_set_mute(GST_STREAM_VOLUME(playbin_), shouldMute);
        lastSetMuteState_ = shouldMute;
    }
}


int GStreamerVideo::getHeight()
{
    return static_cast<int>(height_);
}

int GStreamerVideo::getWidth()
{
    return static_cast<int>(width_);
}


void GStreamerVideo::draw()
{
    frameReady_ = false;
}


bool GStreamerVideo::isPlaying()
{
    return isPlaying_;
}


void GStreamerVideo::setVolume(float volume)
{
    volume_ = volume;
}


void GStreamerVideo::skipForward( )
{

    if ( !isPlaying_ )
        return;

    gint64 current;
    gint64 duration;

    if ( !gst_element_query_position( playbin_, GST_FORMAT_TIME, &current ) )
        return;

    if ( !gst_element_query_duration( playbin_, GST_FORMAT_TIME, &duration ) )
        return;

    current += 60 * GST_SECOND;
    if ( current > duration )
        current = duration-1;
    gst_element_seek_simple( playbin_, GST_FORMAT_TIME, GstSeekFlags( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT ), current );

}


void GStreamerVideo::skipBackward( )
{

    if ( !isPlaying_ )
        return;

    gint64 current;

    if ( !gst_element_query_position( playbin_, GST_FORMAT_TIME, &current ) )
        return;

    if ( current > 60 * GST_SECOND )
        current -= 60 * GST_SECOND;
    else
        current = 0;
    gst_element_seek_simple( playbin_, GST_FORMAT_TIME, GstSeekFlags( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT ), current );

}


void GStreamerVideo::skipForwardp( )
{

    if ( !isPlaying_ )
        return;

    gint64 current;
    gint64 duration;

    if ( !gst_element_query_position( playbin_, GST_FORMAT_TIME, &current ) )
        return;

    if ( !gst_element_query_duration( playbin_, GST_FORMAT_TIME, &duration ) )
        return;

    current += duration/20;
    if ( current > duration )
        current = duration-1;
    gst_element_seek_simple( playbin_, GST_FORMAT_TIME, GstSeekFlags( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT ), current );

}


void GStreamerVideo::skipBackwardp( )
{

    if ( !isPlaying_ )
        return;

    gint64 current;
    gint64 duration;

    if ( !gst_element_query_position( playbin_, GST_FORMAT_TIME, &current ) )
        return;

    if ( !gst_element_query_duration( playbin_, GST_FORMAT_TIME, &duration ) )
        return;

    if ( current > duration/20 )
        current -= duration/20;
    else
        current = 0;
    gst_element_seek_simple( playbin_, GST_FORMAT_TIME, GstSeekFlags( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT ), current );

}


void GStreamerVideo::pause( )
{
    paused_ = !paused_;
    if (paused_)
        gst_element_set_state(GST_ELEMENT(playbin_), GST_STATE_PAUSED);
    else
        gst_element_set_state(GST_ELEMENT(playbin_), GST_STATE_PLAYING);
}


void GStreamerVideo::restart( )
{

    if ( !isPlaying_ )
        return;

    gst_element_seek_simple( playbin_, GST_FORMAT_TIME, GstSeekFlags( GST_SEEK_FLAG_FLUSH), 0 );

}


unsigned long long GStreamerVideo::getCurrent( )
{
    gint64 ret = 0;
    if ( !gst_element_query_position( playbin_, GST_FORMAT_TIME, &ret ) || !isPlaying_ )
        ret = 0;
    return (unsigned long long)ret;
}


unsigned long long GStreamerVideo::getDuration( )
{
    gint64 ret = 0;
    if ( !gst_element_query_duration( playbin_, GST_FORMAT_TIME, &ret ) || !isPlaying_ )
        ret = 0;
    return (unsigned long long)ret;
}


bool GStreamerVideo::isPaused( )
{
    return paused_;
}