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
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <gst/app/gstappsink.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>

bool GStreamerVideo::initialized_ = false;

//todo: this started out as sandbox code. This class needs to be refactored

// MUST match video size
//gboolean GStreamerVideo::busCallback(GstBus * /* bus */, GstMessage * /* msg */, gpointer /* data */)
//{ 
//    // this callback only needs to be defined so we can loop the video once it completes
//    return TRUE;
//}
GStreamerVideo::GStreamerVideo( int monitor )
    : playbin_(NULL)
    , videoBin_(NULL)
    , videoSink_(NULL)
    , videoConvert_(NULL)
    , videoConvertCaps_(NULL)
    , videoBus_(NULL)
    , texture_(NULL)
    , height_(0)
    , width_(0)
    , videoBuffer_(NULL)
    , frameReady_(false)
    , isPlaying_(false)
    , playCount_(0)
    , numLoops_(0)
	, volume_(0.0)
    , currentVolume_(0.0)
    , monitor_(monitor)
{
}
GStreamerVideo::~GStreamerVideo()
{
    stop();
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

void GStreamerVideo::processNewBuffer (GstElement * /* fakesink */, GstBuffer *buf, GstPad *new_pad, gpointer userdata)
{
    GStreamerVideo *video = (GStreamerVideo *)userdata;

    SDL_LockMutex(SDL::getMutex());
    if (!video->frameReady_ && video && video->isPlaying_)
    {
        if(!video->width_ || !video->height_)
        {
            GstCaps *caps = gst_pad_get_current_caps (new_pad);
            GstStructure *s = gst_caps_get_structure(caps, 0);

            gst_structure_get_int(s, "width", &video->width_);
            gst_structure_get_int(s, "height", &video->height_);
        }

        if(video->height_ && video->width_ && !video->videoBuffer_)
        {
            video->videoBuffer_ = gst_buffer_ref(buf);
            video->frameReady_ = true;
        }
    }
    SDL_UnlockMutex(SDL::getMutex());
}


bool GStreamerVideo::initialize()
{
    if(initialized_)
    {
        return true;
    }

    std::string path = Utils::combinePath(Configuration::absolutePath, "Core");
    gst_init(NULL, NULL);

#ifdef WIN32
    GstRegistry *registry = gst_registry_get();
    gst_registry_scan_path(registry, path.c_str());
#endif

    initialized_ = true;

    return true;
}

bool GStreamerVideo::deInitialize()
{
    gst_deinit();
    initialized_ = false;
    return true;
}


bool GStreamerVideo::stop()
{
    if(!initialized_)
    {
        return false;
    }

    if(videoSink_)
    {
        g_object_set(G_OBJECT(videoSink_), "signal-handoffs", FALSE, NULL);
    }

    if(playbin_)
    {
        (void)gst_element_set_state(playbin_, GST_STATE_NULL);
    }

    if(texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = NULL;
    }

    if(videoBuffer_)
    {
        gst_buffer_unref(videoBuffer_);
        videoBuffer_ = NULL;
    }

    freeElements();

    isPlaying_ = false;
    height_ = 0;
    width_ = 0;
    frameReady_ = false;

    return true;
}

bool GStreamerVideo::play(std::string file)
{
    playCount_ = 0;

    if(!initialized_)
    {
        return false;
    }

    stop();

    currentFile_ = file;

    const gchar *uriFile = gst_filename_to_uri (file.c_str(), NULL);
    if(!uriFile)
    {
        return false;
    }
    else
    {
//        Configuration::convertToAbsolutePath(Configuration::absolutePath, file);
        file = uriFile;
        delete uriFile;

        if(!playbin_)
        {
            playbin_ = gst_element_factory_make("playbin", "player");
            videoBin_ = gst_bin_new("SinkBin");
            videoSink_  = gst_element_factory_make("fakesink", "video_sink");
            videoConvert_  = gst_element_factory_make("capsfilter", "video_convert");
            videoConvertCaps_ = gst_caps_from_string("video/x-raw,format=(string)I420,pixel-aspect-ratio=(fraction)1/1");
            height_ = 0;
            width_ = 0;
            if(!playbin_)
            {
                Logger::write(Logger::ZONE_DEBUG, "Video", "Could not create playbin");
                freeElements();
                return false;
            }
            if(!videoSink_)
            {
                Logger::write(Logger::ZONE_DEBUG, "Video", "Could not create video sink");
                freeElements();
                return false;
            }
            if(!videoConvert_)
            {
                Logger::write(Logger::ZONE_DEBUG, "Video", "Could not create video converter");
                freeElements();
                return false;
            }
            if(!videoConvertCaps_)
            {
                Logger::write(Logger::ZONE_DEBUG, "Video", "Could not create video caps");
                freeElements();
                return false;
            }

            gst_bin_add_many(GST_BIN(videoBin_), videoConvert_, videoSink_, NULL);
            gst_element_link_filtered(videoConvert_, videoSink_, videoConvertCaps_);
            GstPad *videoConvertSinkPad = gst_element_get_static_pad(videoConvert_, "sink");

            if(!videoConvertSinkPad)
            {
                Logger::write(Logger::ZONE_DEBUG, "Video", "Could not get video convert sink pad");
                freeElements();
                return false;
            }

            g_object_set(G_OBJECT(videoSink_), "sync", TRUE, "qos", FALSE, NULL);

            GstPad *videoSinkPad = gst_ghost_pad_new("sink", videoConvertSinkPad);
            if(!videoSinkPad)
            {
                Logger::write(Logger::ZONE_DEBUG, "Video", "Could not get video bin sink pad");
                freeElements();
                gst_object_unref(videoConvertSinkPad);
                videoConvertSinkPad = NULL;
                return false;
            }

            gst_element_add_pad(videoBin_, videoSinkPad);
            gst_object_unref(videoConvertSinkPad);
            videoConvertSinkPad = NULL;
        }
        g_object_set(G_OBJECT(playbin_), "uri", file.c_str(), "video-sink", videoBin_, NULL);

        isPlaying_ = true;


        g_object_set(G_OBJECT(videoSink_), "signal-handoffs", TRUE, NULL);
        g_signal_connect(videoSink_, "handoff", G_CALLBACK(processNewBuffer), this);

        videoBus_ = gst_pipeline_get_bus(GST_PIPELINE(playbin_));
//        gst_bus_add_watch(videoBus_, &busCallback, this);

        /* Start playing */
        GstStateChangeReturn playState = gst_element_set_state(GST_ELEMENT(playbin_), GST_STATE_PLAYING);
        if (playState != GST_STATE_CHANGE_ASYNC)
        {
            isPlaying_ = false;
            std::stringstream ss;
            ss << "Unable to set the pipeline to the playing state: ";
            ss << playState;
            Logger::write(Logger::ZONE_ERROR, "Video", ss.str());
            freeElements();
            return false;
        }
    }

    gst_stream_volume_set_volume( GST_STREAM_VOLUME( playbin_ ), GST_STREAM_VOLUME_FORMAT_LINEAR, 0.0 );
    gst_stream_volume_set_mute( GST_STREAM_VOLUME( playbin_ ), true );

    return true;
}

void GStreamerVideo::freeElements()
{
    if(videoBus_)
    {
        gst_object_unref(videoBus_);
        videoBus_ = NULL;
    }
    if(playbin_)
    {
        gst_object_unref(playbin_);
        playbin_ = NULL;
    }
//    if(videoSink_)
//    {
//        gst_object_unref(videoSink_);
        videoSink_ = NULL;
//    }
//    if(videoConvert_)
//    {
 //       gst_object_unref(videoConvert_);
        videoConvert_ = NULL;
//    }
    if(videoConvertCaps_)
    {
        gst_caps_unref(videoConvertCaps_);
        videoConvertCaps_ = NULL;
    }
//    if(videoBin_)
//    {
//        gst_object_unref(videoBin_);
        videoBin_ = NULL;
//    }
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

void GStreamerVideo::update(float /* dt */)
{
    SDL_LockMutex(SDL::getMutex());
    if(!texture_ && width_ != 0 && height_ != 0)
    {
        texture_ = SDL_CreateTexture(SDL::getRenderer(monitor_), SDL_PIXELFORMAT_IYUV,
                                    SDL_TEXTUREACCESS_STREAMING, width_, height_);
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    }

	if(playbin_)
	{
		if(volume_ > 1.0)
			volume_ = 1.0;
        if ( currentVolume_ > volume_ || currentVolume_ + 0.005 >= volume_ )
            currentVolume_ = volume_;
        else
            currentVolume_ += 0.005;
		gst_stream_volume_set_volume( GST_STREAM_VOLUME( playbin_ ), GST_STREAM_VOLUME_FORMAT_LINEAR, static_cast<double>(currentVolume_));
		if(currentVolume_ < 0.1)
			gst_stream_volume_set_mute( GST_STREAM_VOLUME( playbin_ ), true );
		else
			gst_stream_volume_set_mute( GST_STREAM_VOLUME( playbin_ ), false );
	}

    if(videoBuffer_)
    {
        GstVideoMeta *meta;
        meta = gst_buffer_get_video_meta(videoBuffer_);

        // Presence of meta indicates non-contiguous data in the buffer
        if (!meta)
        {
            void *pixels;
            int pitch;
            unsigned int vbytes = width_ * height_;
            vbytes += (vbytes / 2);
            gsize bufSize = gst_buffer_get_size(videoBuffer_);

            if (bufSize == vbytes)
            {
                SDL_LockTexture(texture_, NULL, &pixels, &pitch);
                gst_buffer_extract(videoBuffer_, 0, pixels, vbytes);
                SDL_UnlockTexture(texture_);
            }
            else
            {
                GstMapInfo bufInfo;
                unsigned int y_stride, u_stride, v_stride;
                const Uint8 *y_plane, *u_plane, *v_plane;
                std::stringstream ss;

                y_stride = GST_ROUND_UP_4(width_);
                u_stride = v_stride = GST_ROUND_UP_4(y_stride / 2);

                gst_buffer_map(videoBuffer_, &bufInfo, GST_MAP_READ);
                y_plane = bufInfo.data;
                u_plane = y_plane + (height_ * y_stride);
                v_plane = u_plane + ((height_ / 2) * u_stride);
                SDL_UpdateYUVTexture(texture_, NULL,
                                     (const Uint8*)y_plane, y_stride,
                                     (const Uint8*)u_plane, u_stride,
                                     (const Uint8*)v_plane, v_stride);
                gst_buffer_unmap(videoBuffer_, &bufInfo);
            }
        }
        else
        {
            GstMapInfo y_info, u_info, v_info;
            void *y_plane, *u_plane, *v_plane;
            int y_stride, u_stride, v_stride;

            gst_video_meta_map(meta, 0, &y_info, &y_plane, &y_stride, GST_MAP_READ);
            gst_video_meta_map(meta, 1, &u_info, &u_plane, &u_stride, GST_MAP_READ);
            gst_video_meta_map(meta, 2, &v_info, &v_plane, &v_stride, GST_MAP_READ);
            SDL_UpdateYUVTexture(texture_, NULL,
                                 (const Uint8*)y_plane, y_stride,
                                 (const Uint8*)u_plane, u_stride,
                                 (const Uint8*)v_plane, v_stride);
            gst_video_meta_unmap(meta, 0, &y_info);
            gst_video_meta_unmap(meta, 1, &u_info);
            gst_video_meta_unmap(meta, 2, &v_info);
        }

        gst_buffer_unref(videoBuffer_);
        videoBuffer_ = NULL;
    }

    if(videoBus_)
    {
        GstMessage *msg = gst_bus_pop(videoBus_);
        if(msg)
        {
            if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
            {
                playCount_++;

                //todo: nesting hazard
                // if number of loops is 0, set to infinite (todo: this is misleading, rename variable)
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
                    isPlaying_ = false;
                }
            }

            gst_message_unref(msg);
        }
    }
    SDL_UnlockMutex(SDL::getMutex());
}


bool GStreamerVideo::isPlaying()
{
    return isPlaying_;
}


void GStreamerVideo::setVolume(float volume)
{
    volume_ = volume;
}
