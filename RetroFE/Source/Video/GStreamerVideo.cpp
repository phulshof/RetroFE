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
    , videoConvertCaps_(NULL)
    , videoBus_(NULL)
    , texture_(NULL)
    , height_(0)
    , width_(0)
    , videoBuffer_(NULL)
    , pool_(NULL)
    , frameReady_(false)
    , isPlaying_(false)
    , playCount_(0)
    , numLoops_(0)
    , volume_(0.0)
    , currentVolume_(0.0)
    , monitor_(monitor)
	  , MuteVideo(Configuration::MuteVideo)
    , hide_(false)
    , nv12BufferSize_(0)
    , lastSetVolume_(0.0)
    , lastSetMuteState_(false)
{
    paused_ = false;
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
    
    if(videoSink_)
    {
        g_object_set(G_OBJECT(videoSink_), "emit-signals", FALSE, NULL);
    }

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

    // Added buffer pool cleanup logic
    if (pool_)
    {
        gst_buffer_pool_set_active(pool_, FALSE);
        gst_object_unref(pool_);
        pool_ = NULL;
    }

    freeElements();

    isPlaying_ = false;
    height_ = 0;
    width_ = 0;
    frameReady_ = false;
    
    Logger::write(Logger::ZONE_DEBUG, "GStreamerVideo", "Stopped " + Utils::getFileName(currentFile_));
    
    return true;
}

void GStreamerVideo::hide(bool hide)
{
    hide_ = hide;
}

