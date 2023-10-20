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

#include "pluginservice.h"

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

//******************************************************************************
//******************************************************************************
const QString dbusServiceStr      = QStringLiteral(DBUS_SERVICE);
const QString dbusPathStr         = QStringLiteral(DBUS_PATH);
const QString dbusIfaceStr        = QStringLiteral(DBUS_INTERFACE);
const QString dbusMethodStr       = QStringLiteral("handleApplicationArgs");
const QString dbusMethodAppWakeUp = QStringLiteral("handleApplicationWakeUp");

//******************************************************************************
//******************************************************************************
PluginService::PluginService(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDebug() << "PluginService create";

    setAutoRelaySignals(true);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(dbusPathStr, this, QDBusConnection::ExportAllSlots);
    if (!isRegistered()) 
    {
        bool success = dbus.registerService(dbusServiceStr);
        if (!success)
        {
            // Когда здесь может произойти ошибка?
            // TODO emit error?
            // qApp->quit();
            qDebug() << Q_FUNC_INFO << "dbus.registerService failed";
        }
    }
}

//******************************************************************************
//******************************************************************************
PluginService::~PluginService()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterObject(dbusPathStr);
    dbus.unregisterService(dbusServiceStr);
}

//******************************************************************************
//******************************************************************************
bool PluginService::isRegistered()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(dbusServiceStr);
}

//******************************************************************************
//******************************************************************************
QString PluginService::notifyDBusService()
{
    return dbusServiceStr;
}

//******************************************************************************
//******************************************************************************
QString PluginService::notifyDBusPath()
{
    return dbusPathStr;
}

//******************************************************************************
//******************************************************************************
QString PluginService::notifyDBusIface()
{
    return dbusIfaceStr;
}

//******************************************************************************
//******************************************************************************
QString PluginService::notifyDBusMethod()
{
    return dbusMethodAppWakeUp;
}

//******************************************************************************
//******************************************************************************
int PluginService::updateApplicationArgs(const QStringList &arguments)
{
    qDebug() << Q_FUNC_INFO;

    QDBusMessage message = QDBusMessage::createMethodCall(dbusServiceStr, dbusPathStr, dbusIfaceStr, dbusMethodStr);
    message.setArguments(QList<QVariant>() << arguments);
    QDBusMessage reply = QDBusConnection::sessionBus().call(message);

    return 0;
}

//******************************************************************************
// уточнить статус поддержки
//******************************************************************************
void PluginService::handleApplicationArgs(const QStringList &arguments)
{
    qDebug() << Q_FUNC_INFO;

    if (arguments.indexOf(QStringLiteral("/no-gui")) != -1)
    {
        return;
    }

    // somehow show flutter app
    emit guiRequested();
}

//******************************************************************************
// уточнить статус поддержки
//******************************************************************************
void PluginService::handleApplicationWakeUp()
{
    // somehow show flutter app
    emit guiRequested();
}
