import 'dart:collection';
import 'dart:convert';
import 'dart:io';
import 'dart:math';
import 'package:cryptowallet/screens/custom_image.dart';
import 'package:cryptowallet/screens/main_screen.dart';
import 'package:cryptowallet/screens/saved_urls.dart';
import 'package:cryptowallet/screens/security.dart';
import 'package:cryptowallet/screens/settings.dart';
import 'package:cryptowallet/screens/wallet_main_body.dart';
import 'package:cryptowallet/screens/webview_tab.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/slide_up_panel.dart';
import 'package:eth_sig_util/eth_sig_util.dart';
import 'package:eth_sig_util/util/utils.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:page_transition/page_transition.dart';
import 'package:share/share.dart';
import 'package:url_launcher/url_launcher.dart';
import 'package:web3dart/web3dart.dart';
import 'package:web3dart/web3dart.dart' as web3;
import '../utils/rpc_urls.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class Dapp extends StatefulWidget {
  final String provider;
  final String init;
  final String data;
  const Dapp({
    Key key,
    this.data,
    this.provider,
    this.init,
  }) : super(key: key);
  @override
  State<Dapp> createState() => _DappState();
}

class _DappState extends State<Dapp> {
  final browserController = TextEditingController();

  ValueNotifier loadingPercent = ValueNotifier<double>(0);
  String urlLoaded = '';
  InAppWebViewController _controller;
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  bool showWebViewTabsViewer = false;
  String initJs = '';
  List<WebViewTab> webViewTabs = [];
  int currentTabIndex = 0;
  final kHomeUrl = 'https://google.com';
  @override
  initState() {
    super.initState();

    initJs = widget.init;
    webViewTabs.add(createWebViewTab());
  }

  @override
  void dispose() {
    browserController.dispose();
    super.dispose();
  }

  WebViewTab createWebViewTab({String url, int windowId}) {
    WebViewTab webViewTab;

    if (url == null && windowId == null) {
      url = kHomeUrl;
    }

    webViewTab = WebViewTab(
      key: GlobalKey(),
      url: url,
      windowId: windowId,
      onStateUpdated: () {
        setState(() {});
      },
      onCloseTabRequested: () {
        if (webViewTab != null) {
          _closeWebViewTab(webViewTab);
        }
      },
      onCreateTabRequested: (createWindowAction) {
        _addWebViewTab(windowId: createWindowAction.windowId);
      },
    );
    return webViewTab;
  }

