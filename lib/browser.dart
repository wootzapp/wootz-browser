import 'dart:async';
import 'dart:io';

// import 'package:cached_network_image/cached_network_image.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_browser/custom_image.dart';
import 'package:flutter_browser/tab_viewer.dart';
import 'package:flutter_browser/app_bar/browser_app_bar.dart';
import 'package:flutter_browser/models/webview_model.dart';
import 'package:flutter_browser/webview_tab.dart';
import 'package:provider/provider.dart';

import 'app_bar/tab_viewer_app_bar.dart';
import 'empty_tab.dart';
import 'models/browser_model.dart';

class Browser extends StatefulWidget {
  Browser({Key? key}) : super(key: key);

  @override
  _BrowserState createState() => _BrowserState();
}

class _BrowserState extends State<Browser> with SingleTickerProviderStateMixin {

  static const platform = const MethodChannel('com.pichillilorenzo.flutter_browser.intent_data');

  var _isRestored = false;

  @override
  void initState() {
    super.initState();
    getIntentData();
  }

  getIntentData() async {
    if (Platform.isAndroid) {
      String? url = await platform.invokeMethod("getIntentData");
      if (url != null) {
        var browserModel = Provider.of<BrowserModel>(context, listen: false);
        browserModel.addTab(WebViewTab(
          key: GlobalKey(),
          webViewModel: WebViewModel(url: Uri.parse(url)),
        ));
      }
    }
  }

  @override
  void dispose() {
    super.dispose();
  }

