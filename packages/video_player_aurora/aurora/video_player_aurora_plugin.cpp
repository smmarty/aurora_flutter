// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#include "encodable_helper.h"

#include <video_player_aurora/video_player_aurora_plugin.h>

// #include <flutter/basic-message-channel.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
// #include <flutter/message-codec-type.h>
// #include <flutter/method-codec-type.h>
#include <unistd.h>

#include <unordered_map>
#include <filesystem>

#include "video_player.h"
#include "video_player_notify_interface.h"

//******************************************************************************
//******************************************************************************
typedef flutter::MethodCall<flutter::EncodableValue> MethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> MethodResult;

//******************************************************************************
//******************************************************************************
// constexpr char kVideoPlayerApiChannelInitializeName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.initialize";
// constexpr char kVideoPlayerApiChannelSetMixWithOthersName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.setMixWithOthers";
// constexpr char kVideoPlayerApiChannelCreateName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.create";
// constexpr char kVideoPlayerApiChannelDisposeName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.dispose";
// constexpr char kVideoPlayerApiChannelSetLoopingName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.setLooping";
// constexpr char kVideoPlayerApiChannelSetVolumeName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.setVolume";
// constexpr char kVideoPlayerApiChannelPauseName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.pause";
// constexpr char kVideoPlayerApiChannelPlayName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.play";
// constexpr char kVideoPlayerApiChannelPositionName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.position";
// constexpr char kVideoPlayerApiChannelSetPlaybackSpeedName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.setPlaybackSpeed";
// constexpr char kVideoPlayerApiChannelSeekToName[] =
//     "dev.flutter.pigeon.VideoPlayerApi.seekTo";

// constexpr char kVideoPlayerVideoEventsChannelName[] =
//     "flutter.io/videoPlayer/videoEvents";

// constexpr char kEncodableMapkeyResult[] = "result";
// constexpr char kEncodableMapkeyError[] = "error";

//******************************************************************************
//******************************************************************************
namespace 
{
    const std::string g_channelName        = "friflex/video_player_aurora";
    const std::string g_eventsChannelName  = "friflex/video_player_aurora/events";

    const std::string g_initialize         = "initialize";
    const std::string g_dispose            = "dispose";
    const std::string g_create             = "create";
    const std::string g_setLooping         = "setLooping";
    const std::string g_play               = "play";
    const std::string g_pause              = "pause";
    const std::string g_setVolume          = "setVolume";
    const std::string g_setPlaybackSpeed   = "setPlaybackSpeed";
    const std::string g_seekTo             = "seekTo";
    const std::string g_getPosition        = "getPosition";
    const std::string g_setMixWithOthers   = "setMixWithOthers";

    const std::string g_resultName         = "result";
    const std::string g_errorName          = "error";
}

//******************************************************************************
//******************************************************************************
struct VideoPlayerContext 
{
    std::recursive_mutex                        locker;

    int64_t                                     textureId;
    std::shared_ptr<flutter::TextureVariant>    textureVariant;
    std::shared_ptr<FlutterDesktopPixelBuffer>  buffer;

    std::shared_ptr<GstVideoPlayer>             player;

    std::shared_ptr<flutter::EventChannel<flutter::EncodableValue> > eventChannel;  
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue> >    eventSink;

// #ifdef USE_EGL_IMAGE_DMABUF
//     std::shared_ptr<FlutterDesktopEGLImage> eglImage;
// #endif 

};

//******************************************************************************
//******************************************************************************
class VideoPlayerAuroraPlugin::impl : public NotifyInterface
{
    friend class VideoPlayerAuroraPlugin;

    // 
    impl(VideoPlayerAuroraPlugin * owner, flutter::PluginRegistrar * registrar);

    // 
    void onMethodCall(const MethodCall & call, std::unique_ptr<MethodResult> result);

    // 
    void unimplemented(const MethodCall & call, std::unique_ptr<MethodResult> & result);

    // 
    void onInitialize(const MethodCall & call, std::unique_ptr<MethodResult> & result);

    // 
    void onCreate(const MethodCall & call, std::unique_ptr<MethodResult> & result);

    // 
    void onDispose(const MethodCall & call, std::unique_ptr<MethodResult> & result);
    
    // 
    void onSetLooping(const MethodCall & call, std::unique_ptr<MethodResult> & result);
    
