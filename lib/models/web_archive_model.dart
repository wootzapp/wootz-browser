import 'package:flutter_inappwebview/flutter_inappwebview.dart';

class WebArchiveModel {
  String url;
  String title;
  Favicon favicon;
  String path;
  DateTime timestamp;

  WebArchiveModel({
    this.url,
    this.title,
    this.favicon,
    this.path,
    this.timestamp
  });

  static WebArchiveModel fromMap(Map<String, dynamic> map) {
    return map != null ? WebArchiveModel(
        url: map["url"],
        title: map["title"],
        path: map["path"],
        timestamp: DateTime.fromMicrosecondsSinceEpoch(map["timestamp"]),
        favicon: map["favicon"] != null ? Favicon(
          url: map["favicon"]["url"],
          rel: map["favicon"]["rel"],
          width: map["favicon"]["width"],
          height: map["favicon"]["height"],
        ) : null
    ) : null;
  }

  Map<String, dynamic> toMap() {
    return {
      "url": url,
      "title": title,
      "favicon": favicon?.toMap(),
      "path": path,
      "timestamp": timestamp.millisecondsSinceEpoch
    };
  }

  Map<String, dynamic> toJson() {
    return toMap();
  }

  @override
  String toString() {
    return toMap().toString();
  }
}