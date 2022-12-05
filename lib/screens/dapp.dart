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
  ValueNotifier loadingPercent = ValueNotifier<double>(0);
  String urlLoaded = '';
  bool showWebViewTabsViewer = false;
  String initJs = '';
  List<WebViewTab> webViewTabs = [];
  int currentTabIndex = 0;
  @override
  initState() {
    super.initState();

    initJs = widget.init;
    webViewTabs.add(createWebViewTab());
  }

  @override
  void dispose() {
    try {
      webViewTabs[currentTabIndex].browserController.dispose();
    } catch (_) {}
    super.dispose();
  }

  WebViewTab createWebViewTab({String url, int windowId}) {
    WebViewTab webViewTab;

    if (url == null && windowId == null) {
      url = walletURL;
    }

    webViewTab = WebViewTab(
      key: GlobalKey(),
      url: url,
      provider: widget.provider,
      init: widget.init,
      data: widget.data,
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

  Widget _buildWebViewTabAppBar() {
    final localize = AppLocalizations.of(context);
    final historyTitle = localize.history;
    final historyEmpty = localize.noHistory;
    final pref = Hive.box(secureStorageKey);
    return PreferredSize(
      child: SafeArea(
        child: SizedBox(
          height: 75,
          child: Row(
            mainAxisAlignment: MainAxisAlignment.center,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              IconButton(
                constraints: const BoxConstraints(maxWidth: 35),
                onPressed: () {
                  _addWebViewTab();
                },
                icon: const Icon(Icons.add),
              ),
              Flexible(
                child: TextFormField(
                  onFieldSubmitted: (value) async {
                    FocusManager.instance.primaryFocus?.unfocus();
                    if (webViewTabs[currentTabIndex].controller != null) {
                      Uri uri = blockChainToHttps(value.trim());
                      await webViewTabs[currentTabIndex].controller.loadUrl(
                            urlRequest: URLRequest(url: WebUri.uri(uri)),
                          );
                    }
                  },
                  textInputAction: TextInputAction.search,
                  controller: webViewTabs[currentTabIndex].browserController,
                  decoration: InputDecoration(
                    prefixIconConstraints:
                        const BoxConstraints(minWidth: 35, maxWidth: 35),
                    contentPadding: const EdgeInsets.all(0),
                    suffixIconConstraints:
                        const BoxConstraints(minWidth: 35, maxWidth: 35),
                    prefixIcon: webViewTabs[currentTabIndex].isSecure != null
                        ? Padding(
                            padding: const EdgeInsets.only(left: 8.0, right: 8),
                            child: Icon(
                                webViewTabs[currentTabIndex].isSecure == true
                                    ? Icons.lock
                                    : Icons.lock_open,
                                color: webViewTabs[currentTabIndex].isSecure ==
                                        true
                                    ? Colors.green
                                    : Colors.red,
                                size: 18),
                          )
                        : Container(),
                    isDense: true,
                    suffixIcon: IconButton(
                      icon: const Icon(
                        Icons.cancel,
                      ),
                      onPressed: () {
                        webViewTabs[currentTabIndex].browserController.clear();
                      },
                    ),
                    hintText: AppLocalizations.of(context).searchOrEnterUrl,

                    filled: true,
                    focusedBorder: const OutlineInputBorder(
                        borderRadius: BorderRadius.all(Radius.circular(10.0)),
                        borderSide: BorderSide.none),
                    border: const OutlineInputBorder(
                        borderRadius: BorderRadius.all(Radius.circular(10.0)),
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
                onPressed: () async {
                  await webViewTabs[currentTabIndex].updateScreenshot();
                  setState(() {
                    showWebViewTabsViewer = true;
                  });
                },
                icon: Container(
                  decoration: BoxDecoration(
                      border: Border.all(
                        width: 2.0,
                        color: Theme.of(context).iconTheme.color,
                      ),
                      shape: BoxShape.rectangle,
                      borderRadius: BorderRadius.circular(5.0)),
                  constraints: const BoxConstraints(minWidth: 25.0),
                  child: Center(
                      child: Text(
                    webViewTabs.length.toString(),
                    style: const TextStyle(
                        fontWeight: FontWeight.bold, fontSize: 14.0),
                  )),
                ),
              ),
              IconButton(
                constraints: const BoxConstraints(maxWidth: 35),
                onPressed: () {
                  if (webViewTabs[currentTabIndex].controller == null) return;

                  final bookMark = pref.get(bookMarkKey);
                  final url =
                      webViewTabs[currentTabIndex].browserController.text;
                  List savedBookMarks = [];

                  if (bookMark != null) {
                    savedBookMarks.addAll(
                      jsonDecode(bookMark) as List,
                    );
                  }
                  final bookMarkIndex = savedBookMarks.indexWhere(
                    (bookmark) => bookmark != null && bookmark['url'] == url,
                  );

                  bool urlBookMarked = bookMarkIndex != -1;
                  slideUpPanel(
                    context,
                    SingleChildScrollView(
                      child: Padding(
                        padding: const EdgeInsets.all(8.0),
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.start,
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            SizedBox(
                              width: double.infinity,
                              child: InkWell(
                                onTap: () async {
                                  await goForward();
                                  Navigator.pop(context);
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
                                  Navigator.pop(context);
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
                                  if (webViewTabs[currentTabIndex].controller !=
                                      null) {
                                    await webViewTabs[currentTabIndex]
                                        .controller
                                        .reload();
                                    Navigator.pop(context);
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
                                  if (webViewTabs[currentTabIndex].controller !=
                                      null) {
                                    await Share.share(url);
                                    Navigator.pop(context);
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
                                    } else if (webViewTabs[currentTabIndex]
                                            .controller !=
                                        null) {
                                      final title =
                                          await webViewTabs[currentTabIndex]
                                              .controller
                                              .getTitle();

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
                                    Navigator.pop(context);
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
                                    if (webViewTabs[currentTabIndex]
                                            .controller !=
                                        null) {
                                      await webViewTabs[currentTabIndex]
                                          .controller
                                          .clearCache();
                                      Navigator.pop(context);
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
                                  final historyUrl =
                                      await Navigator.pushReplacement(
                                    context,
                                    MaterialPageRoute(
                                      builder: (ctx) => SavedUrls(
                                        historyTitle,
                                        historyEmpty,
                                        historyKey,
                                        data: historyList,
                                      ),
                                    ),
                                  );

                                  if (historyUrl != null) {
                                    webViewTabs[currentTabIndex]
                                        .controller
                                        .loadUrl(
                                          urlRequest: URLRequest(
                                            url: WebUri(historyUrl),
                                          ),
                                        );
                                  }
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
                                          AppLocalizations.of(context).history,
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
                                  onTap: () {
                                    showBlockChainDialog(
                                      context: context,
                                      onTap: (blockChainData) async {
                                        await webViewTabs[currentTabIndex]
                                            .changeBrowserChainId_(
                                          blockChainData['chainId'],
                                          blockChainData['rpc'],
                                        );
                                        int count = 0;
                                        Navigator.popUntil(context, (route) {
                                          return count++ == 2;
                                        });
                                      },
                                      selectedChainId: pref.get(dappChainIdKey),
                                    );
                                  },
                                  child: Container(
                                    color: Colors.transparent,
                                    child: Padding(
                                      padding: const EdgeInsets.all(15),
                                      child: Row(
                                        children: [
                                          const Icon(FontAwesomeIcons.ethereum),
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
                                          }(), builder: (context, snapshot) {
                                            return Text(
                                              '${AppLocalizations.of(context).network}: ${snapshot.hasData ? snapshot.data : ''}',
                                            );
                                          })
                                        ],
                                      ),
                                    ),
                                  )),
                            ),
                          ],
                        ),
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
        ),
      ),
      preferredSize: const Size.fromHeight(100),
    );
    return AppBar(
      leading: IconButton(
          onPressed: () {
            _addWebViewTab();
          },
          icon: const Icon(Icons.add)),
      title: TextFormField(
        onFieldSubmitted: (value) async {
          FocusManager.instance.primaryFocus?.unfocus();
          if (webViewTabs[currentTabIndex].controller != null) {
            Uri uri = blockChainToHttps(value.trim());
            await webViewTabs[currentTabIndex].controller.loadUrl(
                  urlRequest: URLRequest(url: WebUri.uri(uri)),
                );
          }
        },
        textInputAction: TextInputAction.search,
        controller: webViewTabs[currentTabIndex].browserController,
        decoration: InputDecoration(
          prefixIcon: webViewTabs[currentTabIndex].isSecure != null
              ? Icon(
                  webViewTabs[currentTabIndex].isSecure == true
                      ? Icons.lock
                      : Icons.lock_open,
                  color: webViewTabs[currentTabIndex].isSecure == true
                      ? Colors.green
                      : Colors.red,
                  size: 12)
              : Container(),
          isDense: true,
          suffixIcon: IconButton(
            icon: const Icon(
              Icons.cancel,
            ),
            onPressed: () {
              webViewTabs[currentTabIndex].browserController.clear();
            },
          ),
          hintText: AppLocalizations.of(context).searchOrEnterUrl,

          filled: true,
          focusedBorder: const OutlineInputBorder(
              borderRadius: BorderRadius.all(Radius.circular(10.0)),
              borderSide: BorderSide.none),
          border: const OutlineInputBorder(
              borderRadius: BorderRadius.all(Radius.circular(10.0)),
              borderSide: BorderSide.none),
          enabledBorder: const OutlineInputBorder(
            borderRadius: BorderRadius.all(Radius.circular(10.0)),
            borderSide: BorderSide.none,
          ), // you
        ),
      ),

      // title: Column(
      //   mainAxisAlignment: MainAxisAlignment.center,
      //   children: [
      //     Text(
      //       webViewTabs[currentTabIndex].title ?? '',
      //       overflow: TextOverflow.fade,
      //     ),
      //     // Row(
      //     //   mainAxisSize: MainAxisSize.max,
      //     //   mainAxisAlignment: MainAxisAlignment.center,
      //     //   children: [
      //     //     webViewTabs[currentTabIndex].isSecure != null
      //     //         ? Icon(
      //     //             webViewTabs[currentTabIndex].isSecure == true
      //     //                 ? Icons.lock
      //     //                 : Icons.lock_open,
      //     //             color: webViewTabs[currentTabIndex].isSecure == true
      //     //                 ? Colors.green
      //     //                 : Colors.red,
      //     //             size: 12)
      //     //         : Container(),
      //     //     const SizedBox(
      //     //       width: 5,
      //     //     ),
      //     //     // Flexible(
      //     //     //   child: Text(
      //     //     //     webViewTabs[currentTabIndex].currentUrl ??
      //     //     //         webViewTabs[currentTabIndex].url ??
      //     //     //         '',
      //     //     //     style: const TextStyle(fontSize: 12, color: Colors.white70),
      //     //     //     overflow: TextOverflow.fade,
      //     //     //   ),
      //     //     // ),
      //     //   ],
      //     // ),
      //   ],
      // ),
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
      IconButton(
        icon: const Icon(Icons.more_vert),
        onPressed: () {
          FocusManager.instance.primaryFocus?.unfocus();

          if (webViewTabs[currentTabIndex].controller == null) return;
          final pref = Hive.box(secureStorageKey);
          final bookMark = pref.get(bookMarkKey);
          final url = webViewTabs[currentTabIndex].browserController.text;
          List savedBookMarks = [];

          if (bookMark != null) {
            savedBookMarks.addAll(
              jsonDecode(bookMark) as List,
            );
          }
          final bookMarkIndex = savedBookMarks.indexWhere(
            (bookmark) => bookmark != null && bookmark['url'] == url,
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
                        Navigator.pop(context);
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
                        Navigator.pop(context);
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
                        if (webViewTabs[currentTabIndex].controller != null) {
                          FocusManager.instance.primaryFocus?.unfocus();
                          await webViewTabs[currentTabIndex]
                              .controller
                              .reload();
                          Navigator.pop(context);
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
                        if (webViewTabs[currentTabIndex].controller != null) {
                          await Share.share(url);
                          Navigator.pop(context);
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
                          } else if (webViewTabs[currentTabIndex].controller !=
                              null) {
                            final title = await webViewTabs[currentTabIndex]
                                .controller
                                .getTitle();

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
                          Navigator.pop(context);
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
                          if (webViewTabs[currentTabIndex].controller != null) {
                            await webViewTabs[currentTabIndex]
                                .controller
                                .clearCache();
                            Navigator.pop(context);
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
                          historyList = jsonDecode(savedHistory) as List;
                        }

                        final localize = AppLocalizations.of(context);
                        final historyTitle = localize.history;
                        final historyEmpty = localize.noHistory;

                        final historyUrl = await Navigator.pushReplacement(
                          context,
                          MaterialPageRoute(
                            builder: (ctx) => SavedUrls(
                              historyTitle,
                              historyEmpty,
                              historyKey,
                              data: historyList,
                            ),
                          ),
                        );

                        if (historyUrl != null) {
                          webViewTabs[currentTabIndex].controller.loadUrl(
                                urlRequest: URLRequest(
                                  url: WebUri(historyUrl),
                                ),
                              );
                        }
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
                                AppLocalizations.of(context).history,
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
                                await webViewTabs[currentTabIndex]
                                    .changeBrowserChainId_(
                                  blockChainData['chainId'],
                                  blockChainData['rpc'],
                                );
                                int count = 0;

                                Navigator.popUntil(
                                    context, (route) => count++ == 2);
                              },
                              selectedChainId: pref.get(dappChainIdKey),
                            );
                          },
                          child: Container(
                            color: Colors.transparent,
                            child: Padding(
                              padding: const EdgeInsets.all(15),
                              child: Row(
                                children: [
                                  const Icon(FontAwesomeIcons.ethereum),
                                  const SizedBox(
                                    width: 10,
                                  ),
                                  FutureBuilder(future: () async {
                                    final pref = Hive.box(secureStorageKey);
                                    final chainId = pref.get(dappChainIdKey);
                                    return getEthereumDetailsFromChainId(
                                        chainId)['symbol'];
                                  }(), builder: (context, snapshot) {
                                    return Text(
                                      '${AppLocalizations.of(context).network}: ${snapshot.hasData ? snapshot.data : ''}',
                                    );
                                  })
                                ],
                              ),
                            ),
                          )),
                    ),
                  if (pref.get(dappChainIdKey) != null) const Divider(),
                  SizedBox(
                    width: double.infinity,
                    child: InkWell(
                      onTap: () async {
                        Navigator.pop(context);
                        await Navigator.push(
                          context,
                          MaterialPageRoute(
                            builder: (ctx) => const Settings(),
                          ),
                        );
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
                                AppLocalizations.of(context).settings,
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
                          bool hasWallet = pref.get(currentMmenomicKey) != null;

                          bool hasPasscode =
                              pref.get(userUnlockPasscodeKey) != null;
                          Widget dappWidget;
                          Navigator.pop(context);

                          if (hasWallet) {
                            dappWidget = const WalletMainBody();
                          } else if (hasPasscode) {
                            dappWidget = const MainScreen();
                          } else {
                            dappWidget = const Security();
                          }
                          await Navigator.push(
                            context,
                            MaterialPageRoute(
                              builder: (ctx) => dappWidget,
                            ),
                          );
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
                                  AppLocalizations.of(context).wallet,
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
        },
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
      title: const Text('Tabs'),
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

  Future<bool> goBack() async {
    if (webViewTabs[currentTabIndex].controller != null &&
        await webViewTabs[currentTabIndex].controller.canGoBack()) {
      webViewTabs[currentTabIndex].controller.goBack();
      return true;
    }
    return false;
  }

  Future<bool> goForward() async {
    if (webViewTabs[currentTabIndex].controller != null &&
        await webViewTabs[currentTabIndex].controller.canGoForward()) {
      webViewTabs[currentTabIndex].controller.goForward();
      return true;
    }
    return false;
  }

  @override
  Widget build(BuildContext context) {
    return WillPopScope(
      child: Scaffold(
          appBar: showWebViewTabsViewer
              ? _buildWebViewTabViewerAppBar()
              : _buildWebViewTabAppBar(),
          body: IndexedStack(
            index: showWebViewTabsViewer ? 1 : 0,
            children: [_buildWebViewTabs(), _buildWebViewTabsViewer()],
          )),
      onWillPop: () async {
        if (showWebViewTabsViewer) {
          setState(() {
            showWebViewTabsViewer = false;
          });
        } else if (await webViewTabs[currentTabIndex].canGoBack()) {
          webViewTabs[currentTabIndex].goBack();
        } else {
          return true;
        }
        return false;
      },
    );
  }
}
