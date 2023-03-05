import 'dart:io';

import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:http/http.dart';
import 'package:path_provider/path_provider.dart';

import '../utils/alt_ens.dart';

class NotificationApi {
  static final _notification = FlutterLocalNotificationsPlugin();

  static _notificationDetails({
    String picturePath = '',
    String title = '',
    String body = '',
    bool showBigPicture = false,
  }) {
    return NotificationDetails(
      android: AndroidNotificationDetails(
        'channel id',
        'channel name',
        channelDescription: 'channel description',
        styleInformation: picturePath != null
            ? _buildBigPictureStyleInformation(
                title,
                body,
                picturePath,
                showBigPicture,
              )
            : null,
        importance: Importance.max,
      ),
      iOS: DarwinNotificationDetails(
        attachments: [
          if (picturePath != null) DarwinNotificationAttachment(picturePath)
        ],
      ),
    );
  }

  static Future _init(
      {Function(NotificationResponse response)? onclick}) async {
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

  static BigPictureStyleInformation? _buildBigPictureStyleInformation(
    String title,
    String body,
    String picturePath,
    bool showBigPicture,
  ) {
    if (picturePath == '') return null;
    final FilePathAndroidBitmap filePath = FilePathAndroidBitmap(picturePath);
    return BigPictureStyleInformation(
      showBigPicture ? filePath : const FilePathAndroidBitmap("empty"),
      largeIcon: filePath,
      contentTitle: title,
      htmlFormatContentTitle: true,
      summaryText: body,
      htmlFormatSummaryText: true,
    );
  }

  static Future showNotification({
    int id = 0,
    String title = '',
    String body = '',
    String payload = '',
    Function? onclose,
    String imageUrl = '',
    Function(NotificationResponse response)? onclick,
  }) async {
    String picturePath = await _downloadAndSavePicture(imageUrl);

    await _init(onclick: onclick);
    await _notification.show(
      id,
      title,
      body,
      _notificationDetails(
        picturePath: picturePath,
        title: title,
        body: body,
        showBigPicture: true,
      ),
      payload: payload,
    );
  }

  static Future<String> _downloadAndSavePicture(
    String url,
  ) async {
    if (url == '') return '';
    final Directory directory = await getApplicationDocumentsDirectory();
    final String filePath = '${directory.path}/${sha3(url)}';
    final Response response = await get(Uri.parse(url));
    final File file = File(filePath);
    await file.writeAsBytes(response.bodyBytes);
    return filePath;
  }
}
