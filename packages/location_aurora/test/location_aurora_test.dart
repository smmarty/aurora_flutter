import 'package:flutter_test/flutter_test.dart';
import 'package:location_aurora/location_aurora.dart';
import 'package:location_aurora/location_aurora_platform_interface.dart';
import 'package:location_aurora/location_aurora_method_channel.dart';
import 'package:location_platform_interface/location_platform_interface.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockLocationAuroraPlatform
    with MockPlatformInterfaceMixin
    implements LocationAuroraPlatform {
  @override
  Future<LocationData> getLocation() {
    return Future.value(LocationData.fromMap(
      {
        'latitude': 50.0,
        'longitude': 50.0,
      },
    ));
  }

  @override
  Future<bool> changeSettings(
      {LocationAccuracy? accuracy, int? interval, double? distanceFilter}) {
    // TODO: implement changeSettings
    throw UnimplementedError();
  }

  @override
  Future<PermissionStatus> hasPermission() {
    // TODO: implement hasPermission
    throw UnimplementedError();
  }

  @override
  Future<bool> serviceEnabled() {
    // TODO: implement serviceEnabled
    throw UnimplementedError();
  }
}

void main() {
  final LocationAuroraPlatform initialPlatform =
      LocationAuroraPlatform.instance;

  test('$MethodChannelLocationAurora is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelLocationAurora>());
  });

  test('getLocation', () async {
    LocationAurora locationAuroraPlugin = LocationAurora();
    MockLocationAuroraPlatform fakePlatform = MockLocationAuroraPlatform();
    LocationAuroraPlatform.instance = fakePlatform;
    final testLocation = LocationData.fromMap({
      'latitude': 50.0,
      'longitude': 50.0,
    });
    expect(await locationAuroraPlugin.getLocation(), testLocation);
  });
}
