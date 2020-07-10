import 'dart:io';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter_browser/models/webview_model.dart';
import 'package:flutter_browser/util.dart';
import 'package:flutter_downloader/flutter_downloader.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:path_provider/path_provider.dart';
import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';

import 'javascript_console_result.dart';
import 'long_press_alert_dialog.dart';
import 'models/browser_model.dart';

class WebViewTab extends StatefulWidget {
  final GlobalKey<WebViewTabState> key;

  WebViewTab({@required this.key, @required this.webViewModel}) : super(key: key);

  final WebViewModel webViewModel;

  @override
  WebViewTabState createState() => WebViewTabState();
}

class WebViewTabState extends State<WebViewTab> with WidgetsBindingObserver {
  InAppWebViewController _webViewController;
  double _opacityLevel = 0.0;
  bool _isWindowClosed = false;

  TextEditingController _httpAuthUsernameController = TextEditingController();
  TextEditingController _httpAuthPasswordController = TextEditingController();

  @override
  void initState() {
    WidgetsBinding.instance.addObserver(this);
    super.initState();
  }

  @override
  void dispose() {
    _webViewController = null;
    widget.webViewModel.webViewController = null;

    _httpAuthUsernameController?.dispose();
    _httpAuthPasswordController?.dispose();

    WidgetsBinding.instance.removeObserver(this);

    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (_webViewController != null) {
      if (state == AppLifecycleState.paused) {
        pause();
      } else {
        if (Platform.isAndroid) {
          resume();
        } else if (Platform.isIOS) {
          var currentWebViewModel = Provider.of<WebViewModel>(context, listen: false);
          if (widget.webViewModel.tabIndex == currentWebViewModel.tabIndex) {
            resumeTimers();
          } else {
            pauseTimers();
          }
        }
      }
    }
  }

  void pause() {
    if (Platform.isAndroid) {
      _webViewController?.android?.pause();
    }
    pauseTimers();
  }

  void resume() {
    if (Platform.isAndroid) {
      _webViewController?.android?.resume();
    }
    resumeTimers();
  }

  void pauseTimers() {
    _webViewController?.pauseTimers();
  }

