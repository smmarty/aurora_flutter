#include <flutter/flutter_aurora.h>
#include <flutter/flutter_compatibility_qt.h>
#include "generated_plugin_registrant.h"

int main(int argc, char *argv[]) {
    aurora::Initialize(argc, argv);
    aurora::EnableQtCompatibility();
    aurora::RegisterPlugins();
    aurora::Launch();
    return 0;
}