bool GStreamerVideo::play(std::string file)
{

    playCount_ = 0;

    if(!initialized_)
    {
        return false;
    }

    currentFile_ = file;

    //stop();

    gchar *uriFile = gst_filename_to_uri (file.c_str(), NULL);
    if(!uriFile)
    {
        return false;
    }
    else
    {
        if(!playbin_)
        {
            playbin_ = gst_element_factory_make("playbin3", "player");
            videoBin_ = gst_bin_new("SinkBin");
            videoSink_  = gst_element_factory_make("appsink", "video_sink");
            videoConvert_  = gst_element_factory_make("capsfilter", "video_convert");
            videoConvertCaps_ = gst_caps_from_string("video/x-raw,format=(string)NV12,pixel-aspect-ratio=(fraction)1/1");
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
        g_object_set(G_OBJECT(playbin_), "uri", uriFile, "video-sink", videoBin_, NULL);
        g_free( uriFile );
		
		#ifdef WIN32
		std::vector<std::string> decoderPluginNames = {"d3d11h264dec", "d3d11h265dec"}; //add decoder names to disable when HardwareVideoAccel = false
		bool HardwareVideoAccel = Configuration::HardwareVideoAccel;
		if (!HardwareVideoAccel)
		{
			for (auto& pluginName : decoderPluginNames)
			{
			GstElementFactory *factory = gst_element_factory_find(pluginName.c_str());
			if (factory)
			{
				gst_plugin_feature_set_rank(GST_PLUGIN_FEATURE(factory), GST_RANK_NONE);
				g_object_unref(factory);
			}
			}
		}
		#endif


        g_signal_connect(playbin_, "element-setup", G_CALLBACK(+[](GstElement *playbin, GstElement *element, gpointer data) {
        GStreamerVideo *video = static_cast<GStreamerVideo *>(data);

        if (video)
        {
            gchar *elementName = gst_element_get_name(element);

                if (g_str_has_prefix(elementName, "avdec_h264") || g_str_has_prefix(elementName, "avdec_h265"))
                {
                    // Modify the properties of the avdec_h265 element here
                    // set "thread-type" property to 2 and AvdecMaxThreads in settings.conf (defaults to 2)
                    g_object_set(G_OBJECT(element), "thread-type", 2, "max-threads", Configuration::AvdecMaxThreads, NULL);
                }

            g_free(elementName);
        }
        }), this);

		videoBus_ = gst_pipeline_get_bus(GST_PIPELINE(playbin_));

        g_object_set(G_OBJECT(videoSink_), "emit-signals", TRUE, "drop", TRUE, "max-buffers", 0, NULL);
        g_signal_connect(videoSink_, "new-sample", G_CALLBACK(GStreamerVideo::static_on_new_sample), this);
        /* Start playing */
        GstStateChangeReturn playState = gst_element_set_state(GST_ELEMENT(playbin_), GST_STATE_PLAYING);
        Logger::write(Logger::ZONE_DEBUG, "GStreamerVideo", "Playing " + Utils::getFileName(currentFile_));
        
        isPlaying_ = true;
        
        //gst_debug_bin_to_dot_file(GST_BIN(playbin_), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
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
    if(videoConvertCaps_)
    {
        gst_caps_unref(videoConvertCaps_);
        videoConvertCaps_ = NULL;
    }
    videoSink_    = NULL;
    videoConvert_ = NULL;
    videoBin_     = NULL;
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
    if (playbin_)
    {
        bool shouldMute = false;
        double targetVolume = 0.0;
        if (MuteVideo)
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

    if (!hide_)
    {
        //SDL_LockMutex(SDL::getMutex());
        if (!texture_ && width_ != 0) //no need to check height here
        {
            texture_ = SDL_CreateTexture(SDL::getRenderer(monitor_), SDL_PIXELFORMAT_NV12,
                                        SDL_TEXTUREACCESS_STREAMING, width_, height_);
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
        }

        if (videoBuffer_)
        {
            GstMapInfo bufInfo;

            if (gst_buffer_map(videoBuffer_, &bufInfo, GST_MAP_READ))
            {
                const Uint8 *Yplane = bufInfo.data;
                const Uint8 *UVplane = bufInfo.data + (width_ * height_); // Start of UV data
                int Ypitch = width_;
                int UVpitch = width_; // Given it's packed UV, it's the same as width

                if (SDL_UpdateNVTexture(texture_, NULL, Yplane, Ypitch, UVplane, UVpitch) != 0) 
                {
                    Logger::write(Logger::ZONE_ERROR, "Video", SDL_GetError());
                    gst_buffer_unmap(videoBuffer_, &bufInfo);
                    gst_buffer_unref(videoBuffer_);
                    videoBuffer_ = NULL;
                    SDL_UnlockMutex(SDL::getMutex());
                    return; // Depending on your flow, adjust this exit point accordingly
                }

                gst_buffer_unmap(videoBuffer_, &bufInfo);
                gst_buffer_unref(videoBuffer_);
                videoBuffer_ = NULL;
            }
        }
        //SDL_UnlockMutex(SDL::getMutex());
    }

    if(videoBus_)
    {
        GstMessage *msg = gst_bus_pop_filtered(videoBus_, GST_MESSAGE_EOS);
        if(msg)
        {
            playCount_++;

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
                Logger::write(Logger::ZONE_DEBUG, "GStreamerVideo", "End Of Stream " + Utils::getFileName(currentFile_));
                stop();
            }
            gst_message_unref(msg);
        }
    }
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
    {
        gst_element_set_state(GST_ELEMENT(playbin_), GST_STATE_PAUSED);
        Logger::write(Logger::ZONE_DEBUG, "GStreamerVideo", "Pausing " + Utils::getFileName(currentFile_));
    }
    else
    {
        gst_element_set_state(GST_ELEMENT(playbin_), GST_STATE_PLAYING);
        Logger::write(Logger::ZONE_DEBUG, "GStreamerVideo", "Unpausing " + Utils::getFileName(currentFile_));
    }
}


void GStreamerVideo::restart( )
{

    if ( !isPlaying_ )
    {
            Logger::write(Logger::ZONE_DEBUG, "GStreamerVideo", "Not playing return");
        return;
    }
    Logger::write(Logger::ZONE_DEBUG, "GStreamerVideo", "Seek to Start of " + Utils::getFileName(currentFile_));
    gst_element_seek_simple( playbin_, GST_FORMAT_TIME, GstSeekFlags( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT ), 0 );

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


int GStreamerVideo::getNumLoops( )
{
    return numLoops_;
}

GstFlowReturn GStreamerVideo::static_on_new_sample(GstAppSink *appsink, gpointer userdata) 
{
    return static_cast<GStreamerVideo*>(userdata)->member_on_new_sample(appsink);
}

GstFlowReturn GStreamerVideo::member_on_new_sample(GstAppSink *appsink) 
{
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (!sample) {
        return GST_FLOW_ERROR;
    }

    GstBuffer *buf = gst_sample_get_buffer(sample);

    if (isPlaying()) 
    {
        //SDL_LockMutex(SDL::getMutex());
        if (!frameReady_) 
        {
            // Getting video dimensions from the sample's caps
            if (!getWidth() || !getHeight()) 
            {
                GstCaps *caps = gst_sample_get_caps(sample);
                GstStructure *s = gst_caps_get_structure(caps, 0);
                gst_structure_get_int(s, "width", &width_);
                gst_structure_get_int(s, "height", &height_);
                nv12BufferSize_ = width_ * height_ * 3 / 2;
                
                if (!initializeBufferPool()) 
                {
                    Logger::write(Logger::ZONE_ERROR, "Video", "Failed to initialize buffer pool");
                    return GST_FLOW_ERROR;
                }
                if (!width_ || !height_) 
                {
                    width_ = 0;
                    height_ = 0;
                }
            }
            if (width_ && !videoBuffer_) 
            {
                // Get a new buffer from the pool
                GstBuffer *poolBuffer = getBufferFromPool();

                if (!poolBuffer) {
                    Logger::write(Logger::ZONE_ERROR, "Video", "Failed to get buffer from pool");
                    return GST_FLOW_ERROR;
                }

                // Copy data from buf to poolBuffer
                GstMapInfo srcInfo, dstInfo;
                if (gst_buffer_map(buf, &srcInfo, GST_MAP_READ) &&
                    gst_buffer_map(poolBuffer, &dstInfo, GST_MAP_WRITE)) {
                    SDL_memcpy(dstInfo.data, srcInfo.data, nv12BufferSize_);  // Ensure this is safe to do!

                    gst_buffer_unmap(buf, &srcInfo);
                    gst_buffer_unmap(poolBuffer, &dstInfo);
                } else {
                    Logger::write(Logger::ZONE_ERROR, "Video", "Failed to map buffers");
                    // Handle error - maybe unref poolBuffer and return an error
                }

                videoBuffer_ = poolBuffer;
                frameReady_ = true;
            }
        }
        //SDL_UnlockMutex(SDL::getMutex());
    }
    gst_sample_unref(sample);
    return GST_FLOW_OK;
}


bool GStreamerVideo::initializeBufferPool() {
    if (pool_) {
        // If the pool already exists, simply return true.
        return true;
    }

    pool_ = gst_buffer_pool_new();
    if (!pool_) {
        Logger::write(Logger::ZONE_ERROR, "Video", "Failed to create a new buffer pool");
        return false;
    }

    GstStructure *config = gst_buffer_pool_get_config(pool_);
    if (!config) {
        Logger::write(Logger::ZONE_ERROR, "Video", "Failed to get buffer pool configuration");
        g_object_unref(pool_);
        pool_ = NULL;
        return false;
    }

    gst_buffer_pool_config_set_params(config, videoConvertCaps_, nv12BufferSize_, 0, 0);
    if (!gst_buffer_pool_set_config(pool_, config)) {
        Logger::write(Logger::ZONE_ERROR, "Video", "Failed to set buffer pool configuration");
        g_object_unref(pool_);
        pool_ = NULL;
        return false;
    }

    gst_buffer_pool_set_active(pool_, TRUE);
    return true;
}


GstBuffer* GStreamerVideo::getBufferFromPool() 
{
    GstBuffer *poolBuffer = NULL;
    GstFlowReturn flow_return;

    if (!pool_) {
        Logger::write(Logger::ZONE_ERROR, "Video", "Buffer pool is not initialized");
        return NULL;
    }

    // Acquire a buffer from the pool
    flow_return = gst_buffer_pool_acquire_buffer(pool_, &poolBuffer, NULL);

    if (flow_return != GST_FLOW_OK || !poolBuffer) {
        Logger::write(Logger::ZONE_ERROR, "Video", "Failed to acquire buffer from pool");
        return NULL;
    }

    return poolBuffer;
}