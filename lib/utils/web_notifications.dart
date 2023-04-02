import 'dart:convert';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:hive/hive.dart';

// ignore: constant_identifier_names
enum WebNotificationPermission { GRANTED, DENIED, DEFAULT }

class WebNotificationAction {
  final String action;
  final String title;
  final String icon;

  WebNotificationAction(this.action, this.title, this.icon);

  WebNotificationAction.fromJson(Map<String, dynamic> json)
      : action = json['action'],
        title = json['title'].toString(),
        icon = json['icon'];

  @override
  String toString() {
    return 'WebNotificationAction{action: $action, title: $title, icon: $icon}';
  }
}

class WebNotification {
  final int id;
  final List<WebNotificationAction> actions;
  final String badge;
  final String body;
  final dynamic data;
  final String dir;
  final String icon;
  final String image;
  final String lang;
  final bool renotify;
  final bool requireInteraction;
  final bool silent;
  final String tag;
  final int timestamp;
  final String title;
  final List<int> vibrate;

  final InAppWebViewController _webViewController;
  ScaffoldFeatureController<SnackBar, SnackBarClosedReason> snackBarController;

  WebNotification(
      this.id,
      this.actions,
      this.badge,
      this.body,
      this.data,
      this.dir,
      this.icon,
      this.image,
      this.lang,
      this.renotify,
      this.requireInteraction,
      this.silent,
      this.tag,
      this.timestamp,
      this.title,
      this.vibrate,
      this._webViewController,
      this.snackBarController);

  close() async {
    await _webViewController.evaluateJavascript(source: """
      if (window._flutter_inappweview_notifications[$id] != null) {
        window._flutter_inappweview_notifications[$id].close();
      }
    """);
  }

  dispatchClick() async {
    await _webViewController.evaluateJavascript(source: """
      if (window._flutter_inappweview_notifications[$id] != null) {
        var notification = window._flutter_inappweview_notifications[$id];
        var event = new Event('click');
        notification.dispatchEvent(event);
        if (notification.onclick != null) {
          notification.onclick(event);
        }
      }
    """);
  }

  WebNotification.fromJson(Map<String, dynamic> json, this._webViewController)
      : id = json['id'],
        actions = json['actions'] != null
            ? (json['actions'].cast<Map<String, dynamic>>()
                    as List<Map<String, dynamic>>)
                .map((e) => WebNotificationAction.fromJson(e))
                .toList()
            : null,
        badge = json['badge'],
        body = json['body'],
        data = json['data'],
        dir = json['dir'],
        icon = json['icon'],
        image = json['image'],
        lang = json['lang'],
        renotify = json['renotify'],
        requireInteraction = json['requireInteraction'],
        silent = json['silent'],
        tag = json['tag'],
        timestamp = json['timestamp'],
        title = json['title'].toString(),
        vibrate = json['vibrate']?.cast<int>();

  @override
  String toString() {
    return 'WebNotification{id: $id, actions: $actions, badge: $badge, body: $body, data: $data, dir: $dir, icon: $icon, image: $image, lang: $lang, renotify: $renotify, requireInteraction: $requireInteraction, silent: $silent, tag: $tag, timestamp: $timestamp, title: $title, vibrate: $vibrate}';
  }
}

class WebNotificationController {
  final Map<int, WebNotification> notifications = {};
  final InAppWebViewController _webViewController;

  WebNotificationController(this._webViewController);

  Future<void> requestPermission() async {
    await _webViewController.evaluateJavascript(source: """
      Notification.requestPermission();
    """);
  }

  Future<WebNotificationPermission> getPermission() async {
    final String permission =
        await _webViewController.evaluateJavascript(source: """
      Notification.permission;
    """);
    switch (permission) {
      case 'granted':
        return WebNotificationPermission.GRANTED;
      case 'denied':
        return WebNotificationPermission.DENIED;
    }
    return WebNotificationPermission.DEFAULT;
  }

  Future<void> resetPermission() async {
    await _webViewController.evaluateJavascript(source: """
      Notification._permission = 'default';
    """);
  }
}

abstract class WebNotificationPermissionDb {
  static final Map<String, String> _db = {};
  static const key = 'WebNotificationPermissionDb';

  static Future<void> loadSavedPermissions() async {
    final prefs = Hive.box(secureStorageKey);
    final json = prefs.get(key);
    if (json != null) {
      _db.addAll(jsonDecode(json).cast<String, String>());
    }
  }

  static Future<bool> clear() async {
    _db.clear();
    try {
      final prefs = Hive.box(secureStorageKey);
      await prefs.delete(key);
      return true;
    } catch (e) {
      return false;
    }
  }

  static Future<bool> savePermission(
      String host, WebNotificationPermission permission) async {
    try {
      _db[host] = permission.name.toLowerCase();
      final prefs = Hive.box(secureStorageKey);
      await prefs.put(key, jsonEncode(_db));
      return true;
    } catch (e) {
      return false;
    }
  }

  static WebNotificationPermission getPermission(String host) {
    final permission = _db[host];
    if (permission != null) {
      switch (permission) {
        case 'granted':
          return WebNotificationPermission.GRANTED;
        case 'denied':
          return WebNotificationPermission.DENIED;
      }
    }
    return null;
  }

  static Map<String, String> getPermissions() {
    return Map.from(_db);
  }
}