    // 
    void onPlay(const MethodCall & call, std::unique_ptr<MethodResult> & result);
    
    // 
    void onPause(const MethodCall & call, std::unique_ptr<MethodResult> & result);
    
    // 
    void onSetVolume(const MethodCall & call, std::unique_ptr<MethodResult> & result);
    
    // 
    void onSetPlaybackSpeed(const MethodCall & call, std::unique_ptr<MethodResult> & result);
    
    // 
    void onSeekTo(const MethodCall & call, std::unique_ptr<MethodResult> & result);
    
    // 
    void onGetPosition(const MethodCall & call, std::unique_ptr<MethodResult> & result);

    // 
    void onSetMixWithOthers(const MethodCall & call, std::unique_ptr<MethodResult> & result);



    // Notifies the completion of initializing the video player.
    void OnNotifyInitialized(int64_t textureId);

    // Notifies the completion of decoding a video frame.
    void OnNotifyFrameDecoded(int64_t textureId);

    // Notifies the completion of playing a video.
    void OnNotifyCompleted(int64_t textureId);

    // Notifies update of playing or pausing a video.
    void OnNotifyPlaying(int64_t textureId, bool is_playing);


    //
    static std::string findAssetsDirectory();

    //
    static EncodableMap wrapError(const std::string& message,
                                    const std::string& code = std::string(),
                                    const std::string& details = std::string());


    VideoPlayerAuroraPlugin * m_o;

    // 
    flutter::PluginRegistrar  * m_pluginRegistrar;
    flutter::TextureRegistrar * m_textureRegistrar;

    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue> > m_methodChannel;

    std::recursive_mutex m_locker;
    std::unordered_map<int64_t, std::shared_ptr<VideoPlayerContext> > m_players;


    typedef void (VideoPlayerAuroraPlugin::impl:: * method_handler_t)(const MethodCall & call, 
                                                                        std::unique_ptr<MethodResult> & result);
    std::unordered_map<std::string, method_handler_t> m_handlers;
};

//******************************************************************************
//******************************************************************************
VideoPlayerAuroraPlugin::impl::impl(VideoPlayerAuroraPlugin * owner, flutter::PluginRegistrar * registrar)
    : m_o(owner)
    , m_pluginRegistrar(registrar)
    , m_textureRegistrar(registrar->texture_registrar())
    , m_methodChannel(new flutter::MethodChannel(registrar->messenger(), 
                                                g_channelName,
                                                &flutter::StandardMethodCodec::GetInstance()))
{
    m_handlers[g_initialize]       = &VideoPlayerAuroraPlugin::impl::onInitialize;
    m_handlers[g_dispose]          = &VideoPlayerAuroraPlugin::impl::onDispose;
    m_handlers[g_create]           = &VideoPlayerAuroraPlugin::impl::onCreate;
    m_handlers[g_setLooping]       = &VideoPlayerAuroraPlugin::impl::onSetLooping;
    m_handlers[g_play]             = &VideoPlayerAuroraPlugin::impl::onPlay;
    m_handlers[g_pause]            = &VideoPlayerAuroraPlugin::impl::onPause;
    m_handlers[g_setVolume]        = &VideoPlayerAuroraPlugin::impl::onSetVolume;
    m_handlers[g_setPlaybackSpeed] = &VideoPlayerAuroraPlugin::impl::onSetPlaybackSpeed;
    m_handlers[g_seekTo]           = &VideoPlayerAuroraPlugin::impl::onSeekTo;
    m_handlers[g_getPosition]      = &VideoPlayerAuroraPlugin::impl::onGetPosition;
    m_handlers[g_setMixWithOthers] = &VideoPlayerAuroraPlugin::impl::onSetMixWithOthers;

    m_methodChannel->SetMethodCallHandler([this](const MethodCall & call, std::unique_ptr<MethodResult> result)
    {
        onMethodCall(call, std::move(result));
    });
}

//******************************************************************************
//******************************************************************************
VideoPlayerAuroraPlugin::VideoPlayerAuroraPlugin(flutter::PluginRegistrar * registrar)
  : m_p(new impl(this, registrar))
{        
    GstVideoPlayer::GstLibraryLoad();
}

//******************************************************************************
//******************************************************************************
// static
void VideoPlayerAuroraPlugin::RegisterWithRegistrar(flutter::PluginRegistrar * registrar)
{
    std::unique_ptr<VideoPlayerAuroraPlugin> plugin(new VideoPlayerAuroraPlugin(registrar));
    registrar->AddPlugin(std::move(plugin));
}

