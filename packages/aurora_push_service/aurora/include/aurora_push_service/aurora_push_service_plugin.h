/*******************************************************************************
** Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
** for details. Use of this source code is governed by a
** BSD-style license that can be found in the LICENSE file.
*******************************************************************************/

//******************************************************************************
//******************************************************************************

#ifndef FLUTTER_PLUGIN_AURORA_PUSH_SERVICE_PLUGIN_H
#define FLUTTER_PLUGIN_AURORA_PUSH_SERVICE_PLUGIN_H

#include <aurora_push_service/globals.h>
#include <flutter/plugin_registrar.h>

//******************************************************************************
//******************************************************************************
class PLUGIN_EXPORT AuroraPushServicePlugin final : public flutter::Plugin
{
public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar * registrar);

public:
    AuroraPushServicePlugin(flutter::PluginRegistrar * registrar);
    
private:
    class impl;
    std::shared_ptr<impl> m_p;
};

#endif /* FLUTTER_PLUGIN_AURORA_PUSH_SERVICE_PLUGIN_H */