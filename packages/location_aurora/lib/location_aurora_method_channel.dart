// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:location_platform_interface/location_platform_interface.dart';

import 'location_aurora_platform_interface.dart';

/// An implementation of [LocationAuroraPlatform] that uses method channels.
class MethodChannelLocationAurora extends LocationAuroraPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('location_aurora');

  @override
  Future<LocationData> getLocation() async {
    final data = await methodChannel
        .invokeMethod<Map<Object?, Object?>>('Location#getLocation');
    if (data != null) {
      return _encodeMap(data);
    } else {
      throw PlatformException(
        code: 'Location#getLocation',
        message: 'Failed to get location',
      );
    }
  }

  LocationData _encodeMap(data) {
    final Map<String, dynamic> encodeMap =
        (data as Map<Object?, Object?>).map((key, value) {
      final keyEncode = key.toString();
      late final Object valueEncode;
      if (key == "satelliteNumber") {
        valueEncode = int.tryParse(value.toString()) ?? 0;
      } else if (key == "provider") {
        valueEncode = value.toString();
      } else if (key == "isMock") {
        valueEncode = false;
      } else {
        valueEncode = double.tryParse(value.toString()) ?? 0.0;
      }
      return MapEntry(keyEncode, valueEncode);
    });
    return LocationData.fromMap(encodeMap);
  }

  @override
  Future<bool> changeSettings(
      {LocationAccuracy? accuracy,
      int? interval,
      double? distanceFilter}) async {
    throw UnimplementedError();
  }

  /// Since in Aurora, the geo permission is issued at the first
  /// launch of the application. Then PermissionStatus is always returned here.granted.
  @override
  Future<PermissionStatus> hasPermission() async {
    return PermissionStatus.granted;
  }

  @override
  Future<bool> serviceEnabled() async {
    throw UnimplementedError();
  }
}
