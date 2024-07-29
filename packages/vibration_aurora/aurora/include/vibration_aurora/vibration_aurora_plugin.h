// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#ifndef __VIBRATION_AURORA_PLUGIN_H__
#define __VIBRATION_AURORA_PLUGIN_H__

//******************************************************************************
//******************************************************************************
#include <flutter/plugin_registrar.h>
#include <vibration_aurora/globals.h>

#include <memory>

//******************************************************************************
//******************************************************************************
class PLUGIN_EXPORT VibrationAuroraPlugin final : public flutter::Plugin
{
public:
    VibrationAuroraPlugin(flutter::PluginRegistrar * registrar);
    virtual ~VibrationAuroraPlugin();

public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar * registrar);
  

private:
    class impl;
    std::shared_ptr<impl> m_p;
};

#endif //__VIBRATION_AURORA_PLUGIN_H__