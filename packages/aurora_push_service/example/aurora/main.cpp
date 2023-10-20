/**
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <flutter/application.h>
#include <flutter/compatibility.h>
#include "generated_plugin_registrant.h"

int main(int argc, char *argv[]) 
{
    Application::Initialize(argc, argv);
    EnableQtCompatibility();
    RegisterPlugins();
    Application::Launch();
    return 0;
}