  void resumeTimers() {
    _webViewController?.resumeTimers();
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      color: Colors.white,
      child: Opacity(
        opacity: _opacityLevel,
        child: _buildWebView(),
      )
    );
  }

  InAppWebView _buildWebView() {
    var browserModel = Provider.of<BrowserModel>(context, listen: true);
    var settings = browserModel.getSettings();
    var currentWebViewModel = Provider.of<WebViewModel>(context, listen: true);

    var initialOptions = widget.webViewModel.options;
    initialOptions.crossPlatform.debuggingEnabled = settings.debuggingEnabled;
    initialOptions.crossPlatform.useOnDownloadStart = true;
    initialOptions.crossPlatform.useOnLoadResource = true;
    initialOptions.crossPlatform.useShouldOverrideUrlLoading = true;
    initialOptions.crossPlatform.javaScriptCanOpenWindowsAutomatically = true;
    initialOptions.crossPlatform.userAgent = "Mozilla/5.0 (Linux; Android 9; LG-H870 Build/PKQ1.190522.001) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/83.0.4103.106 Mobile Safari/537.36";

    initialOptions.android.safeBrowsingEnabled = true;
    initialOptions.android.disableDefaultErrorPage = true;
    initialOptions.android.supportMultipleWindows = true;

    initialOptions.ios.allowsLinkPreview = false;
    initialOptions.ios.isFraudulentWebsiteWarningEnabled = true;

    return InAppWebView(
        initialUrl: widget.webViewModel.url,
        initialOptions: initialOptions,
        windowId: widget.webViewModel.windowId,
        onWebViewCreated: (controller) async {
          Future.delayed(const Duration(milliseconds: 150), () {
            setState(() {
              _opacityLevel = 1.0;
            });
          });
          
          _webViewController = controller;
          widget.webViewModel.webViewController = controller;

          if (Platform.isAndroid) {
            controller.android.startSafeBrowsing();
          }

          widget.webViewModel.options = await controller.getOptions();

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onLoadStart: (controller, url) async {
          widget.webViewModel.isSecure = Util.urlIsSecure(url);
          widget.webViewModel.url = url;
          widget.webViewModel.loaded = false;
          widget.webViewModel.setLoadedResources([]);
          widget.webViewModel.setJavaScriptConsoleResults([]);

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onLoadStop: (controller, url) async {
          widget.webViewModel.url = url;
          widget.webViewModel.favicon = null;
          widget.webViewModel.loaded = true;

          var sslCertificateFuture = _webViewController?.getCertificate();
          var titleFuture = _webViewController?.getTitle();
          var faviconsFuture = _webViewController?.getFavicons();

          var sslCertificate = await sslCertificateFuture;
          if (sslCertificate == null && !Util.isLocalizedContent(url)) {
            widget.webViewModel.isSecure = false;
          }

          widget.webViewModel.title = await titleFuture;

          List<Favicon> favicons = await faviconsFuture;
          if (favicons != null && favicons.isNotEmpty) {
            for (var fav in favicons) {
              if (widget.webViewModel.favicon == null) {
                widget.webViewModel.favicon = fav;
              } else {
                if ((widget.webViewModel.favicon.width == null &&
                    !widget.webViewModel.favicon.url
                        .endsWith("favicon.ico")) ||
                    (fav.width != null &&
                        widget.webViewModel.favicon.width != null &&
                        fav.width > widget.webViewModel.favicon.width)) {
                  widget.webViewModel.favicon = fav;
                }
              }
            }
          }

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onProgressChanged: (controller, progress) {
          widget.webViewModel.progress = progress / 100;

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onUpdateVisitedHistory: (controller, url, androidIsReload) async {
          var currentUrl = await _webViewController?.getUrl();
          widget.webViewModel.url = currentUrl.startsWith("data:") ? url : currentUrl;
          widget.webViewModel.title = await _webViewController?.getTitle();

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onLongPressHitTestResult: (controller, hitTestResult) async {
          if (LongPressAlertDialog.HIT_TEST_RESULT_SUPPORTED
              .contains(hitTestResult.type)) {

            var requestFocusNodeHrefResult = await _webViewController?.requestFocusNodeHref();

            showDialog(
              context: context,
              builder: (context) {
                return LongPressAlertDialog(
                  webViewModel: widget.webViewModel,
                  hitTestResult: hitTestResult,
                  requestFocusNodeHrefResult: requestFocusNodeHrefResult,
                );
              },
            );
          }
        },
        onConsoleMessage: (controller, consoleMessage) {
          Color consoleTextColor = Colors.black;
          Color consoleBackgroundColor = Colors.transparent;
          IconData consoleIconData;
          Color consoleIconColor;
          if (consoleMessage.messageLevel == ConsoleMessageLevel.ERROR) {
            consoleTextColor = Colors.red;
            consoleIconData = Icons.report_problem;
            consoleIconColor = Colors.red;
          } else if (consoleMessage.messageLevel == ConsoleMessageLevel.TIP) {
            consoleTextColor = Colors.blue;
            consoleIconData = Icons.info;
            consoleIconColor = Colors.blueAccent;
          } else if (consoleMessage.messageLevel ==
              ConsoleMessageLevel.WARNING) {
            consoleBackgroundColor = Color.fromRGBO(255, 251, 227, 1);
            consoleIconData = Icons.report_problem;
            consoleIconColor = Colors.orangeAccent;
          }

          widget.webViewModel
              .addJavaScriptConsoleResults(JavaScriptConsoleResult(
            data: consoleMessage.message,
            textColor: consoleTextColor,
            backgroundColor: consoleBackgroundColor,
            iconData: consoleIconData,
            iconColor: consoleIconColor,
          ));

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onLoadResource: (controller, resource) {
          widget.webViewModel.addLoadedResources(resource);

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        shouldOverrideUrlLoading: (controller, shouldOverrideUrlLoadingRequest) async {
          var url = shouldOverrideUrlLoadingRequest.url;
          var uri = Uri.parse(url);

          if (!["http", "https", "file",
            "chrome", "data", "javascript",
            "about"].contains(uri.scheme)) {
            if (await canLaunch(url)) {
              // Launch the App
              await launch(
                url,
              );
              // and cancel the request
              return ShouldOverrideUrlLoadingAction.CANCEL;
            }
          }

          return ShouldOverrideUrlLoadingAction.ALLOW;
        },
        onDownloadStart: (controller, url) async {
          var uri = Uri.parse(url);
          String path = uri.path;
          String fileName = path.substring(path.lastIndexOf('/') + 1);

          final taskId = await FlutterDownloader.enqueue(
            url: url,
            fileName: fileName,
            savedDir: (await getExternalStorageDirectory()).path,
            showNotification: true,
            openFileFromNotification: true,
          );
        },
        onReceivedServerTrustAuthRequest: (controller, challenge) async {
          if (challenge.iosError != null || challenge.androidError != null) {
            if (Platform.isIOS && challenge.iosError == IOSSslError.UNSPECIFIED) {
              return ServerTrustAuthResponse(action: ServerTrustAuthResponseAction.PROCEED);
            }
            widget.webViewModel.isSecure = false;
            if (isCurrentTab(currentWebViewModel)) {
              currentWebViewModel.updateWithValue(widget.webViewModel);
            }
            return ServerTrustAuthResponse(action: ServerTrustAuthResponseAction.CANCEL);
          }
          return ServerTrustAuthResponse(action: ServerTrustAuthResponseAction.PROCEED);
        },
        onLoadError: (controller, url, code, message) async {
          if (Platform.isIOS && code == -999) {
            // NSURLErrorDomain
            return;
          }

          url = url ?? widget.webViewModel.url ?? 'about:blank';

          _webViewController?.loadData(data: """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, user-scalable=no, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <style>
    ${await _webViewController?.getTRexRunnerCss()}
    </style>
    <style>
    .interstitial-wrapper {
        box-sizing: border-box;
        font-size: 1em;
        line-height: 1.6em;
        margin: 0 auto 0;
        max-width: 600px;
        width: 100%;
    }
    </style>
</head>
<body>
    ${await _webViewController?.getTRexRunnerHtml()}
    <div class="interstitial-wrapper">
      <h1>Website not available</h1>
      <p>Could not load web pages at <strong>$url</strong> because:</p>
      <p>$message</p>
    </div>
</body>
    """, baseUrl: url, androidHistoryUrl: url);

          widget.webViewModel.url = url;
          widget.webViewModel.isSecure = false;

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onTitleChanged: (controller, title) async {
          widget.webViewModel.title = title;

          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
        },
        onCreateWindow: (controller, createWindowRequest) async {
          var webViewTab = WebViewTab(
            key: GlobalKey(),
            webViewModel: WebViewModel(
              url: "about:blank",
              windowId: createWindowRequest.windowId
            ),
          );

          browserModel.addTab(
              webViewTab
          );

          return true;
        },
        onCloseWindow: (controller) {
          if (_isWindowClosed) {
            return;
          }
          _isWindowClosed = true;
          browserModel.closeTab(widget.webViewModel.tabIndex);
        },
        androidOnPermissionRequest: (InAppWebViewController controller, String origin, List<String> resources) async {
          return PermissionRequestResponse(resources: resources, action: PermissionRequestResponseAction.GRANT);
        },
        onReceivedHttpAuthRequest: (InAppWebViewController controller, HttpAuthChallenge challenge) async {
          var action = await createHttpAuthDialog(challenge);
          return HttpAuthResponse(
              username: _httpAuthUsernameController.text.trim(),
              password: _httpAuthPasswordController.text,
              action: action,
              permanentPersistence: true);
        },
    );
  }

  bool isCurrentTab(WebViewModel currentWebViewModel) {
    return currentWebViewModel.tabIndex == widget.webViewModel.tabIndex;
  }

  Future<HttpAuthResponseAction> createHttpAuthDialog(HttpAuthChallenge challenge) async {
    HttpAuthResponseAction action = HttpAuthResponseAction.CANCEL;

    await showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: Text("Login"),
          content: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: <Widget>[
              Text(challenge.protectionSpace.host),
              TextField(
                decoration: InputDecoration(
                  labelText: "Username"
                ),
                controller: _httpAuthUsernameController,
              ),
              TextField(
                decoration: InputDecoration(
                    labelText: "Password"
                ),
                controller: _httpAuthPasswordController,
                obscureText: true,
              ),
            ],
          ),
          actions: <Widget>[
            FlatButton(
              child: Text("Cancel"),
              onPressed: () {
                action = HttpAuthResponseAction.CANCEL;
                Navigator.of(context).pop();
              },
            ),
            FlatButton(
              child: Text("Ok"),
              onPressed: () {
                action = HttpAuthResponseAction.PROCEED;
                Navigator.of(context).pop();
              },
            ),
          ],
        );
      },
    );

    return action;
  }

  void onShowTab() {

  }

  void onHideTab() {

  }
}
