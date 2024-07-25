# mobile_scanner

Плагин на ОС Аврора для сканирования штрихкодов и qr-кодов.

Этот плагин является форком [mobile_scanner](https://pub.dev/packages/mobile_scanner).

## Ограничения
Отключена поддержка web из-за несовместимости версии dart sdk.

## Getting Started
### Dart
Добавьте в `pubspec.yaml` зависимость.
```yaml
mobile_scanner:
  hosted: https://pub-aurora.friflex.com
  version: 5.1.1+1
```

### Platform
Для поддержки на ОС Аврора надо добавить следующие строчки в `CMakeLists.txt` вашего приложения.
```cmake
pkg_search_module(GLIB IMPORTED_TARGET REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0)
pkg_check_modules(GSTREAMER-AUDIO REQUIRED IMPORTED_TARGET gstreamer-audio-1.0)
pkg_check_modules(GLES REQUIRED IMPORTED_TARGET glesv2)
```

Добавьте следующие строчки в ваш `spec` файл приложения.
```rpmspec
BuildRequires: pkgconfig(glesv2)
BuildRequires: pkgconfig(streamcamera)
```

А `yuv|ZXing|jpeg` добавьте следующим образом.

Было:
```rpmspec
%global __requires_exclude ^lib(dconf|flutter-embedder|maliit-glib|.+_platform_plugin)\\.so.*$
```
Стало:
```rpmspec
%global __requires_exclude ^lib(yuv|ZXing|jpeg|dconf|flutter-embedder|maliit-glib|.+_platform_plugin)\\.so.*$
```



