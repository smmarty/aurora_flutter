// Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:async';

import 'package:aurora_push_service/src/aurora_push_message.dart';
import 'package:aurora_push_service/src/method_channel_messaging.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

abstract class AuroraPushPlatform extends PlatformInterface {
  /// Constructs a AuroraPushPlatform.
  AuroraPushPlatform() : super(token: _token);

  static final Object _token = Object();

  static AuroraPushPlatform? _instance;

  /// The default instance of [AuroraPushPlatform] to use.
  ///
  /// Defaults to [MethodChannelAuroraPush].
  static AuroraPushPlatform get instance {
    if (_instance == null) {
      MethodChannelAuroraMessaging.setMethodCallHandlers();
      instance = MethodChannelAuroraMessaging();
    }
    return _instance!;
  }

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [AuroraPushPlatform] when
  /// they register themselves.
  static set instance(AuroraPushPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String> initialize({required String applicationId}) {
    throw UnimplementedError('init() has not been implemented.');
  }

  static final StreamController<AuroraPushMessage> onMessage =
      StreamController<AuroraPushMessage>.broadcast();
}
