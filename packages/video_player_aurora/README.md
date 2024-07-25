# video_player_aurora

Federated плагин к `video_player` для поддержки воспроизведения видео на ОС Аврора.

## Getting Started
### Dart
Добавьте в `pubspec.yaml` зависимость.
```yaml
video_player_aurora:
  hosted: https://pub-aurora.friflex.com
  version: 0.1.0
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

И добавьте следующие строчки в ваш `spec` файл приложения.
```rpmspec
%global __requires_exclude ^lib(gstreamer-1.0)\\.so.*$

BuildRequires: pkgconfig(gstreamer-1.0)
BuildRequires: pkgconfig(gstreamer-app-1.0)
```