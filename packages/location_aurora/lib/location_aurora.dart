import 'package:location_platform_interface/location_platform_interface.dart';

import 'location_aurora_platform_interface.dart';

class LocationAurora extends LocationPlatform {
  /// Registration in Aurora
  static void registerWith() {
    LocationPlatform.instance = LocationAurora();
  }

  /// Gets the current location of the user.
  /// Returns a [LocationData] object.
  @override
  Future<LocationData> getLocation() {
    return LocationAuroraPlatform.instance.getLocation();
  }

  /// Setting the quality, interval and filter
  @override
  Future<bool> changeSettings(
      {LocationAccuracy? accuracy, int? interval, double? distanceFilter}) {
    return LocationAuroraPlatform.instance.changeSettings(
      accuracy: accuracy,
      interval: interval,
      distanceFilter: distanceFilter,
    );
  }

  /// Verification of the granted permissions
  @override
  Future<PermissionStatus> hasPermission() {
    return LocationAuroraPlatform.instance.hasPermission();
  }

  /// Checking whether the GPS service is enabled
  @override
  Future<bool> serviceEnabled() {
    return LocationAuroraPlatform.instance.serviceEnabled();
  }
}
