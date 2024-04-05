#include <flutter/application.h>
#include <flutter/compatibility.h>
#include "generated_plugin_registrant.h"

int main(int argc, char *argv[]) {
    Application::Initialize(argc, argv);
    EnableQtCompatibility();
    RegisterPlugins();
    Application::Launch();
    return 0;
}