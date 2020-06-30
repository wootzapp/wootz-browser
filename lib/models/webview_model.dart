import 'dart:collection';
import 'dart:typed_data';
import 'package:collection/collection.dart';

import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';

class WebViewModel extends ChangeNotifier {

  int _tabIndex;
  String _url;
  String _title;
  Favicon _favicon;
  double _progress;
  bool _loaded;
  bool _isDesktopMode;
  bool _isIncognitoMode;
  List<Widget> _javaScriptConsoleResults;
  List<String> _javaScriptConsoleHistory;
  List<LoadedResource> _loadedResources;
  bool _isSecure;
  int windowId;
  InAppWebViewGroupOptions options;
  InAppWebViewController webViewController;

  WebViewModel({
    tabIndex,
    url,
    title,
    favicon,
    progress = 0.0,
    loaded = false,
    isDesktopMode = false,
    isIncognitoMode = false,
    javaScriptConsoleResults,
    javaScriptConsoleHistory,
    loadedResources,
    isSecure = false,
    this.windowId,
    this.options,
    this.webViewController,
  }) {
    _tabIndex = tabIndex;
    _url = url;
    _favicon = favicon;
    _progress = progress ?? 0.0;
    _loaded = loaded ?? false;
    _isDesktopMode = isDesktopMode ?? false;
    _isIncognitoMode = isIncognitoMode ?? false;
    _javaScriptConsoleResults = javaScriptConsoleResults ?? [];
    _javaScriptConsoleHistory = javaScriptConsoleHistory ?? [];
    _loadedResources = loadedResources ?? [];
    _isSecure = isSecure ?? false;
    options = options ?? InAppWebViewGroupOptions(
      crossPlatform: InAppWebViewOptions(),
      android: AndroidInAppWebViewOptions(),
      ios: IOSInAppWebViewOptions()
    );
  }

  int get tabIndex => _tabIndex;

  set tabIndex(int value) {
    if (value != _tabIndex) {
      _tabIndex = value;
      notifyListeners();
    }
  }

  String get url => _url;

  set url(String value) {
    if (value != _url) {
      _url = value;
      notifyListeners();
    }
  }

  String get title => _title;

  set title(String value) {
    if (value != _title) {
      _title = value;
      notifyListeners();
    }
  }

  Favicon get favicon => _favicon;

  set favicon(Favicon value) {
    if (value != _favicon) {
      _favicon = value;
      notifyListeners();
    }
  }

  double get progress => _progress;

  set progress(double value) {
    if (value != _progress) {
      _progress = value;
      notifyListeners();
    }
  }

  bool get loaded => _loaded;

  set loaded(bool value) {
    if (value != _loaded) {
      _loaded = value;
      notifyListeners();
    }
  }

  bool get isDesktopMode => _isDesktopMode;

  set isDesktopMode(bool value) {
    if (value != _isDesktopMode) {
      _isDesktopMode = value;
      notifyListeners();
    }
  }

  bool get isIncognitoMode => _isIncognitoMode;

  set isIncognitoMode(bool value) {
    if (value != _isIncognitoMode) {
      _isIncognitoMode = value;
      notifyListeners();
    }
  }

  UnmodifiableListView<Widget> get javaScriptConsoleResults =>
      UnmodifiableListView(_javaScriptConsoleResults);

  setJavaScriptConsoleResults(List<Widget> value) {
    if (!IterableEquality().equals(value, _javaScriptConsoleResults)) {
      _javaScriptConsoleResults = value;
      notifyListeners();
    }
  }

  void addJavaScriptConsoleResults(Widget value) {
    _javaScriptConsoleResults.add(value);
    notifyListeners();
  }

  UnmodifiableListView<String> get javaScriptConsoleHistory =>
      UnmodifiableListView(_javaScriptConsoleHistory);

  setJavaScriptConsoleHistory(List<String> value) {
    if (!IterableEquality().equals(value, _javaScriptConsoleHistory)) {
      _javaScriptConsoleHistory = value;
      notifyListeners();
    }
  }

  void addJavaScriptConsoleHistory(String value) {
    _javaScriptConsoleHistory.add(value);
    notifyListeners();
  }

  UnmodifiableListView<LoadedResource> get loadedResources =>
      UnmodifiableListView(_loadedResources);

  setLoadedResources(List<LoadedResource> value) {
    if (!IterableEquality().equals(value, _loadedResources)) {
      _loadedResources = value;
      notifyListeners();
    }
  }

  void addLoadedResources(LoadedResource value) {
    _loadedResources.add(value);
    notifyListeners();
  }

  bool get isSecure => _isSecure;

  set isSecure(bool value) {
    if (value != _isSecure) {
      _isSecure = value;
      notifyListeners();
    }
  }

  void updateWithValue(WebViewModel webViewModel) {
    tabIndex = webViewModel.tabIndex;
    url = webViewModel.url;
    title = webViewModel.title;
    favicon = webViewModel.favicon;
    progress = webViewModel.progress;
    loaded = webViewModel.loaded;
    isDesktopMode = webViewModel.isDesktopMode;
    isIncognitoMode = webViewModel.isIncognitoMode;
    setJavaScriptConsoleResults(webViewModel._javaScriptConsoleResults.toList());
    setJavaScriptConsoleHistory(webViewModel._javaScriptConsoleHistory.toList());
    setLoadedResources(webViewModel._loadedResources.toList());
    isSecure = webViewModel.isSecure;
    options = webViewModel.options;
    webViewController = webViewModel.webViewController;
  }

  static WebViewModel fromMap(Map<String, dynamic> map) {
    return map != null ? WebViewModel(
        tabIndex: map["tabIndex"],
        url: map["url"],
        title: map["title"],
        favicon: map["favicon"] != null ? Favicon(
          url: map["favicon"]["url"],
          rel: map["favicon"]["rel"],
          width: map["favicon"]["width"],
          height: map["favicon"]["height"],
        ) : null,
        progress: map["progress"],
        isDesktopMode: map["isDesktopMode"],
        isIncognitoMode: map["isIncognitoMode"],
        javaScriptConsoleHistory: map["javaScriptConsoleHistory"]?.cast<String>(),
        isSecure: map["isSecure"],
        options: InAppWebViewGroupOptions.fromMap(map["options"]),
    ) : null;
  }

  Map<String, dynamic> toMap() {
    return {
      "tabIndex": _tabIndex,
      "url": _url,
      "title": _title,
      "favicon": _favicon?.toMap(),
      "progress": _progress,
      "isDesktopMode": _isDesktopMode,
      "isIncognitoMode": _isIncognitoMode,
      "javaScriptConsoleHistory": _javaScriptConsoleHistory,
      "isSecure": _isSecure,
      "options": options?.toMap(),
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