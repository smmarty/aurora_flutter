//******************************************************************************
//******************************************************************************

#include "video_player.h"

#include <iostream>

//******************************************************************************
//******************************************************************************
class GstVideoPlayer::impl
{
friend class GstVideoPlayer;

    impl(GstVideoPlayer * owner, const int64_t textureId, NotifyInterface * handler);

private:
    GstVideoPlayer * m_o;

    struct VideoElements 
    {
        GstElement * pipeline;
        GstElement * playbin;
        GstElement * video_convert;
        GstElement * video_sink;
        GstElement * output;
        GstBus     * bus;
        GstBuffer  * buffer;
    };

    static void HandoffHandler(GstElement* fakesink, GstBuffer* buf,
                                GstPad* new_pad, gpointer user_data);
    static GstBusSyncReply HandleGstMessage(GstBus* bus, GstMessage* message,
                                              gpointer user_data);
    std::string ParseUri(const std::string & uri);
    bool CreatePipeline();
    void DestroyPipeline();
    void Preroll();
    void GetVideoSize(int32_t& width, int32_t& height);
#ifdef USE_EGL_IMAGE_DMABUF
    void UnrefEGLImage();
#endif  // USE_EGL_IMAGE_DMABUF

    VideoElements            m_gst;

    std::string              m_uri;

    std::shared_ptr<uint8_t[]> m_pixels;

    int32_t                  m_width;
    int32_t                  m_height;
    
    double                   m_volume          = 1.0;
    double                   m_playbackRate    = 1.0;
    bool                     m_mute            = false;
    bool                     m_autoRepeat      = false;
    bool                     m_isCompleted     = false;
    std::mutex               m_mutexEventCompleted;
    std::shared_mutex        m_mutexBuffer;

    int64_t                  m_textureId;
    NotifyInterface        * m_streamHandler;

#ifdef USE_EGL_IMAGE_DMABUF
    GstVideoInfo             m_gstVideoInfo;
    GstEGLImage            * m_gstVideoInfo    = nullptr;
    GstGLContext           * m_gstGlCtx        = nullptr;
    GstGLDisplayEGL        * m_gstGlDisplayEgl = nullptr;
#endif  // USE_EGL_IMAGE_DMABUF

};

//******************************************************************************
//******************************************************************************
GstVideoPlayer::impl::impl(GstVideoPlayer * owner, 
                            const int64_t textureId, 
                            NotifyInterface * handler)
    : m_o(owner)
    , m_textureId(textureId)
    , m_streamHandler(handler) 
{
    m_gst.pipeline      = nullptr;
    m_gst.playbin       = nullptr;
    m_gst.video_convert = nullptr;
    m_gst.video_sink    = nullptr;
    m_gst.output        = nullptr;
    m_gst.bus           = nullptr;
    m_gst.buffer        = nullptr;
}

//******************************************************************************
//******************************************************************************
GstVideoPlayer::GstVideoPlayer(const std::string & uri, 
                               const int64_t textureId, 
                               NotifyInterface * handler)
    : m_p(new impl(this, textureId, handler))
{

    m_p->m_uri = m_p->ParseUri(uri);
    if (!m_p->CreatePipeline()) 
    {
        std::cerr << "Failed to create a pipeline" << std::endl;
        m_p->DestroyPipeline();
        return;
    }

    // Prerolls before getting information from the pipeline.
    m_p->Preroll();

    // Sets internal video size and buffier.
    m_p->GetVideoSize(m_p->m_width, m_p->m_height);
    m_p->m_pixels.reset(new uint8_t[m_p->m_width * m_p->m_height * 4]);

    m_p->m_streamHandler->OnNotifyInitialized(m_p->m_textureId);
}

//******************************************************************************
//******************************************************************************
GstVideoPlayer::~GstVideoPlayer() 
{
#ifdef USE_EGL_IMAGE_DMABUF
    m_p->UnrefEGLImage();
#endif  // USE_EGL_IMAGE_DMABUF
    Stop();
    m_p->DestroyPipeline();
}

//******************************************************************************
// static
//******************************************************************************
void GstVideoPlayer::GstLibraryLoad() 
{ 
    gst_init(nullptr, nullptr); 
}

//******************************************************************************
// static
//******************************************************************************
void GstVideoPlayer::GstLibraryUnload() 
{ 
    gst_deinit(); 
}

