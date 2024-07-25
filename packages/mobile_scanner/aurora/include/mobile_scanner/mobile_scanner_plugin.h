// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

//******************************************************************************
//******************************************************************************

#ifndef FLUTTER_PLUGIN_MOBILE_SCANNER_PLUGIN_H
#define FLUTTER_PLUGIN_MOBILE_SCANNER_PLUGIN_H

#include <mobile_scanner/globals.h>

#include <flutter/plugin_registrar.h>

//******************************************************************************
//******************************************************************************
class PLUGIN_EXPORT MobileScannerPlugin final : public flutter::Plugin
{
public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar * registrar);

private:
    // Creates a plugin that communicates on the given channel.
    MobileScannerPlugin(flutter::PluginRegistrar * registrar);

private:
    class impl;
    std::unique_ptr<impl> m_p;
};

#endif /* FLUTTER_PLUGIN_MOBILE_SCANNER_PLUGIN_H */
