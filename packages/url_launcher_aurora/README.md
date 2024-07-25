# url_launcher_aurora

Federated плагин к `url_launcher` для поддержки открытия ссылок на ОС Аврора.

## Ограничения
Данный плагин работает только с ОС Аврора 5.

## Getting Started
### Dart
Добавьте в pubspec.yaml зависимость.
```yaml
url_launcher_aurora:
  hosted: https://pub-aurora.friflex.com
  version: 0.1.0+1
```

### Platform
Для начала необходимо добавить QT-совместимость в main.cpp вашего приложения.

Сначала добавьте новый `include`.
```c++
#include <flutter/flutter_compatibility_qt.h>
```

Затем добавьте `EnableQtCompatibility()` в вашу `main` функцию.
```c++
int main(int argc, char *argv[])
{
    aurora::Initialize(argc, argv);
    aurora::EnableQtCompatibility(); // Включение "поддержки Qt" для плагинов
    aurora::RegisterPlugins();
    aurora::Launch();
    return 0;
}
```

И добавьте следующие строчки в ваш spec файл приложения.
```rpmspec
BuildRequires: pkgconfig(runtime-manager-qt5)
```