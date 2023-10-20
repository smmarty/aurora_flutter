// Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:async';

import 'package:aurora_push_service/src/aurora_push_exception.dart';
import 'package:aurora_push_service/src/aurora_push_message.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import 'aurora_push_platform_interface.dart';

class MethodChannelAuroraMessaging extends AuroraPushPlatform {
  @visibleForTesting
  static const channel = MethodChannel('friflex/aurora_push');

  @visibleForTesting
  static Completer<String>? initCompleter;

  static void setMethodCallHandlers() {
    WidgetsFlutterBinding.ensureInitialized();
    MethodChannelAuroraMessaging.channel.setMethodCallHandler((call) async {
      switch (call.method) {
        case 'Messaging#onMessage':
          final messageMap = Map<String, dynamic>.from(call.arguments);
          AuroraPushPlatform.onMessage
              .add(AuroraPushMessage.fromMap(messageMap));
          break;
        case 'Messaging#onReadinessChanged':
          final isAvailable = call.arguments as bool;
          if (!isAvailable) {
            initCompleter?.completeError(AuroraPushException(
              code: 'push_system_not_available',
              message: 'todo: Push system is not available',
            ));
          }
          break;
        case 'Messaging#onRegistrationError':
          // На данный момент от платформы не приходит информация о причине
          // ошибки при регистрации. Также не понятно когда оно приходит.
          final message = call.arguments.toString();
          initCompleter?.completeError(AuroraPushException(
            code: 'registration_error',
            message: message,
          ));
          break;
        case 'Messaging#applicationRegistered':
          final registrationId = call.arguments as String;
          initCompleter?.complete(registrationId);
          break;
      }
    });
  }

  @override
  Future<String> initialize({required String applicationId}) async {
    if (initCompleter != null) {
      throw AuroraPushException(
        code: 'init_completer_not_finished',
        message:
            'initCompleter not finished. You must not call initialize before finishing.',
      );
    }
    if (applicationId.isEmpty) {
      throw AuroraPushException(
        code: 'application_id_empty',
        message:
            'ApplicationId is empty. Set applicationId from Aurora Center.',
      );
    }
    await channel.invokeMethod('Messaging#init', {
      'applicationId': applicationId,
    });
    initCompleter = Completer();
    // Ждем исполнения Messaging#onReadinessChanged и Messaging#applicationRegistered
    // Также может вернуться ошибка в Messaging#onRegistrationError
    try {
      // При возникновении нетипичных ошибок Aurora не возвращает ошибки,
      // поэтому нужно ставить timeout.
      final registrationId = await initCompleter!.future.timeout(
        const Duration(seconds: 5),
      );
      // Сбрасываем initCompleter чтобы не вызывать потенциальных ошибок с
      // получением нескольких колбеков от аврора-side плагина.
      initCompleter = null;
      return registrationId;
    } on TimeoutException catch (e, s) {
      initCompleter = null;

      /// Эта ошибка может происходить по разным причинам.
      ///
      /// К сожалению, сейчас api не предоставляет ошибок по каким именно
      /// причинам не удалось получить [registrationId].
      ///
      /// Для получения логов запуска пуш сервиса запустите приложение
      /// через консоль.
      ///
      /// Troubleshoot:
      /// * Проверьте подключение телефона к Аврора Центру.
      /// * Проверьте валидность [applicationId]. Если в консоли вы
      /// видите "Can not request push notifications right now", значит
      /// проблема с невалидным applicationId.
      /// * Проверьте интернет соединение пользователя.
      /// * Проверьте соединение с Аврора Центром.
      Error.throwWithStackTrace(
        AuroraPushException(
          code: 'response_timeout',
          message:
              'initialize(applicationId: $applicationId) вернул TimeoutException',
        ),
        s,
      );
    } on Object {
      initCompleter = null;
      rethrow;
    }
  }
}
