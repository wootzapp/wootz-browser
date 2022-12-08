import 'package:flutter_local_notifications/flutter_local_notifications.dart';

class NotificationApi {
  static final _notification = FlutterLocalNotificationsPlugin();

  static _notificationDetails() {
    return const NotificationDetails(
      android: AndroidNotificationDetails(
        'channel id',
        'channel name',
        channelDescription: 'channel description',
        importance: Importance.max,
      ),
      iOS: DarwinNotificationDetails(),
    );
  }

  static Future _init({Function(NotificationResponse response) onclick}) async {
    const android = AndroidInitializationSettings('@mipmap/ic_launcher');
    const iOS = DarwinInitializationSettings();
    const settings = InitializationSettings(android: android, iOS: iOS);
    await _notification.initialize(
      settings,
      onDidReceiveNotificationResponse: (payload) async {
        if (onclick != null) {
          onclick(payload);
        }
      },
    );
  }

  static closeNotification({
    int id = 0,
  }) async {
    await _notification.cancel(id);
  }

  static Future showNotification({
    int id = 0,
    String title,
    String body,
    String payload,
    Function onclose,
    Function(NotificationResponse response) onclick,
  }) async {
    await _init(onclick: onclick);
    _notification.show(
      id,
      title,
      body,
      _notificationDetails(),
      payload: payload,
    );
  }
}
