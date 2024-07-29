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


#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <push_client.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>

#include <QtCore/QObject>

#include <memory>

//******************************************************************************
//******************************************************************************
class PluginController : public QObject
{
    Q_OBJECT

public:
    explicit PluginController(flutter::PluginRegistrar * registrar, QObject *parent = nullptr);

    QString applicationId() const;
    void setApplicationId(const QString & applicationId);

    QString registrationId() const;

signals:
    void applicationIdChanged(const QString &applicationId);
    void registrationIdChanged(const QString &registrationId);
    void pushMessageReceived(const Aurora::PushNotifications::Push & push);

public slots:
    void showGui();

private slots:
    void _setRegistrationId(const QString &registrationId);

private:
    class impl;
    std::shared_ptr<impl> m_p;
};

#endif // APPLICATIONCONTROLLER_H
