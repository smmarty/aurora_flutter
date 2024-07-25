// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'package:location_platform_interface/location_platform_interface.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'location_aurora_method_channel.dart';

abstract class LocationAuroraPlatform extends PlatformInterface {
  /// Constructs a LocationAuroraPlatform.
  LocationAuroraPlatform() : super(token: _token);

  static final Object _token = Object();

  static LocationAuroraPlatform _instance = MethodChannelLocationAurora();

  /// The default instance of [LocationAuroraPlatform] to use.
  ///
  /// Defaults to [MethodChannelLocationAurora].
  static LocationAuroraPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [LocationAuroraPlatform] when
  /// they register themselves.
  static set instance(LocationAuroraPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<LocationData> getLocation() {
    throw UnimplementedError('getLocation() has not been implemented.');
  }

  Future<bool> changeSettings(
      {LocationAccuracy? accuracy, int? interval, double? distanceFilter}) {
    throw UnimplementedError('changeSettings() has not been implemented.');
  }

  Future<PermissionStatus> hasPermission() {
    throw UnimplementedError('hasPermission() has not been implemented.');
  }

  Future<bool> serviceEnabled() {
    throw UnimplementedError('serviceEnabled() has not been implemented.');
  }
}
