// Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'package:aurora_push_service/aurora_push_service.dart';
import 'package:fake_async/fake_async.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  late AuroraPushService pushService;

  setUp(() {
    TestWidgetsFlutterBinding.ensureInitialized();
    pushService = const AuroraPushService();
  });

  test('initialize throws application_id_empty exception', () async {
    expect(
        () => pushService.initialize(applicationId: ''),
        throwsA(AuroraPushException(
          code: 'application_id_empty',
          message:
              'ApplicationId is empty. Set applicationId from Aurora Center.',
        )));
  });

  testWidgets('initialize throws TimeoutException after 5 seconds',
      (tester) async {
    fakeAsync((async) {
      tester.binding.defaultBinaryMessenger.setMockMethodCallHandler(
          MethodChannelAuroraMessaging.channel, (message) async {
        if (message.method == 'Messaging#init') {
          return null;
        }
        return null;
      });
      const appId = 'asd';
      expect(
          () => pushService.initialize(applicationId: appId),
          throwsA(AuroraPushException(
            code: 'response_timeout',
            message:
                'initialize(applicationId: $appId) вернул TimeoutException',
          )));
      async.elapse(const Duration(seconds: 6));
    });
  });

  testWidgets(
      'initialize throws init_completer_not_finished if called when calling initialize',
      (tester) async {
    fakeAsync((async) {
      tester.binding.defaultBinaryMessenger.setMockMethodCallHandler(
          MethodChannelAuroraMessaging.channel, (message) async {
        if (message.method == 'Messaging#init') {
          return null;
        }
        return null;
      });
      const appId = 'asd';
      pushService.initialize(applicationId: appId);
      async.elapse(const Duration(seconds: 3));
      expect(
          () => pushService.initialize(applicationId: appId),
          throwsA(AuroraPushException(
            code: 'init_completer_not_finished',
            message:
                'initCompleter not finished. You must not call initialize before finishing.',
          )));
      // fix infinity test
      async.flushMicrotasks();
    });
  });
}
