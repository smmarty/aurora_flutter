/*******************************************************************************
** Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
** for details. Use of this source code is governed by a
** BSD-style license that can be found in the LICENSE file.
*******************************************************************************/


//******************************************************************************
//******************************************************************************

#include "aurora_push/plugincontroller.h"
#include "aurora_push/pluginservice.h"

#include <aurora_push/aurora_push_plugin.h>
#include <flutter/method-channel.h>

#include <QtDBus/QtDBus>
#include <QtCore/QString>

//******************************************************************************
//******************************************************************************
class AuroraPushPlugin::impl
{
    friend class AuroraPushPlugin;

    std::shared_ptr<PluginController> m_controller;
    std::shared_ptr<PluginService>    m_service;
};

//******************************************************************************
//******************************************************************************
AuroraPushPlugin::AuroraPushPlugin()
    : PluginInterface()
    , m_p(new impl)
{
    qDebug() << Q_FUNC_INFO;
}

//******************************************************************************
//******************************************************************************
void AuroraPushPlugin::RegisterWithRegistrar(PluginRegistrar &registrar)
{
    registrar.RegisterMethodChannel("friflex/aurora_push",
                                    MethodCodecType::Standard,
                                    [this](const MethodCall &call) 
                                    { 
                                        this->onMethodCall(call); 
                                    });
}

//******************************************************************************
//******************************************************************************
void AuroraPushPlugin::onMethodCall(const MethodCall & call)
{
    const auto &method = call.GetMethod();

    if (method == "Messaging#init") 
    {
        init(call);
        return;
    }

    unimplemented(call);
}

//******************************************************************************
//******************************************************************************
void AuroraPushPlugin::init(const MethodCall & call)
{
    qDebug() << Q_FUNC_INFO;

    m_p->m_service.reset(new PluginService(qApp));
    m_p->m_controller.reset(new PluginController(qApp));

    QObject::connect(m_p->m_service.get(),    &PluginService::guiRequested,
                     m_p->m_controller.get(), &PluginController::showGui);

    const std::string applicationId = call.GetArgument<Encodable::String>("applicationId");
    if (applicationId.empty())
    {
        call.SendErrorResponse("-1", "Empty application ID", nullptr);
        return;
    }

    m_p->m_controller->setApplicationId(QString::fromStdString(applicationId));

    call.SendSuccessResponse(nullptr);
}

//******************************************************************************
//******************************************************************************
void AuroraPushPlugin::unimplemented(const MethodCall &call)
{
    //  TODO(mozerrr): заменить на void SendErrorResponse(const std::string &code, const std::string &message, const Encodable &details) const;
    call.SendSuccessResponse(nullptr);
}