//******************************************************************************
//******************************************************************************
VideoPlayerAuroraPlugin::~VideoPlayerAuroraPlugin() 
{
    {
        std::lock_guard<std::recursive_mutex> guard(m_p->m_locker);

        for (const auto & pair : m_p->m_players)
        {
             auto     player    = pair.second;
             player->player.reset();
             player->buffer.reset();
             m_p->m_textureRegistrar->UnregisterTexture(pair.first);
        }
        m_p->m_players.clear();
    }

    GstVideoPlayer::GstLibraryUnload();
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onMethodCall(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> result)
{
    const std::string & method = call.method_name();

    std::cout << "CALL " << method << std::endl;;

    auto it = m_handlers.find(method);
    if (it == m_handlers.end())
    {
        unimplemented(call, result);
        return;
    }

    // TODO looks ugly but c'est la vie
    // (this->*m_handlers[method])(call);
    (this->*it->second) (call, result);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::unimplemented(const MethodCall & /*call*/, 
                                                    std::unique_ptr<MethodResult> & result) 
{
    result->Success();
}

//******************************************************************************
//******************************************************************************
int intArg(const MethodCall & call, const std::string & name, const int defaultValue = -1)
{
    if (Helper::TypeIs<EncodableMap>(*call.arguments())) 
    {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*call.arguments());
        return Helper::GetInt(params, name);
    }

    return defaultValue;
}

//******************************************************************************
//******************************************************************************
std::string stringArg(const MethodCall & call, const std::string & name, const std::string & defaultValue = "")
{
    if (Helper::TypeIs<EncodableMap>(*call.arguments())) 
    {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*call.arguments());
        return Helper::GetString(params, name);
    }

    return defaultValue;
}

//******************************************************************************
//******************************************************************************
bool boolArg(const MethodCall & call, const std::string & name, const bool & defaultValue = false)
{
    if (Helper::TypeIs<EncodableMap>(*call.arguments())) 
    {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*call.arguments());
        return Helper::GetBool(params, name);
    }

    return defaultValue;
}

