import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:location_aurora/location_aurora_method_channel.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  MethodChannelLocationAurora platform = MethodChannelLocationAurora();
  const MethodChannel channel = MethodChannel('location_aurora');

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(
      channel,
      (MethodCall methodCall) async {
        return 'unknow';
      },
    );
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .setMockMethodCallHandler(channel, null);
  });

  test('getLocation', () async {
    expect(await platform.getLocation(), 'unknow');
  });
}
