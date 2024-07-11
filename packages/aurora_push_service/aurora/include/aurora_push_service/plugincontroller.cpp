/*******************************************************************************
**
** Copyright (C) 2021-2022 Open Mobile Platform LLC.
** Contact: https://community.omprussia.ru/open-source
**
** This file is part of the Push Receiver project.
**
** Redistribution and use in source and binary forms,
** with or without modification, are permitted provided
** that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice,
**   this list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer
**   in the documentation and/or other materials provided with the distribution.
** * Neither the name of the copyright holder nor the names of its contributors
**   may be used to endorse or promote products derived from this software
**   without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
** FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
** OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS;
** OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
** EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

/*******************************************************************************
** Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
** for details. Use of this source code is governed by a
** BSD-style license that can be found in the LICENSE file.
*******************************************************************************/

#include "plugincontroller.h"
#include "pluginservice.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
// #include <auroraapp.h>
#include <nemonotifications-qt5/notification.h>

#include <aurora_push_service/aurora_push_service_plugin.h>
#include <flutter/method-channel.h>

//******************************************************************************
//******************************************************************************
class PluginController::impl
{
    friend class PluginController;

    // owner
    PluginController * m_o;

    //
    Aurora::PushNotifications::Client * m_notificationsClient;

    //
    QString m_registrationId;

    //
    MethodChannel m_notificationsChannel;

    impl(PluginController * owner);
};

//******************************************************************************
//******************************************************************************
PluginController::impl::impl(PluginController * owner)
    : m_o(owner)
    , m_notificationsClient(new Aurora::PushNotifications::Client(owner))
    , m_notificationsChannel("friflex/aurora_push_service", MethodCodecType::Standard)
{

}

//******************************************************************************
//******************************************************************************
PluginController::PluginController(QObject *parent)
    : QObject(parent)
    , m_p(new impl(this))
{
    // Вызывается после обработки пуша.
    // Узнать что делать с этим дальше.
    connect(m_p->m_notificationsClient,
            &Aurora::PushNotifications::Client::clientInactive, 
            [this]() 
            {
                qDebug() << "Aurora::PushNotifications::Client::clientInactive";
                // qApp->quit();
            });

    // not available если не сконфигурирован демон или mdm заблокировал пуши (?)
    connect(m_p->m_notificationsClient,
            &Aurora::PushNotifications::Client::pushSystemReadinessChanged, 
            [this](bool status) 
            {
                qWarning() << "Push system is" << (status ? "available" : "not available");
                m_p->m_notificationsChannel.InvokeMethod("Messaging#onReadinessChanged", Encodable(status));
            });

    connect(m_p->m_notificationsClient,
            &Aurora::PushNotifications::Client::registrationId,
            this, 
            &PluginController::_setRegistrationId);

    // TODO(mozerrr): уточнить когда возникает
    connect(m_p->m_notificationsClient, 
            &Aurora::PushNotifications::Client::registrationError, 
            [this]() 
            {
                qWarning() << "Push system have problems with registrationId";
                m_p->m_notificationsChannel.InvokeMethod("Messaging#onRegistrationError", Encodable("Push system have problems with registrationId"));
            });

    // Обработка пушей
    connect(m_p->m_notificationsClient, 
            &Aurora::PushNotifications::Client::notifications, 
            [this](const Aurora::PushNotifications::PushList & pushList) 
            {
                for (const auto & push : pushList) 
                {
                    QJsonDocument jsonDocument = QJsonDocument::fromJson(push.data.toUtf8());
                    QString notifyType = jsonDocument.object().value("mtype").toString();

                    if (notifyType == QStringLiteral("action"))
                    {
                        continue;
                    }

                    static QVariant defaultAction = Notification::remoteAction(QStringLiteral("default"), tr("Open app"),
                                                                               PluginService::notifyDBusService(),
                                                                               PluginService::notifyDBusPath(),
                                                                               PluginService::notifyDBusIface(),
                                                                               PluginService::notifyDBusMethod());

                    qDebug() << Q_FUNC_INFO << push.title << push.message;

                    // emit pushMessageReceived(push);

                    Encodable::Map pushParams;
                    pushParams.emplace(std::make_pair(Encodable("title"), Encodable(push.title.toStdString())));
                    pushParams.emplace(std::make_pair(Encodable("message"), Encodable(push.message.toStdString())));

                    m_p->m_notificationsChannel.InvokeMethod("Messaging#onMessage", pushParams);
                }
            });
}

//******************************************************************************
//******************************************************************************
QString PluginController::applicationId() const
{
    return m_p->m_notificationsClient->applicationId();
}

//******************************************************************************
//******************************************************************************
void PluginController::setApplicationId(const QString & applicationId)
{
    qDebug() << Q_FUNC_INFO << applicationId;

    if (applicationId.isEmpty())
    {
        qWarning() << "Empty application id, ignored";
        return;
    }

    if (m_p->m_notificationsClient->applicationId() == applicationId)
    {
        return;
    }

    m_p->m_notificationsClient->setApplicationId(applicationId);
    m_p->m_notificationsClient->registrate();

    emit applicationIdChanged(applicationId);
}

//******************************************************************************
//******************************************************************************
QString PluginController::registrationId() const
{
    return m_p->m_registrationId;
}

//******************************************************************************
//******************************************************************************
void PluginController::showGui()
{
    qDebug() << Q_FUNC_INFO;
    
    // TODO show gui?

    // if (m_view) 
    // {
    //     m_view->raise();
    //     m_view->showFullScreen();
    // } 
    // else 
    // {
    //     m_view = Aurora::Application::createView();
    //     m_view->rootContext()->setContextProperty(QStringLiteral("PluginController"), this);
    //     somehow show flutter app
    //     m_view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/PushReceiver.qml")));
    //     m_view->show();
    // }
}

//******************************************************************************
//******************************************************************************
void PluginController::_setRegistrationId(const QString &registrationId)
{
    qDebug() << Q_FUNC_INFO << registrationId;

    if (registrationId == m_p->m_registrationId)
    {
        return;
    }

    m_p->m_registrationId = registrationId;

    emit registrationIdChanged(registrationId);

    m_p->m_notificationsChannel.InvokeMethod("Messaging#applicationRegistered", Encodable(registrationId.toStdString()));
}
