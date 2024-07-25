# vibration_aurora

Плагин для вызова вибрации, т.к. на данный момент в эмбедере от Авроры нет поддержки вибрации.

Этот плагин сделан на основе [vibration](https://github.com/benjamindean/flutter_vibration). Но также он может быть
использован отдельно.

## Getting Started
### Dart
Добавьте в pubspec.yaml зависимость.
```yaml
vibration_aurora:
  hosted: https://pub-aurora.friflex.com
  version: 0.1.0
```

## Usage
```dart
await Vibration.vibrate();
```