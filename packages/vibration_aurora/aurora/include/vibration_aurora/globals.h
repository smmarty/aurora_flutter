// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIBRATION_AURORA_PLUGIN_GLOBALS_H
#define FLUTTER_PLUGIN_VIBRATION_AURORA_PLUGIN_GLOBALS_H

#ifdef PLUGIN_IMPL
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define PLUGIN_EXPORT
#endif

#endif /* FLUTTER_PLUGIN_VIBRATION_AURORA_PLUGIN_GLOBALS_H */