  restore() async {
    var browserModel = Provider.of<BrowserModel>(context, listen: true);
    browserModel.restore();
  }

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    if (!_isRestored) {
      _isRestored = true;
      restore();
    }
    precacheImage(AssetImage("assets/icon/icon.png"), context);
  }

  @override
  Widget build(BuildContext context) {
    return _buildBrowser();
  }

  Widget _buildBrowser() {
    var currentWebViewModel = Provider.of<WebViewModel>(context, listen: true);
    var browserModel = Provider.of<BrowserModel>(context, listen: true);

    browserModel.addListener(() {
      browserModel.save();
    });
    currentWebViewModel.addListener(() {
      browserModel.save();
    });

    var canShowTabScroller = browserModel.showTabScroller && browserModel.webViewTabs.isNotEmpty;

    return IndexedStack(
      index: canShowTabScroller ? 1 : 0,
      children: [
        _buildWebViewTabs(),
        canShowTabScroller ? _buildWebViewTabsViewer() : Container()
      ],
    );
  }

  Widget _buildWebViewTabs() {
    return WillPopScope(
        onWillPop: () async {
          var browserModel = Provider.of<BrowserModel>(context, listen: false);
          var webViewModel = browserModel.getCurrentTab()?.webViewModel;
          var _webViewController = webViewModel?.webViewController;

          if (_webViewController != null) {
            if (await _webViewController.canGoBack()) {
              _webViewController.goBack();
              return false;
            }
          }

          if (webViewModel != null && webViewModel.tabIndex != null) {
            setState(() {
              browserModel.closeTab(webViewModel.tabIndex!);
            });
            FocusScope.of(context).unfocus();
            return false;
          }

          return browserModel.webViewTabs.length == 0;
        },
        child: Listener(
          onPointerUp:  (_) {
            FocusScopeNode currentFocus = FocusScope.of(context);
            if (!currentFocus.hasPrimaryFocus && currentFocus.focusedChild != null) {
              currentFocus.focusedChild!.unfocus();
            }
          },
          child: Scaffold(
              appBar: BrowserAppBar(),
              body: _buildWebViewTabsContent()
          ),
        )
    );
  }

  Widget _buildWebViewTabsContent() {
    var browserModel = Provider.of<BrowserModel>(context, listen: true);

    if (browserModel.webViewTabs.length == 0) {
      return EmptyTab();
    }

    var stackChildren = <Widget>[
      IndexedStack(
        index: browserModel.getCurrentTabIndex(),
        children: browserModel.webViewTabs.map((webViewTab) {
          var isCurrentTab = webViewTab.webViewModel.tabIndex == browserModel.getCurrentTabIndex();

          if (isCurrentTab) {
            Future.delayed(const Duration(milliseconds: 100), () {
              webViewTab.key.currentState?.onShowTab();
            });
          } else {
            webViewTab.key.currentState?.onHideTab();
          }

          return webViewTab;
        }).toList(),
      ),
      _createProgressIndicator()
    ];

    return Stack(
      children: stackChildren,
    );
  }

  Widget _createProgressIndicator() {
    return Selector<WebViewModel, double>(
        selector: (context, webViewModel) => webViewModel.progress,
        builder: (context, progress, child) {
          if (progress >= 1.0) {
            return Container();
          }
          return PreferredSize(
              preferredSize: Size(double.infinity, 4.0),
              child: SizedBox(
                  height: 4.0,
                  child: LinearProgressIndicator(
                    value: progress,
                  )));
        });
  }

  Widget _buildWebViewTabsViewer() {
    var browserModel = Provider.of<BrowserModel>(context, listen: true);

    return WillPopScope(
        onWillPop: () async {
          browserModel.showTabScroller = false;
          return false;
        },
        child: Scaffold(
            appBar: TabViewerAppBar(),
            body: TabViewer(
              currentIndex: browserModel.getCurrentTabIndex(),
              children: browserModel.webViewTabs.map((webViewTab) {

                webViewTab.key.currentState?.pause();
                var screenshotData = webViewTab.webViewModel.screenshot;
                Widget screenshotImage = Container(
                  decoration: BoxDecoration(color: Colors.white),
                  width: double.infinity,
                  child: screenshotData != null ? Image.memory(screenshotData) : null,
                );

                var url = webViewTab.webViewModel.url;
                var faviconUrl = webViewTab.webViewModel.favicon != null
                    ? webViewTab.webViewModel.favicon!.url
                    : (url != null && ["http", "https"].contains(url.scheme)
                    ? Uri.parse(url.origin + "/favicon.ico")
                    : null);

                var isCurrentTab = browserModel.getCurrentTabIndex() ==
                    webViewTab.webViewModel.tabIndex;

                return Column(
                  mainAxisSize: MainAxisSize.min,
                  children: <Widget>[
                    Material(
                      color: isCurrentTab
                          ? Colors.blue
                          : (webViewTab.webViewModel.isIncognitoMode
                          ? Colors.black
                          : Colors.white),
                      child: Container(
                        child: ListTile(
                          leading: Column(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: <Widget>[
                              // CachedNetworkImage(
                              //   placeholder: (context, url) =>
                              //   url == "about:blank"
                              //       ? Container()
                              //       : CircularProgressIndicator(),
                              //   imageUrl: faviconUrl,
                              //   height: 30,
                              // )
                              CustomImage(url: faviconUrl, maxWidth: 30.0, height: 30.0)
                            ],
                          ),
                          title: Text(
                              webViewTab.webViewModel.title ??
                                  webViewTab.webViewModel.url?.toString() ?? "",
                              maxLines: 2,
                              style: TextStyle(
                                color:
                                webViewTab.webViewModel.isIncognitoMode ||
                                    isCurrentTab
                                    ? Colors.white
                                    : Colors.black,
                              ),
                              overflow: TextOverflow.ellipsis),
                          subtitle: Text(webViewTab.webViewModel.url?.toString() ?? "",
                              style: TextStyle(
                                color:
                                webViewTab.webViewModel.isIncognitoMode ||
                                    isCurrentTab
                                    ? Colors.white60
                                    : Colors.black54,
                              ),
                              maxLines: 2,
                              overflow: TextOverflow.ellipsis),
                          isThreeLine: true,
                          trailing: Row(
                            mainAxisSize: MainAxisSize.min,
                            children: <Widget>[
                              IconButton(
                                icon: Icon(
                                  Icons.close,
                                  size: 20.0,
                                  color:
                                  webViewTab.webViewModel.isIncognitoMode ||
                                      isCurrentTab
                                      ? Colors.white60
                                      : Colors.black54,
                                ),
                                onPressed: () {
                                  setState(() {
                                    if (webViewTab.webViewModel.tabIndex != null) {
                                      browserModel.closeTab(webViewTab.webViewModel.tabIndex!);
                                      if (browserModel.webViewTabs.length == 0) {
                                        browserModel.showTabScroller = false;
                                      }
                                    }
                                  });
                                },
                              )
                            ],
                          ),
                        ),
                      ),
                    ),
                    Expanded(
                      child: screenshotImage,
                    )
                  ],
                );
              }).toList(),
              onTap: (index) async {
                browserModel.showTabScroller = false;
                browserModel
                    .showTab(index);
              },
            )
        ));
  }
}
