# location_aurora

The Aurora implementation of [`location`](https://pub.dev/packages/location).

## Usage

This package is not an _endorsed_ implementation of `location`.
Therefore, you have to include `location_aurora` alongside `location` as dependencies in your `pubspec.yaml` file.


**pubspec.yaml**

```yaml
dependencies:
  location: ^5.0.3
  location_aurora:
    hosted: https://pub-aurora.friflex.com
    version: ^1.1.1
```

You need to add QT compatibility to main.cpp your app.

```main.cpp
#include <flutter/flutter_aurora.h>
#include <flutter/flutter_compatibility_qt.h>
#include "generated_plugin_registrant.h"

int main(int argc, char *argv[]) {
    aurora::Initialize(argc, argv);
    aurora::EnableQtCompatibility();
    aurora::RegisterPlugins();
    aurora::Launch();
    return 0;
}
```
Add permissions
```desktop
[X-Application]
Permissions=Location;Internet
```
Example

```dart
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:location/location.dart';

class GetLocationWidget extends StatefulWidget {
  const GetLocationWidget({super.key});

  @override
  _GetLocationState createState() => _GetLocationState();
}

class _GetLocationState extends State<GetLocationWidget> {
  final location = Location();
  bool _loading = false;
  LocationData? _location;
  String? _error;

  Future<void> _getLocation() async {
    setState(() {
      _error = null;
      _loading = true;
    });
    try {
      final locationResult = await location.getLocation();
      setState(() {
        _location = locationResult;
        _loading = false;
      });
    } on PlatformException catch (err) {
      setState(() {
        _error = err.code;
        _loading = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'Location: ${_error ?? '${_location ?? "unknown"}'}',
          style: Theme.of(context).textTheme.bodyLarge,
        ),
        Row(
          children: [
            ElevatedButton(
              onPressed: _getLocation,
              child: _loading
                  ? const CircularProgressIndicator(
                      color: Colors.white,
                    )
                  : const Text('Get'),
            )
          ],
        ),
      ],
    );
  }
}

```
