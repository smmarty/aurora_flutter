/*******************************************************************************
** Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
** for details. Use of this source code is governed by a
** BSD-style license that can be found in the LICENSE file.
*******************************************************************************/

//******************************************************************************
//******************************************************************************

#ifndef FLUTTER_PLUGIN_AURORA_PUSH_PLUGIN_H
#define FLUTTER_PLUGIN_AURORA_PUSH_PLUGIN_H

#include <flutter/plugin-interface.h>
#include <aurora_push/globals.h>

#include <QtCore/QObject>

#include <memory>

//******************************************************************************
//******************************************************************************
class PLUGIN_EXPORT AuroraPushPlugin final : public PluginInterface
{
public:
    AuroraPushPlugin();
    
public:
    void RegisterWithRegistrar(PluginRegistrar &registrar) override;

private:
    void onMethodCall(const MethodCall &call);
    void init(const MethodCall &call);
    void unimplemented(const MethodCall &call);

private:
    class impl;
    std::shared_ptr<impl> m_p;
};

#endif /* FLUTTER_PLUGIN_AURORA_PUSH_PLUGIN_H */