//******************************************************************************
//******************************************************************************
bool GstVideoPlayer::Play() 
{
    if (gst_element_set_state(m_p->m_gst.pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) 
    {
        std::cerr << "Failed to change the state to PLAYING" << std::endl;
        return false;
    }

    m_p->m_streamHandler->OnNotifyPlaying(m_p->m_textureId, true);
    return true;
}

//******************************************************************************
//******************************************************************************
bool GstVideoPlayer::Pause() 
{
    if (gst_element_set_state(m_p->m_gst.pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE) 
    {
        std::cerr << "Failed to change the state to PAUSED" << std::endl;
        return false;
    }

    m_p->m_streamHandler->OnNotifyPlaying(m_p->m_textureId, false);
    return true;
}

//******************************************************************************
//******************************************************************************
bool GstVideoPlayer::Stop() 
{
    if (gst_element_set_state(m_p->m_gst.pipeline, GST_STATE_READY) == GST_STATE_CHANGE_FAILURE) 
    {
        std::cerr << "Failed to change the state to READY" << std::endl;
        return false;
    }

    m_p->m_streamHandler->OnNotifyPlaying(m_p->m_textureId, false);
    return true;
}

//******************************************************************************
//******************************************************************************
bool GstVideoPlayer::SetVolume(double volume) 
{
    if (!m_p->m_gst.playbin) 
    {
        return false;
    }

    m_p->m_volume = volume;
    g_object_set(m_p->m_gst.playbin, "volume", volume, NULL);
    return true;
}

//******************************************************************************
//******************************************************************************
bool GstVideoPlayer::SetPlaybackRate(double rate) 
{
    if (!m_p->m_gst.playbin) 
    {
        return false;
    }

    if (rate <= 0) 
    {
        std::cerr << "Rate " << rate << " is not supported" << std::endl;
        return false;
    }

    auto position = GetCurrentPosition();
    if (position < 0) 
    {
        return false;
    }

    if (!gst_element_seek(m_p->m_gst.pipeline, 
                            rate, 
                            GST_FORMAT_TIME, 
                            GST_SEEK_FLAG_FLUSH, 
                            GST_SEEK_TYPE_SET, 
                            position * GST_MSECOND, 
                            GST_SEEK_TYPE_SET,
                            GST_CLOCK_TIME_NONE)) 
    {
        std::cerr << "Failed to set playback rate to " << rate
                    << " (gst_element_seek failed)" << std::endl;
        return false;
    }

    m_p->m_playbackRate = rate;
    m_p->m_mute = (rate < 0.5 || rate > 2);
    g_object_set(m_p->m_gst.playbin, "mute", m_p->m_mute, NULL);

    return true;
}

//******************************************************************************
//******************************************************************************
void GstVideoPlayer::SetAutoRepeat(bool auto_repeat) 
{ 
    m_p->m_autoRepeat = auto_repeat; 
};

//******************************************************************************
//******************************************************************************
bool GstVideoPlayer::SetSeek(int64_t position) 
{
    auto nanosecond = position * 1000 * 1000;
    if (!gst_element_seek(m_p->m_gst.pipeline, 
                            m_p->m_playbackRate, 
                            GST_FORMAT_TIME,
                            (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                            GST_SEEK_TYPE_SET, 
                            nanosecond, 
                            GST_SEEK_TYPE_SET,
                            GST_CLOCK_TIME_NONE)) 
    {
        std::cerr << "Failed to seek " << nanosecond << std::endl;
        return false;
    }
    return true;
}

//******************************************************************************
//******************************************************************************
int64_t GstVideoPlayer::GetDuration() 
{
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 duration_msec;
    if (!gst_element_query_duration(m_p->m_gst.pipeline, fmt, &duration_msec)) 
    {
        std::cerr << "Failed to get duration" << std::endl;
        return -1;
    }
    duration_msec /= GST_MSECOND;
    return duration_msec;
}

//******************************************************************************
//******************************************************************************
int64_t GstVideoPlayer::GetCurrentPosition() 
{
    gint64 position = 0;

    // Sometimes we get an error when playing streaming videos.
    if (!gst_element_query_position(m_p->m_gst.pipeline, GST_FORMAT_TIME, &position)) 
    {
        std::cerr << "Failed to get current position" << std::endl;
        return -1;
    }

    // TODO: We need to handle this code in the proper plase.
    // The VideoPlayer plugin doesn't have a main loop, so EOS message
    // received from GStreamer cannot be processed directly in a callback
    // function. This is because the event channel message of playback complettion
    // needs to be thrown in the main thread.
    {
        std::unique_lock<std::mutex> lock(m_p->m_mutexEventCompleted);
        if (m_p->m_isCompleted) 
        {
            m_p->m_isCompleted = false;
            lock.unlock();

            if (m_p->m_autoRepeat) 
            {
                SetSeek(0);
            } 
            else 
            {
                m_p->m_streamHandler->OnNotifyCompleted(m_p->m_textureId);
            }
        }
    }

    return position / GST_MSECOND;
}

//******************************************************************************
//******************************************************************************
#ifdef USE_EGL_IMAGE_DMABUF
void * GstVideoPlayer::GetEGLImage(void * egl_display, void * egl_context) 
{
    std::shared_lock<std::shared_mutex> lock(m_p->m_mutexBuffer);
    if (!m_p->m_gst.buffer) 
    {
        return nullptr;
    }

    GstMemory * memory = gst_buffer_peek_memory(m_p->m_gst.buffer, 0);
    if (gst_is_dmabuf_memory(memory)) 
    {
        m_p->UnrefEGLImage();

        gint fd = gst_dmabuf_memory_get_fd(memory);
        m_p->m_gstGlDisplayEgl = gst_gl_display_egl_new_with_egl_display(reinterpret_cast<gpointer>(egl_display));
        m_p->m_gstGlCtx = gst_gl_context_new_wrapped(GST_GL_DISPLAY_CAST(m_p->m_gstGlDisplayEgl), 
                                                        reinterpret_cast<guintptr>(egl_context),
                                                        GST_GL_PLATFORM_EGL, 
                                                        GST_GL_API_GLES2);

        gst_gl_context_activate(m_p->m_gstGlCtx, TRUE);

        m_p->m_gstVideoInfo = gst_egl_image_from_dmabuf(m_p->m_gstGlCtx, fd, &m_p->m_gstVideoInfo, 0, 0);
        return reinterpret_cast<void*>(gst_egl_image_get_image(m_p->m_gstVideoInfo));
    }
    return nullptr;
}

//******************************************************************************
//******************************************************************************
void GstVideoPlayer::impl::UnrefEGLImage() 
{
    if (m_gstVideoInfo) 
    {
        gst_egl_image_unref(m_gstVideoInfo);
        gst_object_unref(m_gstGlCtx);
        gst_object_unref(m_gstGlDisplayEgl);
        m_gstVideoInfo    = nullptr;
        m_gstGlCtx        = nullptr;
        m_gstGlDisplayEgl = nullptr;
    }
}
#endif  // USE_EGL_IMAGE_DMABUF

//******************************************************************************
//******************************************************************************
int32_t GstVideoPlayer::GetWidth() const 
{ 
    return m_p->m_width; 
};

//******************************************************************************
//******************************************************************************
int32_t GstVideoPlayer::GetHeight() const 
{ 
    return m_p->m_height; 
};

//******************************************************************************
//******************************************************************************
const uint8_t * GstVideoPlayer::GetFrameBuffer() 
{
    std::shared_lock<std::shared_mutex> lock(m_p->m_mutexBuffer);
    if (!m_p->m_gst.buffer) 
    {
        return nullptr;
    }

    const uint32_t pixel_bytes = m_p->m_width * m_p->m_height * 4;
    gst_buffer_extract(m_p->m_gst.buffer, 0, m_p->m_pixels.get(), pixel_bytes);
    return m_p->m_pixels.get();
}

//******************************************************************************
// Creats a video pipeline using playbin.
// $ playbin uri=<file> video-sink="videoconvert ! video/x-raw,format=RGBA !
// fakesink"
//******************************************************************************
bool GstVideoPlayer::impl::CreatePipeline() 
{
    m_gst.pipeline = gst_pipeline_new("pipeline");
    if (!m_gst.pipeline) 
    {
        std::cerr << "Failed to create a pipeline" << std::endl;
        return false;
    }
    m_gst.playbin = gst_element_factory_make("playbin", "playbin");
    if (!m_gst.playbin) 
    {
        std::cerr << "Failed to create a source" << std::endl;
        return false;
    }
    m_gst.video_convert = gst_element_factory_make("videoconvert", "videoconvert");
    if (!m_gst.video_convert) 
    {
        std::cerr << "Failed to create a videoconvert" << std::endl;
        return false;
    }
    m_gst.video_sink = gst_element_factory_make("fakesink", "videosink");
    if (!m_gst.video_sink) 
    {
        std::cerr << "Failed to create a videosink" << std::endl;
        return false;
    }
    m_gst.output = gst_bin_new("output");
    if (!m_gst.output) 
    {
        std::cerr << "Failed to create an output" << std::endl;
        return false;
    }
    m_gst.bus = gst_pipeline_get_bus(GST_PIPELINE(m_gst.pipeline));
    if (!m_gst.bus) 
    {
        std::cerr << "Failed to create a bus" << std::endl;
        return false;
    }

    gst_bus_set_sync_handler(m_gst.bus, HandleGstMessage, m_o, nullptr);

    // Sets properties to fakesink to get the callback of a decoded frame.
    g_object_set(G_OBJECT(m_gst.video_sink), "sync", true, "qos", false, nullptr);
    g_object_set(G_OBJECT(m_gst.video_sink), "signal-handoffs", true, nullptr);
    g_signal_connect(G_OBJECT(m_gst.video_sink), "handoff", G_CALLBACK(HandoffHandler), m_o);
    gst_bin_add_many(GST_BIN(m_gst.output), m_gst.video_convert, m_gst.video_sink, nullptr);

    // Adds caps to the converter to convert the color format to RGBA.
    auto * caps    = gst_caps_from_string("video/x-raw,format=RGBA");
    auto   link_ok = gst_element_link_filtered(m_gst.video_convert, m_gst.video_sink, caps);
    gst_caps_unref(caps);
    if (!link_ok) 
    {
        std::cerr << "Failed to link elements" << std::endl;
        return false;
    }

    auto * sinkpad       = gst_element_get_static_pad(m_gst.video_convert, "sink");
    auto * ghost_sinkpad = gst_ghost_pad_new("sink", sinkpad);
    gst_pad_set_active(ghost_sinkpad, true);
    gst_element_add_pad(m_gst.output, ghost_sinkpad);

    // Sets properties to playbin.
    g_object_set(m_gst.playbin, "uri", m_uri.c_str(), nullptr);
    g_object_set(m_gst.playbin, "video-sink", m_gst.output, nullptr);
    gst_bin_add_many(GST_BIN(m_gst.pipeline), m_gst.playbin, nullptr);

    return true;
}

//******************************************************************************
//******************************************************************************
void GstVideoPlayer::impl::Preroll() 
{
    if (!m_gst.playbin) 
    {
        return;
    }

    auto result = gst_element_set_state(m_gst.pipeline, GST_STATE_PAUSED);
    if (result == GST_STATE_CHANGE_FAILURE) 
    {
        std::cerr << "Failed to change the state to PAUSED" << std::endl;
        return;
    }

    // Waits until the state becomes GST_STATE_PAUSED.
    if (result == GST_STATE_CHANGE_ASYNC) 
    {
        GstState state;
        result = gst_element_get_state(m_gst.pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
        if (result == GST_STATE_CHANGE_FAILURE) 
        {
            std::cerr << "Failed to get the current state" << std::endl;
        }
    }
}

//******************************************************************************
//******************************************************************************
void GstVideoPlayer::impl::DestroyPipeline() 
{
    if (m_gst.video_sink) 
    {
        g_object_set(G_OBJECT(m_gst.video_sink), "signal-handoffs", FALSE, NULL);
    }

    if (m_gst.pipeline) 
    {
        gst_element_set_state(m_gst.pipeline, GST_STATE_NULL);
    }

    if (m_gst.buffer) 
    {
        gst_buffer_unref(m_gst.buffer);
        m_gst.buffer = nullptr;
    }

    if (m_gst.bus) 
    {
        gst_object_unref(m_gst.bus);
        m_gst.bus = nullptr;
    }

    if (m_gst.pipeline) 
    {
        gst_object_unref(m_gst.pipeline);
        m_gst.pipeline = nullptr;
    }

    if (m_gst.playbin) 
    {
        m_gst.playbin = nullptr;
    }

    if (m_gst.output) 
    {
        m_gst.output = nullptr;
    }

    if (m_gst.video_sink) 
    {
        m_gst.video_sink = nullptr;
    }

    if (m_gst.video_convert) 
    {
        m_gst.video_convert = nullptr;
    }
}

//******************************************************************************
//******************************************************************************
std::string GstVideoPlayer::impl::ParseUri(const std::string & uri) 
{
    if (gst_uri_is_valid(uri.c_str())) 
    {
        return uri;
    }

    const auto * filename_uri = gst_filename_to_uri(uri.c_str(), nullptr);
    if (!filename_uri) 
    {
        std::cerr << "Faild to open " << uri.c_str() << std::endl;
        return uri;
    }
    std::string result_uri(filename_uri);
    delete filename_uri;

    return result_uri;
}

//******************************************************************************
//******************************************************************************
void GstVideoPlayer::impl::GetVideoSize(int32_t& width, int32_t& height) 
{
    if (!m_gst.pipeline || !m_gst.video_sink) 
    {
        std::cerr << "Failed to get video size. The pileline hasn't initialized yet.";
        return;
    }

    auto * sink_pad = gst_element_get_static_pad(m_gst.video_sink, "sink");
    if (!sink_pad) 
    {
        std::cerr << "Failed to get a pad";
        return;
    }

    auto * caps      = gst_pad_get_current_caps(sink_pad);
    auto * structure = gst_caps_get_structure(caps, 0);
    if (!structure) 
    {
        std::cerr << "Failed to get a structure";
        return;
    }

    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);

#ifdef USE_EGL_IMAGE_DMABUF
    gboolean res = gst_video_info_from_caps(&m_gstVideoInfo, caps);
    if (!res) 
    {
        std::cerr << "Failed to get a gst_video_info" << std::endl;
        return;
    }
#endif  // USE_EGL_IMAGE_DMABUF

    gst_caps_unref(caps);
}

//******************************************************************************
//******************************************************************************
// static
void GstVideoPlayer::impl::HandoffHandler(GstElement * /*fakesink*/, GstBuffer * buf,
                                             GstPad * new_pad, gpointer user_data) 
{
    auto * self = reinterpret_cast<GstVideoPlayer*>(user_data);
    auto * caps = gst_pad_get_current_caps(new_pad);
    auto * structure = gst_caps_get_structure(caps, 0);

    int width;
    int height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);
    gst_caps_unref(caps);

    if (width != self->m_p->m_width || height != self->m_p->m_height) 
    {
        self->m_p->m_width  = width;
        self->m_p->m_height = height;
        self->m_p->m_pixels.reset(new uint8_t[width * height * 4]);
        std::cout << "Pixel buffer size: width = " << width
                    << ", height = " << height << std::endl;
    }

    std::lock_guard<std::shared_mutex> lock(self->m_p->m_mutexBuffer);
    if (self->m_p->m_gst.buffer) 
    {
        gst_buffer_unref(self->m_p->m_gst.buffer);
        self->m_p->m_gst.buffer = nullptr;
    }
    self->m_p->m_gst.buffer = gst_buffer_ref(buf);
    self->m_p->m_streamHandler->OnNotifyFrameDecoded(self->m_p->m_textureId);
}

//******************************************************************************
//******************************************************************************
// static
GstBusSyncReply GstVideoPlayer::impl::HandleGstMessage(GstBus* /*bus*/,
                                                       GstMessage* message,
                                                         gpointer user_data) 
{
    switch (GST_MESSAGE_TYPE(message)) 
    {
        case GST_MESSAGE_EOS: 
        {
            auto * self = reinterpret_cast<GstVideoPlayer*>(user_data);
            std::lock_guard<std::mutex> lock(self->m_p->m_mutexEventCompleted);
            self->m_p->m_isCompleted = true;
            break;
        }
        case GST_MESSAGE_WARNING: 
        {
            gchar  * debug;
            GError * error;
            gst_message_parse_warning(message, &error, &debug);
            g_printerr("WARNING from element %s: %s\n", 
                        GST_OBJECT_NAME(message->src),
                        error->message);
            g_printerr("Warning details: %s\n", debug);
            g_free(debug);
            g_error_free(error);
            break;
        }
        case GST_MESSAGE_ERROR: 
        {
            gchar  * debug;
            GError * error;
            gst_message_parse_error(message, &error, &debug);
            g_printerr("ERROR from element %s: %s\n", 
                            GST_OBJECT_NAME(message->src),
                            error->message);
            g_printerr("Error details: %s\n", debug);
            g_free(debug);
            g_error_free(error);
            break;
        }
        default:
        break;
    }
    return GST_BUS_PASS;
}