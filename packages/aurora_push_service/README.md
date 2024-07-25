# aurora_push_service

Сервис для получения пуш уведомлений от Aurora OS.

## Ограничения
Данный плагин поддерживается только на ОС Аврора 4. На данный момент он находится в "замороженном" состоянии.
Подробнее можно узнать в этом [issue](https://github.com/smmarty/aurora_flutter/issues/5#issuecomment-2220103449).

## Getting Started
Для начала необходимо добавить QT-совместимость в main.cpp вашего приложения. 

Сначала добавьте новый `include`.
```c++
#include <flutter/compatibility.h>
```

Затем добавьте `EnableQtCompatibility()` в вашу `main` функцию.
```c++
int main(int argc, char *argv[])
{
    Application::Initialize(argc, argv);
    EnableQtCompatibility(); // Включение "поддержки Qt" для плагинов
    RegisterPlugins();
    Application::Launch();
    return 0;
}
```

Также необходимо добавить разрешение на push-уведомления.
Для этого по пути `aurora/desktop/` в файле `YOUR_APP_NAME.desktop`
добавьте в Permissions `PushNotifications`.
```desktop
[X-Application]
Permissions=PushNotifications;Internet
```

После этого можно использовать Flutter плагин.
```dart
// Call after WidgetsFlutterBinding.ensureInitialized();
final auroraPushService = const AuroraPushService();
final notificationPlugin = FlutterLocalNotificationsPlugin();
// Шаг 1: Инициализируйте AuroraPushService.
final registrationId = await auroraPushService.initialize(
        // TODO: Add your applicationId from Aurora Center
        applicationId: '',
      );
// ···
// Шаг 2: Сохраните registrationId, по нему вы будете
// отправлять пуш уведомления этому устройству.
// ···
// Шаг 3: Слушайте onMessage stream.
final messagesSubscription =
      auroraPushService.onMessage.listen((event) async {
    await notificationPlugin.show(
      0,
      event.title,
      event.message,
      null,
      payload: event.data,
    );
  });
```