//******************************************************************************
//******************************************************************************
double doubleArg(const MethodCall & call, const std::string & name, const double defaultValue = 0)
{
    if (Helper::TypeIs<EncodableMap>(*call.arguments())) 
    {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*call.arguments());
        return Helper::GetDouble(params, name);
    }

    return defaultValue;
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onInitialize(const MethodCall & /*call*/, 
                                                    std::unique_ptr<MethodResult> & result)
{
    EncodableMap map;
    map.emplace(std::make_pair(EncodableValue(g_resultName), EncodableValue()));

    result->Success(flutter::EncodableValue(map));
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onCreate(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    std::string uri;

    std::string asset = stringArg(call, "asset"); // call.GetArgument(Encodable::String("asset")).GetString();
    if (asset.empty())
    {
        uri = stringArg(call, "uri");
    }
    else
    {
        std::string assetsPath = findAssetsDirectory();
        uri = "file://"+ assetsPath +"/flutter_assets/"+ asset;
    }

    std::cout << "\turl " << uri << std::endl;

    std::shared_ptr<VideoPlayerContext> context(new VideoPlayerContext);

// #ifdef USE_EGL_IMAGE_DMABUF
//   // context->egl_image = std::make_unique<FlutterDesktopEGLImage>();
//   // context->texture =
//   //     std::make_unique<flutter::TextureVariant>(flutter::EGLImageTexture(
//   //         [context](
//   //             size_t width, size_t height, void* egl_display,
//   //             void* egl_context) -> const FlutterDesktopEGLImage* {
//   //           context->egl_image->width = context->player->GetWidth();
//   //           context->egl_image->height = context->player->GetHeight();
//   //           context->egl_image->egl_image =
//   //               context->player->GetEGLImage(egl_display, egl_context);
//   //           return context->egl_image.get();
//   //         }));
// #else

    context->buffer = std::make_unique<FlutterDesktopPixelBuffer>();

    context->textureVariant = std::make_shared<flutter::TextureVariant>(
                            flutter::PixelBufferTexture([context](size_t /*width*/, size_t /*height*/) -> const FlutterDesktopPixelBuffer *
                                {
                                    context->buffer->width  = context->player->GetWidth();
                                    context->buffer->height = context->player->GetHeight();
                                    context->buffer->buffer = context->player->GetFrameBuffer();
                                    return context->buffer.get();
                                }));

// #endif  // USE_EGL_IMAGE_DMABUF

    context->textureId = m_textureRegistrar->RegisterTexture(context->textureVariant.get());

    std::cout << "\ttextureId " << context->textureId << std::endl;

    {
        auto listenHandler = flutter::StreamHandlerListen<flutter::EncodableValue>(
                                [context, this](const flutter::EncodableValue * /*arguments*/,
                                                std::unique_ptr<flutter::EventSink<flutter::EncodableValue> > && events)
                                                            -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue> >
                                {
                                    {
                                        std::lock_guard<std::recursive_mutex> guard(context->locker);
                                        context->eventSink = std::move(events);
                                    }
   
                                    OnNotifyInitialized(context->textureId);
                                    return nullptr;
                                });

        auto cancelHandler = flutter::StreamHandlerCancel<flutter::EncodableValue>(
                                [context](const flutter::EncodableValue * /*arguments*/)
                                                            -> std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue> > 
                                {
                                    std::lock_guard<std::recursive_mutex> guard(context->locker);
                                    context->eventSink = nullptr;
                                    return nullptr;
                                });

        auto channelHandler = std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue> >(listenHandler, cancelHandler);

        std::string eventsChannel = g_eventsChannelName + std::to_string(context->textureId);

        context->eventChannel.reset(new flutter::EventChannel<flutter::EncodableValue>(
                                                        m_pluginRegistrar->messenger(),
                                                        eventsChannel,
                                                        &flutter::StandardMethodCodec::GetInstance()));

        context->eventChannel->SetStreamHandler(std::move(channelHandler));
    }

    {
        context->player.reset(new GstVideoPlayer(uri, context->textureId, this));

        {
            std::lock_guard<std::recursive_mutex> guard(m_locker);
            m_players[context->textureId] = context;
        }
    }

    EncodableMap map;
    map.emplace("textureId", context->textureId);

    EncodableMap value;
    value.emplace(g_resultName, map);

    result->Success(value);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onDispose(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);

        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));        
            result->Success(map);
            return;
        }

        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);

        player->player.reset();

        m_players.erase(textureId);

        m_textureRegistrar->UnregisterTexture(textureId);
    }
    
    map.emplace(g_resultName, EncodableValue());
    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onSetLooping(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);

        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);
        player->player->SetAutoRepeat(boolArg(call, "isLooping"));
    }

    map.emplace(g_resultName, EncodableValue());
    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onPlay(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);

        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);
        player->player->Play();
    }

    map.emplace(g_resultName, EncodableValue());
    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onPause(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);
        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);
        player->player->Pause();
    }
  
    map.emplace(g_resultName, EncodableValue());
    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onSetVolume(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);
        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);
        player->player->SetVolume(doubleArg(call, "volume"));
    }
  
    map.emplace(g_resultName, EncodableValue());
    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onSetPlaybackSpeed(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);
        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);
        player->player->SetPlaybackRate(doubleArg(call, "speed"));
    }
  
    map.emplace(g_resultName, EncodableValue());
    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onSeekTo(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);
        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);
        player->player->SetSeek(intArg(call, "position"));
    }
  
    map.emplace(g_resultName, EncodableValue());
    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onGetPosition(const MethodCall & call, 
                                                    std::unique_ptr<MethodResult> & result)
{
    int64_t textureId = intArg(call, "textureId");

    std::shared_ptr<VideoPlayerContext> player;

    EncodableMap map;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);

        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            std::string errorMsg = "Couldn't find the player with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);

        int64_t position = player->player->GetCurrentPosition();
        if (position < 0) 
        {
            std::string errorMsg = "Failed to get current position with texture id: " + std::to_string(textureId);
            map.emplace(g_errorName, wrapError(errorMsg));
            result->Success(map);
            return;
        }

        EncodableMap value = 
        {
            { "textureId", textureId},
            { "position",  position}
        };

        map.emplace(g_resultName, value);
    }

    result->Success(map);
}

//******************************************************************************
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::onSetMixWithOthers(const MethodCall & /*call*/, 
                                                    std::unique_ptr<MethodResult> & result)
{
    EncodableMap map;
    map.emplace(g_resultName, EncodableValue(false));
    result->Success(map);
}

