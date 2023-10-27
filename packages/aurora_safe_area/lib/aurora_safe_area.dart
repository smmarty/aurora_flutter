library aurora_safe_area;

import 'dart:async';
import 'dart:io';

import 'package:dbus/dbus.dart';
import 'package:device_info_plus_aurora/ru_omp_deviceinfo_features.dart';
import 'package:flutter/material.dart';
import 'package:flutter_keyboard_visibility_aurora/flutter_keyboard_visibility_aurora_platform_interface.dart';

class AuroraSafeArea extends StatefulWidget {
  const AuroraSafeArea({
    super.key,
    required this.child,
  });

  final Widget child;

  @override
  State<AuroraSafeArea> createState() => _AuroraSafeAreaState();
}

class _AuroraSafeAreaState extends State<AuroraSafeArea> {
  StreamSubscription<double>? _streamSubscription;
  double _keyboardHeight = 0;
  double _notchHeight = 0;
  final _devices = {
    'TrustPhone T1': 40.0,
  };

  @override
  void initState() async {
    super.initState();
    _cachedAuroraPlatform = _isAurora();
    if (!_cachedAuroraPlatform) return;
    // FlutterKeyboardVisibilityAuroraPlatform.instance.onChangeHeight();
    //FlutterKeyboardVisibilityAuroraPlatform.instance.getKeyboardHeight();
    getDeviceInfo();

    // use hack because we cant get height direct
    _streamSubscription = FlutterKeyboardVisibilityAuroraPlatform.instance
        .onChangeHeight()
        .listen((event) {
      if (!mounted) return;
      setState(() {
        _keyboardHeight = event;
      });
    });
  }

  Future<void> getDeviceInfo() async {
    final client = DBusClient.session();

    // Features
    final features = RuOmpDeviceinfoFeatures(client, 'ru.omp.deviceinfo',
        DBusObjectPath('/ru/omp/deviceinfo/Features'));
    final deviceModel = await features.callgetDeviceModel();
    if (_devices.containsKey(deviceModel)) {
      _notchHeight = _devices[deviceModel]!;
      if (!mounted) return;
      setState(() {});
    }
  }

  @override
  void dispose() {
    _streamSubscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    // Пропускаем если это не аврора
    if (!_cachedAuroraPlatform) return widget.child;
    final mq = MediaQuery.of(context);
    final viewInsets = mq.viewInsets;
    final padding = mq.padding;
    final orientation = mq.orientation;
    EdgeInsets? devicePadding;
    if (_notchHeight > 0) {
      switch (orientation) {
        case Orientation.portrait:
          devicePadding = EdgeInsets.fromLTRB(
            padding.left,
            _notchHeight,
            padding.right,
            _notchHeight,
          );
          break;
        case Orientation.landscape:
          devicePadding = EdgeInsets.fromLTRB(
            _notchHeight,
            padding.top,
            _notchHeight,
            padding.bottom,
          );
          break;
      }
    }

    return MediaQuery(
      data: mq.copyWith(
        padding: devicePadding,
        viewPadding: devicePadding,
        viewInsets: EdgeInsets.fromLTRB(
          viewInsets.left,
          viewInsets.top,
          viewInsets.right,
          _keyboardHeight,
        ),
      ),
      child: widget.child,
    );
  }
}

bool _cachedAuroraPlatform = false;

// Дублируем здесь, чтобы можно было собирать на чистом flutter движке
bool _isAurora() {
  if (!Platform.isLinux) return false;
  try {
    return File('/etc/os-release').readAsLinesSync().contains('ID=auroraos');
  } catch (e) {
    return false;
  }
}
