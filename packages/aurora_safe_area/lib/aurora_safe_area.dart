library aurora_safe_area;

import 'dart:async';
import 'dart:io';

import 'package:dbus/dbus.dart';
import 'package:device_info_plus_aurora/device_info_plus_aurora.dart';
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

  @override
  void initState() async {
    super.initState();
    _cachedAuroraPlatform = _isAurora();
    if (!_cachedAuroraPlatform) return;
    // FlutterKeyboardVisibilityAuroraPlatform.instance.onChangeHeight();
    //FlutterKeyboardVisibilityAuroraPlatform.instance.getKeyboardHeight();
    // getDeviceInfo();
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

    if (deviceModel == 'some device') {
      // la la la
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
    final orientation = mq.orientation;
    final viewInsets = mq.viewInsets;
    final padding = mq.padding;
    switch (mq.orientation) {
      case Orientation.portrait:
        // if has notch, add padding on top or bottom with keyboard
        // TODO: Handle this case.
        break;
      case Orientation.landscape:
        // if tablet skip all
        // if has notch, add padding on left or right
        // TODO: Handle this case.
        break;
    }
    const notchSize = 0.0;
    // do stuff on aurora
    return MediaQuery(
      data: mq.copyWith(
        padding: EdgeInsets.fromLTRB(
          padding.left,
          // calc if has notch
          notchSize,
          padding.right,
          padding.bottom,
        ),
        viewPadding: EdgeInsets.fromLTRB(
          padding.left,
          // calc if has notch
          notchSize,
          padding.right,
          padding.bottom,
        ),
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
