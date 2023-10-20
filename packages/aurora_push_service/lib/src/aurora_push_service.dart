// Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'package:aurora_push_service/src/aurora_push_message.dart';
import 'aurora_push_platform_interface.dart';

/// Сервис для получения пуш уведомлений от ОС Аврора и Аврора Центра.
class AuroraPushService {
  const AuroraPushService();

  /// Инициализация пуш сервиса.
  ///
  /// Можно вызывать несколько раз, но не имеет практического смысла.
  Future<String> initialize({required String applicationId}) {
    return AuroraPushPlatform.instance.initialize(applicationId: applicationId);
  }

  /// [Stream] с приходящими пушами от Аврора центра.
  ///
  /// По умолчанию они не показываются, поэтому нужно
  /// использовать [FlutterLocalNotificationsPlugin](https://gitlab.com/omprussia/flutter/flutter-plugins/-/tree/master/packages/flutter_local_notifications/flutter_local_notifications_aurora?ref_type=heads).
  Stream<AuroraPushMessage> get onMessage =>
      AuroraPushPlatform.onMessage.stream;
}
