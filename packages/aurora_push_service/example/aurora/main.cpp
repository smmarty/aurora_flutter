/**
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <flutter/flutter_aurora.h>
#include <flutter/flutter_compatibility_qt.h>
#include "generated_plugin_registrant.h"

int main(int argc, char *argv[]) 
{
    aurora::Initialize(argc, argv);
    aurora::EnableQtCompatibility();
    aurora::RegisterPlugins();
    aurora::Launch();
    return 0;
}
