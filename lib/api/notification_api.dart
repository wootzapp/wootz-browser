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
      iOS: IOSNotificationDetails(),
    );
  }

  static Future _init() async {
    const android = AndroidInitializationSettings('@mipmap/ic_launcher');
    const iOS = IOSInitializationSettings();
    const settings = InitializationSettings(android: android, iOS: iOS);
    await _notification.initialize(
      settings,
      onSelectNotification: (payload) async {},
    );
  }

  static Future showNotification({
    int id = 0,
    String title,
    String body,
    String payload,
  }) async {
    await _init();
    _notification.show(
      id,
      title,
      body,
      _notificationDetails(),
      payload: payload,
    );
  }
}
