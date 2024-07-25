// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#include <vibration_aurora/vibration_aurora_plugin.h>

#include <flutter/basic-message-channel.h>
#include <flutter/encodable.h>
#include <flutter/event-channel.h>
#include <flutter/method-channel.h>
#include <flutter/plugin-registrar.h>
#include <flutter/message-codec-type.h>
#include <flutter/method-codec-type.h>
#include <unistd.h>

#include <QFeedbackEffect> 

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
class VibrationAuroraPlugin::impl
{
    friend class VibrationAuroraPlugin;

    // 
    impl();

    // 
    void unimplemented(const MethodCall & call);

    // 
    void onMethodCall(const MethodCall & call);


    QFeedbackHapticsEffect m_vibro;
};

//******************************************************************************
//******************************************************************************
VibrationAuroraPlugin::impl::impl()
{
}

//******************************************************************************
//******************************************************************************
VibrationAuroraPlugin::VibrationAuroraPlugin()
  : m_p(new impl)
{        
}

//******************************************************************************
//******************************************************************************
VibrationAuroraPlugin::~VibrationAuroraPlugin() 
{
}

//******************************************************************************
//******************************************************************************
void VibrationAuroraPlugin::RegisterWithRegistrar(PluginRegistrar & registrar) 
{
    registrar.RegisterMethodChannel(g_channelName,
                                    MethodCodecType::Standard,
                                    [this](const MethodCall & call) 
                                    { 
                                        m_p->onMethodCall(call); 
                                    });

}

//******************************************************************************
//******************************************************************************
void VibrationAuroraPlugin::impl::unimplemented(const MethodCall& call) 
{
    std::cout << __func__ << std::endl;
    call.SendSuccessResponse(nullptr);
}

//******************************************************************************
//******************************************************************************
int64_t int64Arg(const MethodCall & call, const std::string & name)
{
    return call.GetArgument<int64_t>(Encodable(name));
}

//******************************************************************************
//******************************************************************************
void VibrationAuroraPlugin::impl::onMethodCall(const MethodCall & call)
{
    const auto & method = call.GetMethod();

    std::cout << "CALL " << method << std::endl;;

    if (method == g_vibrate)
    {
        int64_t duration  = int64Arg(call, "duration");
        int64_t amplitude = int64Arg(call, "amplitude");

        m_vibro.setIntensity(static_cast<double>(amplitude) / 256);
        m_vibro.setDuration(duration);

        m_vibro.start();

        return;
    }

    else if (method == g_cancel)
    {
        m_vibro.stop();

        return;
    }

    unimplemented(call);
}
