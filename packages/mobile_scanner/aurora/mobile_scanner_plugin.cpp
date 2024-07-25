// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#include <mobile_scanner/mobile_scanner_plugin.h>
#include <mobile_scanner/texture_camera.h>

#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

//******************************************************************************
//******************************************************************************

// Flutter encodable
typedef flutter::EncodableValue EncodableValue;
typedef flutter::EncodableMap   EncodableMap;
typedef flutter::EncodableList  EncodableList;
// Flutter methods
typedef flutter::MethodChannel<EncodableValue> MethodChannel;
typedef flutter::MethodCall<EncodableValue>    MethodCall;
typedef flutter::MethodResult<EncodableValue>  MethodResult;
typedef flutter::EventChannel<EncodableValue>  EventChannel;
typedef flutter::EventSink<EncodableValue>     EventSink;

//******************************************************************************
//******************************************************************************
namespace Channels 
{
    const char * Methods = "dev.steenbakker.mobile_scanner/scanner/method";
    const char * Events  = "dev.steenbakker.mobile_scanner/scanner/event";
    const char * Errors  = "dev.steenbakker.mobile_scanner/scanner/error";
} // namespace Channels

//******************************************************************************
//******************************************************************************
namespace Methods 
{
    const char * State            = "state";
    const char * Request          = "request";
    const char * ResetScale       = "resetScale";
    const char * SetScale         = "setScale";
    const char * Start            = "start";
    const char * Stop             = "stop";
    const char * ToggleTorch      = "toggleTorch";
    const char * UpdateScanWindow = "updateScanWindow";
} // namespace Methods

//******************************************************************************
//******************************************************************************
class MobileScannerPlugin::impl
{
    friend class MobileScannerPlugin;

    //
    MobileScannerPlugin * m_o;

    // 
    std::unique_ptr<MethodChannel> m_methodChannel;
    std::unique_ptr<EventChannel>  m_eventChannel; 
    std::unique_ptr<EventChannel>  m_errorChannel; 

    std::unique_ptr<EventSink>     m_eventSink;
    std::unique_ptr<EventSink>     m_errorSink;

    std::unique_ptr<TextureCamera> m_textureCamera;

    //
    impl(MobileScannerPlugin * owner, flutter::PluginRegistrar * registrar);

    //
    void onBarcoreCaptured(const std::string & data);

    //
    void onMethodCall(const MethodCall & call,
                        std::unique_ptr<MethodResult> result);
    
