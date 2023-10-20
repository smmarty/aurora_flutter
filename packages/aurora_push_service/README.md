# aurora_push_service

Сервис для получения пуш уведомлений от Aurora OS.

## Getting Started

```dart
// Call after WidgetsFlutterBinding.ensureInitialized();
final auroraPushPlugin = const AuroraPushPlugin();
final notificationPlugin = FlutterLocalNotificationsPlugin();
final registrationId = await auroraPushPlugin.initialize(
        // TODO: Add your applicationId from Aurora Center
        applicationId: '',
      );
// ···
// Send registrationId to your backend and send push message from it.
// ···
final messagesSubscription =
      auroraPushPlugin.onMessage.listen((event) async {
    await notificationPlugin.show(
      0,
      event.title,
      event.message,
      null,
      payload: event.data,
    );
  });
```