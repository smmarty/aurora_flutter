// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#include "encodable_helper.h"

#include <vibration_aurora/vibration_aurora_plugin.h>

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include <unistd.h>

#include <QFeedbackEffect> 
#include <QTimer>

typedef flutter::MethodCall<flutter::EncodableValue> MethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> MethodResult;

//******************************************************************************
//******************************************************************************
namespace 
{
    const std::string g_channelName        = "friflex/vibration_aurora";

    const std::string g_vibrate            = "vibrate";
    const std::string g_cancel             = "cancel";
}

//******************************************************************************
//******************************************************************************
class VibrationAuroraPlugin::impl : public QObject
{
    Q_OBJECT;

    friend class VibrationAuroraPlugin;

private:
    // 
    impl(VibrationAuroraPlugin * owner, flutter::PluginRegistrar * registrar);

private:
    // 
    void unimplemented(const MethodCall & call, std::unique_ptr<MethodResult> & result);

    // 
    void onMethodCall(const MethodCall & call, std::unique_ptr<MethodResult> result);

private slots:
    //
    void onTimer();

    //
    void onVibroState();

private:
    //
    VibrationAuroraPlugin * m_o;

    //
    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue> > m_methodChannel;

    //
    QFeedbackHapticsEffect m_vibro;

    //
    std::vector<uint64_t> m_pattern;
    size_t                m_currentPosition;
};

//******************************************************************************
//******************************************************************************
VibrationAuroraPlugin::impl::impl(VibrationAuroraPlugin * owner, flutter::PluginRegistrar * registrar)
    : m_o(owner)
    , m_methodChannel(new flutter::MethodChannel(registrar->messenger(), 
                                                    g_channelName,
                                                    &flutter::StandardMethodCodec::GetInstance()))
{
    m_methodChannel->SetMethodCallHandler([this](const MethodCall & call, std::unique_ptr<MethodResult> result)
    {
        onMethodCall(call, std::move(result));
    });

    connect(&m_vibro, &QFeedbackHapticsEffect::stateChanged, this, &VibrationAuroraPlugin::impl::onVibroState);
}

//******************************************************************************
//******************************************************************************
VibrationAuroraPlugin::VibrationAuroraPlugin(flutter::PluginRegistrar * registrar)
  : m_p(new impl(this, registrar))
{        
}

//******************************************************************************
//******************************************************************************
VibrationAuroraPlugin::~VibrationAuroraPlugin() 
{
}

//******************************************************************************
//******************************************************************************
// static
void VibrationAuroraPlugin::RegisterWithRegistrar(flutter::PluginRegistrar * registrar)
{
    std::unique_ptr<VibrationAuroraPlugin> plugin(new VibrationAuroraPlugin(registrar));
    registrar->AddPlugin(std::move(plugin));
}

//******************************************************************************
//******************************************************************************
void VibrationAuroraPlugin::impl::unimplemented(const MethodCall & /*call*/,
                                                std::unique_ptr<MethodResult> & result) 
{
    std::cout << __func__ << std::endl;
    result->Success(nullptr);
}

//******************************************************************************
//******************************************************************************
int intArg(const MethodCall & call, const std::string & name, const int defaultValue = -1)
{
    if (Helper::TypeIs<flutter::EncodableMap>(*call.arguments())) 
    {
        const flutter::EncodableMap params = Helper::GetValue<flutter::EncodableMap>(*call.arguments());
        return Helper::GetInt(params, name);
    }

    return defaultValue;
}

//******************************************************************************
//******************************************************************************
void VibrationAuroraPlugin::impl::onMethodCall(const MethodCall & call,
                                                std::unique_ptr<MethodResult> result)
{
    const auto & method = call.method_name();

    std::cout << "CALL " << method << std::endl;;

    if (method == g_vibrate)
    {
        if (pattern)
        {
            m_pattern         = vecIntArg(call, "pattern");
            m_currentPosition = 0;

            QTimer::singleShot(m_pattern[m_currentPosition], &VibrationAuroraPlugin::impl::onTimer);

            ++m_currentPosition;            
        }

        else
        {
            int64_t duration  = intArg(call, "duration");
            int64_t amplitude = intArg(call, "amplitude");

            m_vibro.setIntensity(static_cast<double>(amplitude) / 256);
            m_vibro.setDuration(duration);

            m_vibro.start();
        }

        result->Success(nullptr);
        return;
    }

    else if (method == g_cancel)
    {
        m_vibro.stop();

        result->Success(nullptr);
        return;
    }

    unimplemented(call, result);
}

//******************************************************************************
//******************************************************************************
void VibrationAuroraPlugin::impl::onTimer()
{
    if (m_currentPosition >= m_pattern.size())
    {
        // end of pattern
        return;
    }

    m_vibro.setIntensity(static_cast<double>(amplitude) / 256);
    m_vibro.setDuration(m_pattern[m_currentPosition]);

    m_vibro.start();

    ++m_currentPosition;
}

//******************************************************************************
//******************************************************************************
void VibrationAuroraPlugin::impl::onVibroState()
{
    if (m_vibro.state() != stopped)
    {
        return;
    }

    if (m_currentPosition >= m_pattern.size())
    {
        // end of pattern
        return;
    }

    QTimer::singleShot(m_pattern[m_currentPosition], &VibrationAuroraPlugin::impl::onTimer);

    ++m_currentPosition;
}

//******************************************************************************
//******************************************************************************
#include "vibration_aurora_plugin.moc"