    //
    void unimplemented(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onCameraAuthState(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onRequestPermissions(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onResetScale(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onSetScale(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onStart(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onStop(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onToggleTorch(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onUpdateScanWindow(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);
};

//******************************************************************************
//******************************************************************************
MobileScannerPlugin::impl::impl(MobileScannerPlugin * owner, flutter::PluginRegistrar * registrar)
    : m_o(owner)
    , m_methodChannel(new MethodChannel(registrar->messenger(), 
                                        Channels::Methods,
                                        &flutter::StandardMethodCodec::GetInstance()))
    , m_eventChannel(new EventChannel(registrar->messenger(),
                                        Channels::Events,
                                        &flutter::StandardMethodCodec::GetInstance()))
    , m_errorChannel(new EventChannel(registrar->messenger(),
                                        Channels::Errors,
                                        &flutter::StandardMethodCodec::GetInstance()))
{
    m_textureCamera = std::make_unique<TextureCamera>(
                                        registrar->texture_registrar(),
                                        [&]() 
                                        {
                                            std::cout << "ERROR: ";
                                            EncodableMap state = m_textureCamera->GetState();
                                            if (state.count("error"))
                                            {
                                                std::cout << std::get<std::string>(state["error"]) << std::endl;
                                            }
                                            else
                                            {
                                                std::cout << "unknown" << std::endl;
                                            }
                                            // if (m_errorChannel) 
                                            // {
                                            //     m_sinkError->Success(m_textureCamera->GetState());
                                            // }
                                        },
                                        [&](std::string data) 
                                        {
                                            onBarcoreCaptured(data);
                                        });


    // Listen change orientation
    aurora::SubscribeOrientationChanged([&](aurora::DisplayOrientation) 
                                        {
                                            // if (m_errorChannel) 
                                            // {
                                            //     m_sinkError->Success(m_textureCamera->GetState());
                                            // }
                                        });

    // method handlers
    m_methodChannel->SetMethodCallHandler([this](const MethodCall & call, std::unique_ptr<MethodResult> result)
    {
        onMethodCall(call, std::move(result));
    });

    // erroe/state stream
    auto handlerError = std::make_unique<flutter::StreamHandlerFunctions<EncodableValue> >(
                                        [&](const EncodableValue *, std::unique_ptr<flutter::EventSink<EncodableValue> > && /*events*/)
                                                                        -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue> > 
                                        {
                                            // m_sinkError = std::move(events);
                                            // m_sinkError->Success(m_textureCamera->GetState());
                                            return nullptr;
                                        },
                                        [&](const EncodableValue *) -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue> > 
                                        {
                                            // m_sinkError = nullptr;
                                            return nullptr;
                                        });

    m_errorChannel->SetStreamHandler(std::move(handlerError));

    // Set stream handler Qr
    auto handlerEvent = std::make_unique<flutter::StreamHandlerFunctions<EncodableValue>>(
                                        [&](const EncodableValue *, std::unique_ptr<flutter::EventSink<EncodableValue> > && events)
                                                                       -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue> > 
                                        {
                                            std::cout << "got event sink" << std::endl;

                                            m_eventSink = std::move(events);
                                            return nullptr;
                                        },
                                        [&](const EncodableValue *) -> std::unique_ptr<flutter::StreamHandlerError<EncodableValue> > 
                                        {
                                            std::cout << "reset event sink" << std::endl;

                                            m_eventSink = nullptr;
                                            m_textureCamera->EnableSearchQr(false);
                                            return nullptr;
                                        });

    m_eventChannel->SetStreamHandler(std::move(handlerEvent));

}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::RegisterWithRegistrar(flutter::PluginRegistrar * registrar) 
{
    // Create plugin
    std::unique_ptr<MobileScannerPlugin> plugin(new MobileScannerPlugin(registrar));

    // Register plugin
    registrar->AddPlugin(std::move(plugin));
}

//******************************************************************************
//******************************************************************************
MobileScannerPlugin::MobileScannerPlugin(flutter::PluginRegistrar * registrar)
    : m_p(new impl(this, registrar))
{
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onBarcoreCaptured(const std::string & data)
{
    if (data.empty())
    {
        return;
    }

    std::cout << "DATA: " << data << std::endl;

    if (!m_eventSink) 
    {
        std::cout << "no sink, skipped" << std::endl;
        return;
    }

    EncodableMap barcode { {"rawValue", data} };

    EncodableList list;
    list.emplace_back(EncodableValue(barcode));

    EncodableMap event { {"name", "barcode"},
                         {"data", list} };

    m_eventSink->Success(EncodableValue(event));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onMethodCall(const MethodCall & call,
                                                std::unique_ptr<MethodResult> result)
{
    const std::string & method = call.method_name();

    std::cout << "CALL " << method << std::endl;

    if (method == Methods::State)
    {
        onCameraAuthState(call, result);
        return;
    }
    else if (method == Methods::Request)
    {
        onRequestPermissions(call, result);
        return;
    }
    else if (method == Methods::ResetScale)
    {
        onResetScale(call, result);
        return;
    }
    else if (method == Methods::SetScale)
    {
        onSetScale(call, result);
        return;
    }
    else if (method == Methods::Start)
    {
        onStart(call, result);
        return;
    }
    else if (method == Methods::Stop)
    {
        onStop(call, result);
        return;
    }
    else if (method == Methods::ToggleTorch)
    {
        onToggleTorch(call, result);
        return;
    }
    else if (method == Methods::UpdateScanWindow)
    {
        onUpdateScanWindow(call, result);
        return;
    }

    unimplemented(call, result);
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::unimplemented(const MethodCall & /*call*/,
                                                std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    result->Success();
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onCameraAuthState(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;

    // case 0: return MobileScannerAuthorizationState.undetermined;
    // case 1: return MobileScannerAuthorizationState.authorized;
    // case 2: return MobileScannerAuthorizationState.denied;

    result->Success(EncodableValue(1));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onRequestPermissions(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    result->Success(EncodableValue(true));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onResetScale(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    result->Success(EncodableValue(true));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onSetScale(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    result->Success(EncodableValue(true));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onStart(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;

    // const EncodableMap params = Helper::GetValue<EncodableMap>(*call.arguments());
    // if (params.find("cameraResolution") == params.end())
    // {
    //     result->Error("Error", "No cameraResolution param");
    //     return;
    // }

    // std::vector<int> res = Helper::GetValue<std::vector<int> >(params.at("cameraResolution"));
    // if (res.size() < 2)
    // {
    //     result->Error("Error", "cameraResolution param wrong size");
    //     return;
    // }

    // int width  = res[0];
    // int height = res[1];
 
    m_textureCamera->Unregister();

    // select first rear camera 
    std::string cameraId;
    int countOfCameras = 0;

    {
        EncodableList list = m_textureCamera->GetAvailableCameras();
        countOfCameras = list.size();
        for (const EncodableValue & v : list)
        {
            const EncodableMap m = Helper::GetValue<EncodableMap>(v);
            std::cout << Helper::GetString(m, "id") << " - " << Helper::GetString(m, "name") << std::endl;

            if (Helper::GetInt(m, "facing") == 2 && cameraId.empty())
            {
                cameraId = Helper::GetString(m, "id");
            }
        }
    }

    std::cout << "Use " << cameraId << std::endl;

    int width  = 720;
    int height = 1280;

    {
        EncodableList list = m_textureCamera->GetCameraCapabilities(cameraId);
        for (const EncodableValue & v : list)
        {
            EncodableMap m = Helper::GetValue<EncodableMap>(v);
            std::cout << m["width"].LongValue() << "x" << m["height"].LongValue() << " - " << m["fps"].LongValue() << std::endl;
        }
    }

    EncodableMap state = m_textureCamera->Register(cameraId);

    if (state.find("textureId") == state.end())
    {
        result->Error("Error", "No texture id");
        return;
    }
    
    state = m_textureCamera->StartCapture(width, height);
    m_textureCamera->EnableSearchQr(true);

    // error field always in state
    // if (state.find("error") != state.end())
    // {
    //     std::cout << "Error: " << Helper::GetString(state, "error") << std::endl;
    //     result->Success(nullptr);
    //     return;
    // }

    // 'textureId'
    // 'numberOfCameras'
    // 'currentTorchState'
    // 'size''width'
    // 'size''height'

    std::cout << "\ttextureId "         << state["textureId"].LongValue() << std::endl
              << "\tnumberOfCameras "   << countOfCameras << std::endl
              << "\tcurrentTorchState " << -1 << std::endl
              << "\twidth "             << static_cast<double>(Helper::GetInt(state, "width")) << std::endl
              << "\theight "            << static_cast<double>(Helper::GetInt(state, "height")) << std::endl;

    EncodableMap reply;
    reply["textureId"]         = state["textureId"].LongValue();
    reply["numberOfCameras"]   = countOfCameras;
    reply["currentTorchState"] = -1;

    EncodableMap size;
    size["width"]  = static_cast<double>(Helper::GetInt(state, "width"));
    size["height"] = static_cast<double>(Helper::GetInt(state, "height"));

    reply["size"] = size;

    result->Success(EncodableValue(reply));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onStop(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    m_textureCamera->Unregister();
    result->Success(EncodableValue(true));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onToggleTorch(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    result->Success(EncodableValue(true));
}

//******************************************************************************
//******************************************************************************
void MobileScannerPlugin::impl::onUpdateScanWindow(const MethodCall & /*call*/,
                        std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    result->Success(EncodableValue(true));
}
