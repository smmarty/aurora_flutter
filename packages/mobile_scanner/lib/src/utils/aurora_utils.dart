import 'dart:io';

// ignore: avoid_classes_with_only_static_members
class AuroraUtils {
  static bool? _isAurora;
  static bool get isAurora => _isAurora ??= _getInfo();

  static bool _getInfo() {
    if (!Platform.isLinux) return false;
    try {
      return File('/etc/os-release').readAsLinesSync().contains('ID=auroraos');
    } catch (e) {
      return false;
    }
  }
}
