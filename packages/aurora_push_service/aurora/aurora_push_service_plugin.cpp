/*******************************************************************************
** Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
** for details. Use of this source code is governed by a
** BSD-style license that can be found in the LICENSE file.
*******************************************************************************/


//******************************************************************************
//******************************************************************************

#include <aurora_push_service/aurora_push_service_plugin.h>
#include <aurora_push_service/encodable_helper.h>
#include <aurora_push_service/plugincontroller.h>
#include <aurora_push_service/pluginservice.h>

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>

#include <QtDBus/QtDBus>
#include <QtCore/QString>

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

//******************************************************************************
//******************************************************************************
namespace Channels 
{
    const char * Methods = "friflex/aurora_push_service";
} // namespace Channels

//******************************************************************************
//******************************************************************************
class AuroraPushServicePlugin::impl
{
    friend class AuroraPushServicePlugin;

    //
    AuroraPushServicePlugin * m_o;

    //
    flutter::PluginRegistrar * m_registrar;

    // 
    std::unique_ptr<MethodChannel> m_methodChannel;

    //
    std::shared_ptr<PluginController> m_controller;
    
    //
    std::shared_ptr<PluginService>    m_service;

    //
    impl(AuroraPushServicePlugin * owner, flutter::PluginRegistrar * registrar);

    //
    void onMethodCall(const MethodCall & call,
                        std::unique_ptr<MethodResult> result);

    //
    void unimplemented(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    //
    void onInit(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);
};

//******************************************************************************
//******************************************************************************
AuroraPushServicePlugin::impl::impl(AuroraPushServicePlugin * owner, flutter::PluginRegistrar * registrar)
    : m_o(owner)
    , m_registrar(registrar)
    , m_methodChannel(new MethodChannel(registrar->messenger(), 
                                        Channels::Methods,
                                        &flutter::StandardMethodCodec::GetInstance()))
{
    // method handlers
    m_methodChannel->SetMethodCallHandler([this](const MethodCall & call, std::unique_ptr<MethodResult> result)
    {
        onMethodCall(call, std::move(result));
    });
}

//******************************************************************************
//******************************************************************************
void AuroraPushServicePlugin::RegisterWithRegistrar(flutter::PluginRegistrar * registrar) 
{
    // Create plugin
    std::unique_ptr<AuroraPushServicePlugin> plugin(new AuroraPushServicePlugin(registrar));

    // Register plugin
    registrar->AddPlugin(std::move(plugin));
}

//******************************************************************************
//******************************************************************************
AuroraPushServicePlugin::AuroraPushServicePlugin(flutter::PluginRegistrar * registrar)
    : m_p(new impl(this, registrar))
{
    qDebug() << Q_FUNC_INFO;
}

//******************************************************************************
//******************************************************************************
void AuroraPushServicePlugin::impl::onMethodCall(const MethodCall & call,
                                                std::unique_ptr<MethodResult> result)
{
    const std::string & method = call.method_name();

    std::cout << "CALL " << method << std::endl;

    if (method == "Messaging#init") 
    {
        onInit(call, result);
        return;
    }

    unimplemented(call, result);
}

//******************************************************************************
//******************************************************************************
void AuroraPushServicePlugin::impl::unimplemented(const MethodCall & /*call*/,
                                                std::unique_ptr<MethodResult> & result)
{
    std::cout << __func__ << std::endl;
    result->Success();
}

//******************************************************************************
//******************************************************************************
void AuroraPushServicePlugin::impl::onInit(const MethodCall & call,
                    std::unique_ptr<MethodResult> & result)
{
    qDebug() << Q_FUNC_INFO;

    m_service.reset(new PluginService(qApp));
    m_controller.reset(new PluginController(m_registrar, qApp));

    QObject::connect(m_service.get(),    &PluginService::guiRequested,
                     m_controller.get(), &PluginController::showGui);

    const EncodableMap params = Helper::GetValue<EncodableMap>(*call.arguments());

    const std::string applicationId = Helper::GetString(params, "applicationId");
    if (applicationId.empty())
    {
        result->Error("-1", "Empty application ID", nullptr);
        return;
    }

    m_controller->setApplicationId(QString::fromStdString(applicationId));

    result->Success(EncodableValue(nullptr));
}
