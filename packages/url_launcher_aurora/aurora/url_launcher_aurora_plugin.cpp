// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#include "encodable_helper.h"

#include <url_launcher_aurora/url_launcher_aurora_plugin.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include <RuntimeManager/IntentsInvoker>
#include <QJsonObject>

//******************************************************************************
//******************************************************************************
typedef flutter::MethodCall<flutter::EncodableValue> MethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> MethodResult;

//******************************************************************************
//******************************************************************************
std::string stringArg(const MethodCall & call, const std::string & name, const std::string & defaultValue = "")
{
    const auto args = call.arguments();

    if (Helper::TypeIs<EncodableMap>(*args)) 
    {
        const EncodableMap params = Helper::GetValue<EncodableMap>(*call.arguments());
        return Helper::GetString(params, name);
    }

    if (std::holds_alternative<std::string>(*args))
    {
        return std::get<std::string>(*args);
    }

    return defaultValue;
}

//******************************************************************************
//******************************************************************************
class UrlLauncherAuroraPlugin::impl
{
    friend class UrlLauncherAuroraPlugin;

private:
    UrlLauncherAuroraPlugin * m_o;

    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue> > m_methodChannel;

private:    
    impl(UrlLauncherAuroraPlugin * owner, flutter::PluginRegistrar * registrar);

    bool haveSupportedShema(const std::string & url);

    void onMethodCall(const MethodCall & call,
                        std::unique_ptr<MethodResult> result);
    void unimplemented(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    void onCanLaunch(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);
    void onLaunch(const MethodCall & call,
                        std::unique_ptr<MethodResult> & result);

    RuntimeManager::IntentsInvoker m_invoker;
};

//******************************************************************************
//******************************************************************************
namespace 
{
    const char * g_channelName        = "plugins.flutter.io/url_launcher";
    const char * g_launchError        = "Launch error";
    const char * g_incorrectArguments = "Incorrect arguments";
    const char * g_canLaunchMethod    = "canLaunch";
    const char * g_launchMethod       = "launch";

    const std::string g_shemas[]      = { "http:", "https:", "sms:", "tel:", "file:" };

    const QString g_hintPreferredHandler = QStringLiteral("preferredHandler");
    const QString g_intentNameStart      = QStringLiteral("Start");
    const QString g_intentNameOpenURI    = QStringLiteral("OpenURI");
}

//******************************************************************************
//******************************************************************************
UrlLauncherAuroraPlugin::impl::impl(UrlLauncherAuroraPlugin * owner, flutter::PluginRegistrar * registrar)
    : m_o(owner)
    , m_methodChannel(new flutter::MethodChannel(registrar->messenger(), 
                                                g_channelName,
                                                &flutter::StandardMethodCodec::GetInstance()))
{
    m_methodChannel->SetMethodCallHandler([this](const MethodCall & call, std::unique_ptr<MethodResult> result)
    {
        onMethodCall(call, std::move(result));
    });
}

//******************************************************************************
//******************************************************************************
UrlLauncherAuroraPlugin::UrlLauncherAuroraPlugin(flutter::PluginRegistrar * registrar)
    : m_p(new impl(this, registrar))
{

}

//******************************************************************************
//******************************************************************************
// static
void UrlLauncherAuroraPlugin::RegisterWithRegistrar(flutter::PluginRegistrar * registrar)
{
    std::unique_ptr<UrlLauncherAuroraPlugin> plugin(new UrlLauncherAuroraPlugin(registrar));
    registrar->AddPlugin(std::move(plugin));
}

//******************************************************************************
//******************************************************************************
bool UrlLauncherAuroraPlugin::impl::haveSupportedShema(const std::string & url)
{
    for (const std::string & sh : g_shemas)
    {
        if (url.size() >= sh.size() && url.substr(0, sh.size()) == sh)
        {
            return true;
        }
    }
    return false;
}

//******************************************************************************
//******************************************************************************
void UrlLauncherAuroraPlugin::impl::onCanLaunch(const MethodCall & call,
                                                std::unique_ptr<MethodResult> & result)
{
    std::string url = stringArg(call, "url", "");
    if (url.empty())
    {
        result->Error(g_incorrectArguments, "Bad url");
        return; 
    }

    result->Success(EncodableValue(haveSupportedShema(url)));
}

//******************************************************************************
//******************************************************************************
void UrlLauncherAuroraPlugin::impl::onLaunch(const MethodCall & call,
                                                std::unique_ptr<MethodResult> & result)
{
    std::string url = stringArg(call, "url", "");
    if (url.empty())
    {
        result->Error(g_incorrectArguments, "Bad url");
        return; 
    }

    QJsonObject hints;
    QJsonObject params = { {"uri", QString::fromStdString(url)}, };
    m_invoker.invoke(g_intentNameOpenURI, hints, params);

    result->Success(EncodableValue(true));
    // call.SendErrorResponse("-1", g_launchError, nullptr);
}

//******************************************************************************
//******************************************************************************
void UrlLauncherAuroraPlugin::impl::onMethodCall(const MethodCall & call,
                                                std::unique_ptr<MethodResult> result)
{
    const std::string & method = call.method_name();
    if (method == g_canLaunchMethod)
    {
        onCanLaunch(call, result);
        return;
    }
    else if (method == g_launchMethod)
    {
        onLaunch(call, result);
        return;
    }

    unimplemented(call, result);
}

//******************************************************************************
//******************************************************************************
void UrlLauncherAuroraPlugin::impl::unimplemented(const MethodCall & /*call*/,
                                                std::unique_ptr<MethodResult> & result)
{
    result->Success();
}
