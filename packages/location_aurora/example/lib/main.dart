import 'package:flutter/material.dart';
import 'package:location/location.dart';

void main(List<String> args) {
  runApp(MaterialApp(
      home: Scaffold(
    appBar: AppBar(title: const Text('Location Plugin example')),
    body: const _GetLocationWidget(),
  )));
}

class _GetLocationWidget extends StatefulWidget {
  const _GetLocationWidget();

  @override
  State<_GetLocationWidget> createState() => _GetLocationState();
}

class _GetLocationState extends State<_GetLocationWidget> {
  bool _loading = false;
  LocationData? _location;
  String? _error;

  Future<void> _getLocation() async {
    setState(() {
      _error = null;
      _loading = true;
    });
    try {
      final locationResult =
          await Location().getLocation().timeout(const Duration(seconds: 15));
      setState(() {
        _location = locationResult;
        _loading = false;
      });
    } on Object catch (error, stackTrace) {
      setState(() {
        _error = '$error\n$stackTrace';
        _loading = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.center,
        mainAxisAlignment: MainAxisAlignment.center,
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            'LocationData: ${_error ?? _result}',
            style: Theme.of(context).textTheme.bodyLarge,
          ),
          ElevatedButton(
            onPressed: _getLocation,
            child: _loading
                ? const CircularProgressIndicator()
                : const Text('Get location'),
          )
        ],
      ),
    );
  }

  String get _result {
    if (_location == null) {
      return 'unknown';
    } else {
      return '\nlatitude: ${_location?.latitude},\nlongitude: ${_location?.longitude}';
    }
  }
}
