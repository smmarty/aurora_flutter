import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:aurora_push_service/aurora_push_service.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
// ignore_for_file: use_build_context_synchronously

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final pushes = <AuroraPushMessage>[];
  final _auroraPushPlugin = const AuroraPushService();
  StreamSubscription<AuroraPushMessage>? messagesSubscription;
  String registrationId = '';
  int notificationCounterId = 0;
  bool wasInitialized = false;

  Future<void> initPlatformState(BuildContext context) async {
    setState(() {
      wasInitialized = false;
    });
    try {
      registrationId = await _auroraPushPlugin.initialize(
        // TODO: добавьте ваш applicationId
        applicationId: '',
      );
      if (registrationId.isNotEmpty) setState(() {});
      messagesSubscription ??=
          _auroraPushPlugin.onMessage.listen((event) async {
        if (!mounted) return;
        setState(() {
          pushes.add(event);
        });
        final notificationPlugin = FlutterLocalNotificationsPlugin();
        await notificationPlugin.show(
          notificationCounterId++,
          "#$notificationCounterId ${event.title}",
          "${event.message}",
          null,
        );
      });
    } on Object catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text(e.toString())),
      );
    }
    if (!mounted || registrationId.isEmpty) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text('Your registration id: $registrationId'),
        action: SnackBarAction(
          label: 'Copy',
          onPressed: () async {
            await Clipboard.setData(ClipboardData(text: registrationId));
          },
        ),
      ),
    );
    setState(() {
      wasInitialized = true;
    });
  }

  @override
  void dispose() {
    messagesSubscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(useMaterial3: true),
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Column(
          children: [
            if (!wasInitialized)
              const Text(
                'Tap FAB to initialize AuroraPushService.',
              ),
            if (registrationId.isNotEmpty)
              ListTile(
                title: Text(registrationId),
                trailing: IconButton(
                  icon: const Icon(Icons.copy),
                  onPressed: () async {
                    await Clipboard.setData(
                      ClipboardData(text: registrationId),
                    );
                  },
                ),
              ),
            Expanded(
              child: ListView.builder(
                // Чтобы самые свежие пуши отображались в начале
                itemCount: pushes.length,
                itemBuilder: (context, index) {
                  final push = pushes.reversed.toList()[index];
                  return ListTile(
                    title: Text('Title: ${push.title}'),
                    subtitle: Text('Message: ${push.message}'),
                    onTap: () {
                      Navigator.push(
                        context,
                        MaterialPageRoute(
                          builder: (context) => ListTileScreen(
                            title: push.title ?? '',
                            message: push.message ?? '',
                            data: push.data ?? '',
                          ),
                        ),
                      );
                    },
                  );
                },
              ),
            ),
          ],
        ),
        floatingActionButton: Builder(
          builder: (context) {
            return FloatingActionButton(
              tooltip: 'Long tap to re-initialize',
              onPressed: () async {
                await initPlatformState(context);
              },
              child: const Icon(Icons.replay),
            );
          },
        ),
      ),
    );
  }
}

class ListTileScreen extends StatelessWidget {
  const ListTileScreen({
    super.key,
    this.title = '',
    this.message = '',
    this.data = '',
  });

  final String title;
  final String message;
  final String data;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Push info'),
      ),
      body: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          if (title.isNotEmpty)
            ListTile(
              title: Text('Title: $title'),
              onTap: () async {
                await Clipboard.setData(
                  ClipboardData(text: title),
                );
              },
            ),
          if (message.isNotEmpty)
            ListTile(
              title: Text('Message: $message'),
              onTap: () async {
                await Clipboard.setData(
                  ClipboardData(text: message),
                );
              },
            ),
          if (data.isNotEmpty)
            ListTile(
              title: Text('Data: $data'),
              onTap: () async {
                await Clipboard.setData(
                  ClipboardData(text: data),
                );
              },
            ),
        ],
      ),
    );
  }
}