//******************************************************************************
// Notifies the completion of initializing the video player.
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::OnNotifyInitialized(int64_t textureId)
{
    std::cout << __func__ << std::endl;

    std::shared_ptr<VideoPlayerContext> player;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);

        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            return;
        }

        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);

        if (player->eventSink)
        {
            EncodableMap value = 
            {
                { EncodableValue("event"),    EncodableValue("initialized")},
                { EncodableValue("duration"), EncodableValue(player->player->GetDuration())},
                { EncodableValue("width"),    EncodableValue(player->player->GetWidth())},
                { EncodableValue("height"),   EncodableValue(player->player->GetHeight())}
            };

            EncodableValue event(value);
            player->eventSink->Success(event);
        }
    }
}

//******************************************************************************
// Notifies the completion of decoding a video frame.
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::OnNotifyFrameDecoded(int64_t textureId)
{
    m_textureRegistrar->MarkTextureFrameAvailable(textureId);
}

//******************************************************************************
// Notifies the completion of playing a video.
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::OnNotifyCompleted(int64_t textureId)
{
    std::cout << __func__ << std::endl;

    std::shared_ptr<VideoPlayerContext> player;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);

        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            return;
        }
        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);

        if (player->eventSink)
        {
            EncodableMap value = 
            {
               { "event", EncodableValue("completed")}
            };

            EncodableValue event(value);
            player->eventSink->Success(event);
        }
    }
}

//******************************************************************************
// Notifies update of playing or pausing a video.
//******************************************************************************
void VideoPlayerAuroraPlugin::impl::OnNotifyPlaying(int64_t textureId, bool is_playing)
{
    std::cout << __func__ << " " << is_playing << std::endl;

    std::shared_ptr<VideoPlayerContext> player;

    {
        std::lock_guard<std::recursive_mutex> guard(m_locker);

        auto it = m_players.find(textureId);
        if (it == m_players.end()) 
        {
            return;
        }

        player = it->second;
    }

    if (player)
    {
        std::lock_guard<std::recursive_mutex> guard(player->locker);

        if (player->eventSink)
        {
            EncodableMap value = 
            {
                { "event",     EncodableValue("isPlayingStateUpdate")},
                { "isPlaying", EncodableValue(is_playing)}
            };

            EncodableValue event(value);
            player->eventSink->Success(event);
        }
    }
}

//******************************************************************************
//******************************************************************************
// static
EncodableMap VideoPlayerAuroraPlugin::impl::wrapError(const std::string & message, 
                                                         const std::string & code,
                                                         const std::string & details)
{
    EncodableMap map =    {
                                {"message", EncodableValue(message)},
                                {"code",    EncodableValue(code)},
                                {"details", EncodableValue(details)}
                            };
    return map;
}

//******************************************************************************
//******************************************************************************
// static
std::string VideoPlayerAuroraPlugin::impl::findAssetsDirectory() 
{
    char* value = getenv("XDG_DATA_DIRS");
    std::string xdgDataDirsEnv;
    if (value)
    {
        xdgDataDirsEnv = std::string(value);
    } 
    else 
    {
        xdgDataDirsEnv = "";
    }

    std::vector<std::string> dirs = {};
    if (xdgDataDirsEnv.empty()) 
    {
        dirs.push_back(std::string("~/.local/share/"));
        dirs.push_back(std::string("/usr/local/share"));
        dirs.push_back(std::string("/usr/share"));
    } 
    else 
    {
        std::string delimiter = ":";

        size_t pos = 0;
        std::string token;

        while ((pos = xdgDataDirsEnv.find(delimiter)) != std::string::npos) 
        {
            token = xdgDataDirsEnv.substr(0, pos);
            dirs.push_back(token);
            xdgDataDirsEnv.erase(0, pos + delimiter.length());
        }
    }

    char buf[1024] = {};
    readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    std::string exe_path = std::string(buf);
    size_t slash_pos = exe_path.find_last_of('/');
    std::string exe_name = exe_path.substr(slash_pos, exe_path.length()-1);

    for (auto it = dirs.begin(); it != dirs.end(); ++it)
    {
        if (std::filesystem::is_directory( *(it) + exe_name + "/flutter_assets"))
        {
            return *(it) + exe_name;
        }
    }

    return std::string();
}