  AppBar _buildWebViewTabAppBar() {
    return AppBar(
      leading: IconButton(
          onPressed: () {
            _addWebViewTab();
          },
          icon: const Icon(Icons.add)),
      title: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Text(
            webViewTabs[currentTabIndex].title ?? '',
            overflow: TextOverflow.fade,
          ),
          Row(
            mainAxisSize: MainAxisSize.max,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              webViewTabs[currentTabIndex].isSecure != null
                  ? Icon(
                      webViewTabs[currentTabIndex].isSecure == true
                          ? Icons.lock
                          : Icons.lock_open,
                      color: webViewTabs[currentTabIndex].isSecure == true
                          ? Colors.green
                          : Colors.red,
                      size: 12)
                  : Container(),
              const SizedBox(
                width: 5,
              ),
              Flexible(
                  child: Text(
                webViewTabs[currentTabIndex].currentUrl ??
                    webViewTabs[currentTabIndex].url ??
                    '',
                style: const TextStyle(fontSize: 12, color: Colors.white70),
                overflow: TextOverflow.fade,
              )),
            ],
          )
        ],
      ),
      actions: _buildWebViewTabActions(),
    );
  }

  Widget _buildWebViewTabs() {
    return IndexedStack(index: currentTabIndex, children: webViewTabs);
  }

  List<Widget> _buildWebViewTabActions() {
    return [
      IconButton(
        onPressed: () async {
          await webViewTabs[currentTabIndex].updateScreenshot();
          setState(() {
            showWebViewTabsViewer = true;
          });
        },
        icon: Container(
          margin: const EdgeInsets.only(top: 5, bottom: 5),
          decoration: BoxDecoration(
              border: Border.all(width: 2.0, color: Colors.white),
              shape: BoxShape.rectangle,
              borderRadius: BorderRadius.circular(5.0)),
          constraints: const BoxConstraints(minWidth: 25.0),
          child: Center(
              child: Text(
            webViewTabs.length.toString(),
            style: const TextStyle(
                color: Colors.white,
                fontWeight: FontWeight.bold,
                fontSize: 14.0),
          )),
        ),
      ),
    ];
  }

  AppBar _buildWebViewTabViewerAppBar() {
    return AppBar(
      leading: IconButton(
          onPressed: () {
            setState(() {
              showWebViewTabsViewer = false;
            });
          },
          icon: const Icon(Icons.arrow_back)),
      title: const Text('WebView Tab Viewer'),
      actions: _buildWebViewTabsViewerActions(),
    );
  }

  Widget _buildWebViewTabsViewer() {
    return GridView.count(
      crossAxisCount: 2,
      children: webViewTabs.map((webViewTab) {
        return _buildWebViewTabGrid(webViewTab);
      }).toList(),
    );
  }

  Widget _buildWebViewTabGrid(WebViewTab webViewTab) {
    final webViewIndex = webViewTabs.indexOf(webViewTab);
    final screenshotData = webViewTab.screenshot;
    final favicon = webViewTab.favicon;

    return Card(
      clipBehavior: Clip.antiAlias,
      elevation: 2,
      shape: RoundedRectangleBorder(
          side: currentTabIndex == webViewIndex
              ? const BorderSide(
                  // border color
                  color: Colors.black,
                  // border thickness
                  width: 2)
              : BorderSide.none,
          borderRadius: const BorderRadius.all(
            Radius.circular(5),
          )),
      child: InkWell(
        onTap: () {
          _selectWebViewTab(webViewTab);
        },
        child: Column(
          children: [
            ListTile(
              tileColor: Colors.black12,
              selected: currentTabIndex == webViewIndex,
              selectedColor: Colors.white,
              selectedTileColor: Colors.black,
              contentPadding: const EdgeInsets.only(left: 10),
              visualDensity: const VisualDensity(horizontal: 0, vertical: -4),
              title: Row(mainAxisSize: MainAxisSize.max, children: [
                Container(
                  padding: const EdgeInsets.only(right: 10),
                  child: favicon != null
                      ? CustomImage(
                          url: favicon.url, maxWidth: 20.0, height: 20.0)
                      : null,
                ),
                Expanded(
                    child: Text(
                  webViewTab.title ?? '',
                  maxLines: 1,
                  overflow: TextOverflow.ellipsis,
                  style: const TextStyle(fontSize: 12),
                ))
              ]),
              trailing: IconButton(
                  onPressed: () {
                    _closeWebViewTab(webViewTab);
                  },
                  icon: const Icon(
                    Icons.close,
                    size: 16,
                  )),
            ),
            Expanded(
                child: Ink(
              decoration: screenshotData != null
                  ? BoxDecoration(
                      image: DecorationImage(
                      image: MemoryImage(screenshotData),
                      fit: BoxFit.fitWidth,
                      alignment: Alignment.topCenter,
                    ))
                  : null,
            ))
          ],
        ),
      ),
    );
  }

  List<Widget> _buildWebViewTabsViewerActions() {
    return [
      IconButton(
          onPressed: () {
            _closeAllWebViewTabs();
          },
          icon: const Icon(Icons.clear_all))
    ];
  }

  void _addWebViewTab({String url, int windowId}) {
    webViewTabs.add(createWebViewTab(url: url, windowId: windowId));
    setState(() {
      currentTabIndex = webViewTabs.length - 1;
    });
  }

  void _selectWebViewTab(WebViewTab webViewTab) {
    final webViewIndex = webViewTabs.indexOf(webViewTab);
    webViewTabs[currentTabIndex].pause();
    webViewTab.resume();
    setState(() {
      currentTabIndex = webViewIndex;
      showWebViewTabsViewer = false;
    });
  }

  void _closeWebViewTab(WebViewTab webViewTab) {
    final webViewIndex = webViewTabs.indexOf(webViewTab);
    webViewTabs.remove(webViewTab);
    if (currentTabIndex > webViewIndex) {
      currentTabIndex--;
    }
    if (webViewTabs.isEmpty) {
      webViewTabs.add(createWebViewTab());
      currentTabIndex = 0;
    }
    setState(() {
      currentTabIndex = max(0, min(webViewTabs.length - 1, currentTabIndex));
    });
  }

  void _closeAllWebViewTabs() {
    webViewTabs.clear();
    webViewTabs.add(createWebViewTab());
    setState(() {
      currentTabIndex = 0;
    });
  }

  changeBrowserChainId_(int chainId, String rpc) async {
    if (_controller == null) return;
    initJs = await changeBlockChainAndReturnInit(
      getEthereumDetailsFromChainId(chainId)['coinType'],
      chainId,
      rpc,
    );

    await _controller.removeAllUserScripts();
    await _controller.addUserScript(
      userScript: UserScript(
        source: widget.provider + initJs,
        injectionTime: UserScriptInjectionTime.AT_DOCUMENT_START,
      ),
    );
    await _controller.reload();
  }

  Future<bool> goBack() async {
    if (_controller != null && await _controller.canGoBack()) {
      _controller.goBack();
      return true;
    }
    return false;
  }

  Future<bool> goForward() async {
    if (_controller != null && await _controller.canGoForward()) {
      _controller.goForward();
      return true;
    }
    return false;
  }

  @override
  Widget build(BuildContext context) {
    final localize = AppLocalizations.of(context);
    final historyTitle = localize.history;
    final historyEmpty = localize.noHistory;
    return WillPopScope(
      onWillPop: () async {
        final bool controllerCanGoBack = await goBack();
        return !controllerCanGoBack;
      },
      child: Scaffold(
        key: _scaffoldKey,
        body: SafeArea(
          child: Container(
            color: Colors.transparent,
            height: MediaQuery.of(context).size.height,
            width: double.infinity,
            child: Column(children: [
              const SizedBox(
                height: 10,
              ),
              Row(
                children: [
                  IconButton(
                    onPressed: () {
                      if (_controller != null) {
                        _controller.loadUrl(
                          urlRequest: URLRequest(
                            url: WebUri(walletURL),
                          ),
                        );
                      }
                    },
                    icon: const Icon(Icons.home_filled),
                  ),
                  Flexible(
                    child: TextFormField(
                      onFieldSubmitted: (value) async {
                        if (_controller != null) {
                          Uri uri = blockChainToHttps(value.trim());
                          await _controller.loadUrl(
                            urlRequest: URLRequest(url: WebUri.uri(uri)),
                          );
                        }
                      },
                      textInputAction: TextInputAction.search,
                      controller: browserController,
                      decoration: InputDecoration(
                        isDense: true,
                        suffixIcon: IconButton(
                          icon: const Icon(
                            Icons.cancel,
                          ),
                          onPressed: () {
                            browserController.clear();
                          },
                        ),
                        hintText: AppLocalizations.of(context).searchOrEnterUrl,

                        filled: true,
                        focusedBorder: const OutlineInputBorder(
                            borderRadius:
                                BorderRadius.all(Radius.circular(10.0)),
                            borderSide: BorderSide.none),
                        border: const OutlineInputBorder(
                            borderRadius:
                                BorderRadius.all(Radius.circular(10.0)),
                            borderSide: BorderSide.none),
                        enabledBorder: const OutlineInputBorder(
                          borderRadius: BorderRadius.all(Radius.circular(10.0)),
                          borderSide: BorderSide.none,
                        ), // you
                      ),
                    ),
                  ),
                  const SizedBox(
                    width: 5,
                  ),
                  IconButton(
                    onPressed: () {
                      if (_controller == null) return;
                      final pref = Hive.box(secureStorageKey);
                      final bookMark = pref.get(bookMarkKey);
                      final url = browserController.text;
                      List savedBookMarks = [];

                      if (bookMark != null) {
                        savedBookMarks.addAll(
                          jsonDecode(bookMark) as List,
                        );
                      }
                      final bookMarkIndex = savedBookMarks.indexWhere(
                        (bookmark) =>
                            bookmark != null && bookmark['url'] == url,
                      );

                      bool urlBookMarked = bookMarkIndex != -1;
                      slideUpPanel(
                        context,
                        SingleChildScrollView(
                          child: Column(
                            mainAxisAlignment: MainAxisAlignment.start,
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                  onTap: () async {
                                    await goForward();
                                    Get.back();
                                  },
                                  child: Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: Row(
                                      children: [
                                        const Icon(Icons.arrow_forward),
                                        const SizedBox(
                                          width: 10,
                                        ),
                                        Text(
                                          AppLocalizations.of(context).forward,
                                        )
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                              const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                  onTap: () async {
                                    await goBack();
                                    Get.back();
                                  },
                                  child: Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: Row(
                                      children: [
                                        const Icon(Icons.arrow_back),
                                        const SizedBox(
                                          width: 10,
                                        ),
                                        Text(
                                          AppLocalizations.of(context).back,
                                        )
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                              const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                  onTap: () async {
                                    if (_controller != null) {
                                      await _controller.reload();
                                      Get.back();
                                    }
                                  },
                                  child: Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: Row(
                                      children: [
                                        const Icon(Icons.replay_outlined),
                                        const SizedBox(
                                          width: 10,
                                        ),
                                        Text(
                                          AppLocalizations.of(context).reload,
                                        )
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                              const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                  onTap: () async {
                                    if (_controller != null) {
                                      await Share.share(url);
                                      Get.back();
                                    }
                                  },
                                  child: Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: Row(
                                      children: [
                                        const Icon(Icons.share),
                                        const SizedBox(
                                          width: 10,
                                        ),
                                        Text(
                                          AppLocalizations.of(context).share,
                                        )
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                              const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                    onTap: () async {
                                      if (urlBookMarked) {
                                        savedBookMarks.removeAt(
                                          bookMarkIndex,
                                        );
                                      } else if (_controller != null) {
                                        final title =
                                            await _controller.getTitle();

                                        savedBookMarks.addAll([
                                          {'url': url, 'title': title}
                                        ]);
                                      }

                                      await pref.put(
                                        bookMarkKey,
                                        jsonEncode(
                                          savedBookMarks,
                                        ),
                                      );
                                      Get.back();
                                    },
                                    child: Padding(
                                      padding: const EdgeInsets.all(15),
                                      child: Row(
                                        children: [
                                          const Icon(
                                            Icons.bookmark,
                                          ),
                                          const SizedBox(
                                            width: 10,
                                          ),
                                          Text(
                                            urlBookMarked
                                                ? AppLocalizations.of(
                                                    context,
                                                  ).removeBookMark
                                                : AppLocalizations.of(
                                                    context,
                                                  ).bookMark,
                                          )
                                        ],
                                      ),
                                    )),
                              ),
                              const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                    onTap: () async {
                                      if (_controller != null) {
                                        await _controller.clearCache();
                                        Get.back();
                                      }
                                    },
                                    child: Container(
                                      color: Colors.transparent,
                                      child: Padding(
                                        padding: const EdgeInsets.all(15),
                                        child: Row(
                                          children: [
                                            const Icon(Icons.delete),
                                            const SizedBox(
                                              width: 10,
                                            ),
                                            Text(
                                              AppLocalizations.of(context)
                                                  .clearBrowserCache,
                                            )
                                          ],
                                        ),
                                      ),
                                    )),
                              ),
                              const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                  onTap: () async {
                                    List historyList = [];

                                    final savedHistory = pref.get(historyKey);

                                    if (savedHistory != null) {
                                      historyList =
                                          jsonDecode(savedHistory) as List;
                                    }

                                    final historyUrl = await Get.off(
                                      SavedUrls(
                                        historyTitle,
                                        historyEmpty,
                                        historyKey,
                                        data: historyList,
                                      ),
                                    );

                                    _controller.loadUrl(
                                      urlRequest: URLRequest(
                                        url: WebUri(historyUrl),
                                      ),
                                    );
                                  },
                                  child: Container(
                                    color: Colors.transparent,
                                    child: Padding(
                                      padding: const EdgeInsets.all(15),
                                      child: Row(
                                        children: [
                                          const Icon(Icons.history),
                                          const SizedBox(
                                            width: 10,
                                          ),
                                          Text(
                                            AppLocalizations.of(context)
                                                .history,
                                          )
                                        ],
                                      ),
                                    ),
                                  ),
                                ),
                              ),
                              const Divider(),
                              if (pref.get(currentMmenomicKey) != null)
                                SizedBox(
                                  width: double.infinity,
                                  child: InkWell(
                                      onTap: () {
                                        showBlockChainDialog(
                                          context: context,
                                          onTap: (blockChainData) async {
                                            await changeBrowserChainId_(
                                              blockChainData['chainId'],
                                              blockChainData['rpc'],
                                            );
                                            int count = 0;

                                            Get.until((route) => count++ == 2);
                                          },
                                          selectedChainId:
                                              pref.get(dappChainIdKey),
                                        );
                                      },
                                      child: Container(
                                        color: Colors.transparent,
                                        child: Padding(
                                          padding: const EdgeInsets.all(15),
                                          child: Row(
                                            children: [
                                              const Icon(
                                                  FontAwesomeIcons.ethereum),
                                              const SizedBox(
                                                width: 10,
                                              ),
                                              FutureBuilder(future: () async {
                                                final pref =
                                                    Hive.box(secureStorageKey);
                                                final chainId =
                                                    pref.get(dappChainIdKey);
                                                return getEthereumDetailsFromChainId(
                                                    chainId)['symbol'];
                                              }(), builder:
                                                  (context, snapshot) {
                                                return Text(
                                                  '${AppLocalizations.of(context).network}: ${snapshot.hasData ? snapshot.data : ''}',
                                                );
                                              })
                                            ],
                                          ),
                                        ),
                                      )),
                                ),
                              if (pref.get(dappChainIdKey) != null)
                                const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                  onTap: () async {
                                    Get.back();

                                    await Get.to(const Settings());
                                  },
                                  child: Container(
                                    color: Colors.transparent,
                                    child: Padding(
                                      padding: const EdgeInsets.all(15),
                                      child: Row(
                                        children: [
                                          const Icon(Icons.settings),
                                          const SizedBox(
                                            width: 10,
                                          ),
                                          Text(
                                            AppLocalizations.of(context)
                                                .settings,
                                          )
                                        ],
                                      ),
                                    ),
                                  ),
                                ),
                              ),
                              const Divider(),
                              SizedBox(
                                width: double.infinity,
                                child: InkWell(
                                    onTap: () async {
                                      final pref = Hive.box(secureStorageKey);
                                      bool hasWallet =
                                          pref.get(currentMmenomicKey) != null;

                                      bool hasPasscode =
                                          pref.get(userUnlockPasscodeKey) !=
                                              null;
                                      Widget dappWidget;
                                      Get.back();

                                      if (hasWallet) {
                                        dappWidget = const WalletMainBody();
                                      } else if (hasPasscode) {
                                        dappWidget = const MainScreen();
                                      } else {
                                        dappWidget = const Security();
                                      }
                                      await Get.to(dappWidget);
                                    },
                                    child: Container(
                                      color: Colors.transparent,
                                      child: Padding(
                                        padding: const EdgeInsets.all(15),
                                        child: Row(
                                          children: [
                                            const Icon(Icons.wallet),
                                            const SizedBox(
                                              width: 10,
                                            ),
                                            Text(
                                              AppLocalizations.of(context)
                                                  .wallet,
                                            )
                                          ],
                                        ),
                                      ),
                                    )),
                              ),
                              const Divider(),
                            ],
                          ),
                        ),
                      );

                      return;
                    },
                    icon: const Icon(
                      Icons.more_vert,
                    ),
                  )
                ],
              ),
              const SizedBox(height: 10),
              ValueListenableBuilder(
                valueListenable: loadingPercent,
                builder: (_, loadin_, __) {
                  return LinearProgressIndicator(
                    value: loadin_ == 1 ? 0 : loadin_,
                    backgroundColor: Theme.of(context).backgroundColor,
                  );
                },
              ),
              Flexible(
                child: Container(
                  color: Colors.transparent,
                  height: double.infinity,
                  width: double.infinity,
                  child: InAppWebView(
                    initialUrlRequest:
                        URLRequest(url: WebUri(widget.data ?? walletURL)),
                    initialSettings: InAppWebViewSettings(
                      useShouldOverrideUrlLoading: true,
                      forceDark: Theme.of(context).brightness == Brightness.dark
                          ? ForceDark.ON
                          : ForceDark.OFF,
                    ),
                    onPermissionRequest: (controller, request) async {
                      return PermissionResponse(
                        resources: request.resources,
                        action: PermissionResponseAction.GRANT,
                      );
                    },
                    shouldOverrideUrlLoading: (
                      InAppWebViewController controller,
                      NavigationAction shouldOverrideUrl,
                    ) async {
                      Uri url = shouldOverrideUrl.request.url;

                      List<String> allowedAction = [
                        "http",
                        "https",
                        "file",
                        "chrome",
                        "data",
                        "javascript",
                        "about"
                      ];
                      if (!allowedAction.contains(url.scheme)) {
                        try {
                          await launchUrl(url);
                        } catch (_) {}
                        return NavigationActionPolicy.CANCEL;
                      }
                      try {
                        final urlResponse =
                            await get(Uri.parse(scamDbUrl + url.host));
                        final isResponseError =
                            urlResponse.statusCode ~/ 100 == 4 ||
                                urlResponse.statusCode ~/ 100 == 5;

                        if (!isResponseError) {
                          final jsonDecoded = jsonDecode(urlResponse.body);

                          if (jsonDecoded['success'] &&
                              jsonDecoded['result']['status'] == 'blocked') {
                            PackageInfo packageInfo =
                                await PackageInfo.fromPlatform();

                            bool shouldByPass = false;

                            await slideUpPanel(
                              context,
                              SingleChildScrollView(
                                child: Padding(
                                  padding: const EdgeInsets.all(25),
                                  child: Column(children: [
                                    Text(
                                      AppLocalizations.of(context)
                                          .scamSiteDetected,
                                      style: const TextStyle(fontSize: 20),
                                    ),
                                    const SizedBox(
                                      height: 20,
                                    ),
                                    Text(
                                      AppLocalizations.of(context)
                                          .scamSiteDetectedDescription(
                                        url.host,
                                        packageInfo.appName,
                                      ),
                                      textAlign: TextAlign.center,
                                      style: const TextStyle(
                                        fontSize: 18,
                                      ),
                                    ),
                                    const SizedBox(
                                      height: 20,
                                    ),
                                    TextButton(
                                      style: TextButton.styleFrom(
                                        foregroundColor: Colors.white,
                                        backgroundColor: Colors.red,
                                      ),
                                      onPressed: () async {
                                        shouldByPass = true;
                                        Get.back();
                                      },
                                      child: Text(
                                        AppLocalizations.of(context).ignore,
                                        style: const TextStyle(
                                          fontSize: 20,
                                        ),
                                      ),
                                    ),
                                  ]),
                                ),
                              ),
                            );

                            if (shouldByPass) {
                              return NavigationActionPolicy.ALLOW;
                            }

                            return NavigationActionPolicy.CANCEL;
                          }
                        }
                      } catch (_) {}

                      return NavigationActionPolicy.ALLOW;
                    },
                    onWebViewCreated: (controller) async {
                      _controller = controller;

                      _controller.addJavaScriptHandler(
                        handlerName: 'requestAccounts',
                        callback: (args) async {
                          final pref = Hive.box(secureStorageKey);
                          final mnemonic = pref.get(currentMmenomicKey);

                          int chainId = pref.get(dappChainIdKey);
                          final blockChainDetails =
                              getEthereumDetailsFromChainId(chainId);
                          final web3Response = await getEthereumFromMemnomic(
                            mnemonic,
                            blockChainDetails['coinType'],
                          );

                          final sendingAddress =
                              web3Response['eth_wallet_address'];
                          final id = args[0];
                          try {
                            await _controller.evaluateJavascript(
                              source:
                                  'AlphaWallet.executeCallback($id, null, ["$sendingAddress"]);',
                            );
                          } catch (e) {
                            //  replace all quotes in error
                            final error = e.toString().replaceAll('"', '\'');

                            await _controller.evaluateJavascript(
                              source:
                                  'AlphaWallet.executeCallback($id, "$error",null);',
                            );
                          }
                        },
                      );
                      _controller.addJavaScriptHandler(
                        handlerName: 'walletAddEthereumChain',
                        callback: (args) async {
                          final pref = Hive.box(secureStorageKey);
                          int chainId = pref.get(dappChainIdKey);
                          final id = args[0];

                          final switchChainId =
                              BigInt.parse(json.decode(args[1])['chainId'])
                                  .toInt();

                          final currentChainIdData =
                              getEthereumDetailsFromChainId(chainId);

                          final switchChainIdData =
                              getEthereumDetailsFromChainId(switchChainId);

                          if (chainId == switchChainId) {
                            await _controller.evaluateJavascript(
                                source:
                                    'AlphaWallet.executeCallback($id, "cancelled", null);');
                            return;
                          }

                          if (switchChainIdData == null) {
                            await _controller.evaluateJavascript(
                                source:
                                    'AlphaWallet.executeCallback($id, "we can not add this block", null);');
                          } else {
                            switchEthereumChain(
                              context: context,
                              currentChainIdData: currentChainIdData,
                              switchChainIdData: switchChainIdData,
                              onConfirm: () async {
                                await changeBrowserChainId_(
                                  switchChainIdData['chainId'],
                                  switchChainIdData['rpc'],
                                );

                                await _controller.evaluateJavascript(
                                  source:
                                      'AlphaWallet.executeCallback($id, null, null);',
                                );
                                Navigator.pop(context);
                              },
                              onReject: () async {
                                await _controller.evaluateJavascript(
                                    source:
                                        'AlphaWallet.executeCallback($id, "user rejected switch", null);');
                                Navigator.pop(context);
                              },
                            );
                          }
                        },
                      );
                      _controller.addJavaScriptHandler(
                        handlerName: 'walletSwitchEthereumChain',
                        callback: (args) async {
                          final pref = Hive.box(secureStorageKey);
                          int chainId = pref.get(dappChainIdKey);
                          final id = args[0];

                          final switchChainId =
                              BigInt.parse(json.decode(args[1])['chainId'])
                                  .toInt();

                          final currentChainIdData =
                              getEthereumDetailsFromChainId(chainId);

                          final switchChainIdData =
                              getEthereumDetailsFromChainId(switchChainId);

                          if (chainId == switchChainId) {
                            await _controller.evaluateJavascript(
                              source:
                                  'AlphaWallet.executeCallback($id, "cancelled", null);',
                            );
                            return;
                          }

                          if (switchChainIdData == null) {
                            await _controller.evaluateJavascript(
                              source:
                                  'AlphaWallet.executeCallback($id, "we can not add this block", null);',
                            );
                          } else {
                            switchEthereumChain(
                              context: context,
                              currentChainIdData: currentChainIdData,
                              switchChainIdData: switchChainIdData,
                              onConfirm: () async {
                                await changeBrowserChainId_(
                                  switchChainIdData['chainId'],
                                  switchChainIdData['rpc'],
                                );

                                await _controller.evaluateJavascript(
                                  source:
                                      'AlphaWallet.executeCallback($id, null, null);',
                                );
                                Get.back();
                              },
                              onReject: () async {
                                await _controller.evaluateJavascript(
                                  source:
                                      'AlphaWallet.executeCallback($id, "user rejected switch", null);',
                                );
                                Get.back();
                              },
                            );
                          }
                        },
                      );
                      _controller.addJavaScriptHandler(
                        handlerName: 'ethCall',
                        callback: (args) async {
                          final pref = Hive.box(secureStorageKey);
                          int chainId = pref.get(dappChainIdKey);
                          final rpc =
                              getEthereumDetailsFromChainId(chainId)['rpc'];
                          final id = args[0];
                          final tx = json.decode(args[1]) as Map;
                          try {
                            final client = web3.Web3Client(
                              rpc,
                              Client(),
                            );

                            final mnemonic = pref.get(currentMmenomicKey);
                            final blockChainDetails =
                                getEthereumDetailsFromChainId(chainId);
                            final web3Response = await getEthereumFromMemnomic(
                              mnemonic,
                              blockChainDetails['coinType'],
                            );

                            final sendingAddress =
                                web3Response['eth_wallet_address'];

                            final response = await client.callRaw(
                              sender: EthereumAddress.fromHex(sendingAddress),
                              contract: EthereumAddress.fromHex(tx['to']),
                              data: txDataToUintList(tx['data']),
                            );
                            await _controller.evaluateJavascript(
                              source:
                                  'AlphaWallet.executeCallback($id, null, "$response");',
                            );
                          } catch (e) {
                            final error = e.toString().replaceAll('"', '\'');

                            await _controller.evaluateJavascript(
                              source:
                                  'AlphaWallet.executeCallback($id, "$error",null);',
                            );
                          }
                        },
                      );
                      _controller.addJavaScriptHandler(
                        handlerName: 'signTransaction',
                        callback: (args) async {
                          final pref = Hive.box(secureStorageKey);
                          int chainId = pref.get(dappChainIdKey);
                          final mnemonic = pref.get(currentMmenomicKey);

                          final blockChainDetails =
                              getEthereumDetailsFromChainId(chainId);
                          final rpc = blockChainDetails['rpc'];
                          final web3Response = await getEthereumFromMemnomic(
                            mnemonic,
                            blockChainDetails['coinType'],
                          );

                          final privateKey =
                              web3Response['eth_wallet_privateKey'];

                          final sendingAddress =
                              web3Response['eth_wallet_address'];
                          final client = web3.Web3Client(
                            rpc,
                            Client(),
                          );
                          final credentials = EthPrivateKey.fromHex(privateKey);

                          final id = args[0];
                          final to = args[1];
                          final value = args[2];
                          final nonce = args[3] == -1 ? null : args[3];
                          final gasPrice = args[5];
                          final data = args[6];

                          await signTransaction(
                            gasPriceInWei_: gasPrice,
                            to: to,
                            from: sendingAddress,
                            txData: data,
                            valueInWei_: value,
                            gasInWei_: null,
                            networkIcon: null,
                            context: context,
                            blockChainCurrencySymbol:
                                blockChainDetails['symbol'],
                            name: '',
                            onConfirm: () async {
                              try {
                                final signedTransaction =
                                    await client.signTransaction(
                                  credentials,
                                  Transaction(
                                    to: to != null
                                        ? EthereumAddress.fromHex(to)
                                        : null,
                                    value: value != null
                                        ? EtherAmount.inWei(
                                            BigInt.parse(value),
                                          )
                                        : null,
                                    nonce: nonce,
                                    gasPrice: gasPrice != null
                                        ? EtherAmount.inWei(
                                            BigInt.parse(gasPrice))
                                        : null,
                                    data: txDataToUintList(data),
                                  ),
                                  chainId: chainId,
                                );

                                final response = await client
                                    .sendRawTransaction(signedTransaction);

                                await _controller.evaluateJavascript(
                                  source:
                                      'AlphaWallet.executeCallback($id, null, "$response");',
                                );
                              } catch (e) {
                                final error =
                                    e.toString().replaceAll('"', '\'');

                                await _controller.evaluateJavascript(
                                  source:
                                      'AlphaWallet.executeCallback($id, "$error",null);',
                                );
                              } finally {
                                Get.back();
                              }
                            },
                            onReject: () async {
                              await _controller.evaluateJavascript(
                                source:
                                    'AlphaWallet.executeCallback($id, "user rejected transaction",null);',
                              );
                              Get.back();
                            },
                            title: 'Sign Transaction',
                            chainId: chainId,
                          );
                        },
                      );

                      _controller.addJavaScriptHandler(
                        handlerName: 'signMessage',
                        callback: (args) async {
                          final pref = Hive.box(secureStorageKey);
                          int chainId = pref.get(dappChainIdKey);
                          final mnemonic = pref.get(currentMmenomicKey);

                          final blockChainDetails =
                              getEthereumDetailsFromChainId(chainId);
                          final web3Response = await getEthereumFromMemnomic(
                            mnemonic,
                            blockChainDetails['coinType'],
                          );

                          final privateKey =
                              web3Response['eth_wallet_privateKey'];

                          final credentials = EthPrivateKey.fromHex(privateKey);

                          final id = args[0];
                          String data = args[1];
                          String messageType = args[2];
                          if (messageType == typedMessageSignKey) {
                            data = json.decode(data)['data'];
                          }

                          await signMessage(
                            context: context,
                            messageType: messageType,
                            data: data,
                            networkIcon: null,
                            name: null,
                            onConfirm: () async {
                              try {
                                String signedDataHex;
                                Uint8List signedData;
                                if (messageType == typedMessageSignKey) {
                                  signedDataHex = EthSigUtil.signTypedData(
                                    privateKey: privateKey,
                                    jsonData: data,
                                    version: TypedDataVersion.V4,
                                  );
                                } else if (messageType == personalSignKey) {
                                  signedData =
                                      await credentials.signPersonalMessage(
                                    txDataToUintList(data),
                                  );
                                  signedDataHex =
                                      bytesToHex(signedData, include0x: true);
                                } else if (messageType == normalSignKey) {
                                  try {
                                    signedDataHex = EthSigUtil.signMessage(
                                      privateKey: privateKey,
                                      message: txDataToUintList(data),
                                    );
                                  } catch (e) {
                                    signedData =
                                        await credentials.signPersonalMessage(
                                      txDataToUintList(data),
                                    );
                                    signedDataHex =
                                        bytesToHex(signedData, include0x: true);
                                  }
                                }
                                await _controller.evaluateJavascript(
                                  source:
                                      'AlphaWallet.executeCallback($id, null, "$signedDataHex");',
                                );
                              } catch (e) {
                                final error =
                                    e.toString().replaceAll('"', '\'');

                                await _controller.evaluateJavascript(
                                  source:
                                      'AlphaWallet.executeCallback($id, "$error",null);',
                                );
                              } finally {
                                Get.back();
                              }
                            },
                            onReject: () {
                              _controller.evaluateJavascript(
                                source:
                                    'AlphaWallet.executeCallback($id, "user rejected signature",null);',
                              );
                              Get.back();
                            },
                          );
                        },
                      );
                    },
                    initialUserScripts: UnmodifiableListView([
                      UserScript(
                        source: widget.provider + initJs,
                        injectionTime:
                            UserScriptInjectionTime.AT_DOCUMENT_START,
                      )
                    ]),
                    onLoadStart:
                        (InAppWebViewController controller, Uri url) async {
                      browserController.text = url.toString();

                      final pref = Hive.box(secureStorageKey);

                      final documentTitle = await controller.getTitle();

                      List history = [
                        {
                          'url': url.toString(),
                          'title': documentTitle,
                        }
                      ];
                      final savedHistory = pref.get(historyKey);
                      if (savedHistory != null) {
                        history.addAll(jsonDecode(savedHistory) as List);
                      }
                      history.length = maximumBrowserHistoryToSave;
                      await pref.put(
                        historyKey,
                        jsonEncode(history),
                      );
                    },
                    onProgressChanged:
                        (InAppWebViewController controller, int progress) {
                      loadingPercent.value = progress / 100;
                    },
                    onConsoleMessage: (InAppWebViewController controller,
                        ConsoleMessage message) {
                      if (kDebugMode) {
                        print(message.toString());
                      }
                    },
                  ),
                ),
              ),
            ]),
          ),
        ),
      ),
    );
  }
}
