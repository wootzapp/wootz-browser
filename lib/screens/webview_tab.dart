import 'dart:collection';
import 'dart:convert';
import 'dart:typed_data';
import 'package:cryptowallet/utils/wc_connector.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:cryptowallet/utils/slide_up_panel.dart';
import 'package:eth_sig_util/eth_sig_util.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:http/http.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:url_launcher/url_launcher.dart';
import 'package:wallet_connect/wallet_connect.dart';
import 'package:web3dart/crypto.dart';
import 'package:web3dart/web3dart.dart';

import '../utils/app_config.dart';

class WebViewTab extends StatefulWidget {
  final String url;
  final int windowId;
  final String provider;
  final String init;
  final String data;

  final Function() onStateUpdated;
  final Function(CreateWindowAction createWindowAction) onCreateTabRequested;
  final Function() onCloseTabRequested;

  String get currentUrl {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._url;
  }

  bool get isSecure {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._isSecure;
  }

  InAppWebViewController get controller {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._controller;
  }

  TextEditingController get browserController {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._browserController;
  }

  Uint8List get screenshot {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._screenshot;
  }

  String get title {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._title;
  }

  Favicon get favicon {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._favicon;
  }

  const WebViewTab(
      {GlobalKey key,
      this.url,
      this.onStateUpdated,
      this.onCloseTabRequested,
      this.onCreateTabRequested,
      this.data,
      this.provider,
      this.init,
      this.windowId})
      : assert(url != null || windowId != null),
        super(key: key);

  @override
  State<WebViewTab> createState() => _WebViewTabState();

  Future<void> updateScreenshot() async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    await state?.updateScreenshot();
  }

  Future<void> pause() async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    await state?.pause();
  }

  Future<void> resume() async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    await state?.resume();
  }

  Future<bool> canGoBack() async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return await state?.canGoBack() ?? false;
  }

  Future<void> goBack() async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    await state?.goBack();
  }

  Future<bool> canGoForward() async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return await state?.canGoForward() ?? false;
  }

  Future<void> goForward() async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    await state?.goForward();
  }

  Future<void> changeBrowserChainId_(int chainId, String rpc) async {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    await state?.changeBrowserChainId_(chainId, rpc);
  }
}

class _WebViewTabState extends State<WebViewTab> with WidgetsBindingObserver {
  InAppWebViewController _controller;
  final _browserController = TextEditingController();
  Uint8List _screenshot;
  String _url = '';
  bool _isSecure;
  String _title = '';
  Favicon _favicon;
  double _progress = 0;
  String initJs = '';
  @override
  void initState() {
    super.initState();
    initJs = widget.init;
    WidgetsBinding.instance.addObserver(this);

    _url = widget.url ?? '';
  }

  @override
  void dispose() {
    _controller = null;
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
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

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (!kIsWeb) {
      if (state == AppLifecycleState.resumed) {
        resume();
        _controller?.resumeTimers();
      } else {
        pause();
        _controller?.pauseTimers();
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Column(children: <Widget>[
      Expanded(
        child: Stack(
          children: [
            Container(
              color: Colors.white,
            ),
            InAppWebView(
              windowId: widget.windowId,
              initialUrlRequest:
                  URLRequest(url: WebUri(widget.data ?? walletURL)),
              initialSettings: InAppWebViewSettings(
                useShouldOverrideUrlLoading: true,
                forceDark: Theme.of(context).brightness == Brightness.dark
                    ? ForceDark.ON
                    : ForceDark.OFF,
                isFraudulentWebsiteWarningEnabled: true,
                safeBrowsingEnabled: true,
                mediaPlaybackRequiresUserGesture: false,
                allowsInlineMediaPlayback: true,
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
                String url_ = url.toString();
                List<String> allowedAction = [
                  "http",
                  "https",
                  "file",
                  "chrome",
                  "data",
                  "javascript",
                  "about"
                ];
                if (url_.contains('wc?uri=')) {
                  final wcUri = Uri.parse(
                    Uri.decodeFull(
                      Uri.parse(url_).queryParameters['uri'],
                    ),
                  );

                  final session = WCSession.from(wcUri.toString());

                  if (session != WCSession.empty()) {
                    await WcConnector.qrScanHandler(wcUri.toString());
                  } else {
                    await WcConnector.wcReconnect();
                  }

                  return NavigationActionPolicy.CANCEL;
                } else if (url_.startsWith('wc:')) {
                  final session = WCSession.from(url_);

                  if (session != WCSession.empty()) {
                    await WcConnector.qrScanHandler(url_);
                  } else {
                    await WcConnector.wcReconnect();
                  }

                  return NavigationActionPolicy.CANCEL;
                }
                if (!allowedAction.contains(url.scheme)) {
                  try {
                    await launchUrl(url);
                  } catch (_) {}
                  return NavigationActionPolicy.CANCEL;
                }

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

                    final sendingAddress = web3Response['eth_wallet_address'];
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
                        BigInt.parse(json.decode(args[1])['chainId']).toInt();

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
                        BigInt.parse(json.decode(args[1])['chainId']).toInt();

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
                          Navigator.pop(context);
                        },
                        onReject: () async {
                          await _controller.evaluateJavascript(
                            source:
                                'AlphaWallet.executeCallback($id, "user rejected switch", null);',
                          );
                          Navigator.pop(context);
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
                    final rpc = getEthereumDetailsFromChainId(chainId)['rpc'];
                    final id = args[0];
                    final tx = json.decode(args[1]) as Map;
                    try {
                      final client = Web3Client(
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

                      final sendingAddress = web3Response['eth_wallet_address'];

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

                    final privateKey = web3Response['eth_wallet_privateKey'];

                    final sendingAddress = web3Response['eth_wallet_address'];
                    final client = Web3Client(
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
                      blockChainCurrencySymbol: blockChainDetails['symbol'],
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
                                  ? EtherAmount.inWei(BigInt.parse(gasPrice))
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
                          final error = e.toString().replaceAll('"', '\'');

                          await _controller.evaluateJavascript(
                            source:
                                'AlphaWallet.executeCallback($id, "$error",null);',
                          );
                        } finally {
                          Navigator.pop(context);
                        }
                      },
                      onReject: () async {
                        await _controller.evaluateJavascript(
                          source:
                              'AlphaWallet.executeCallback($id, "user rejected transaction",null);',
                        );
                        Navigator.pop(context);
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

                    final privateKey = web3Response['eth_wallet_privateKey'];

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
                            signedData = await credentials.signPersonalMessage(
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
                          final error = e.toString().replaceAll('"', '\'');

                          await _controller.evaluateJavascript(
                            source:
                                'AlphaWallet.executeCallback($id, "$error",null);',
                          );
                        } finally {
                          Navigator.pop(context);
                        }
                      },
                      onReject: () {
                        _controller.evaluateJavascript(
                          source:
                              'AlphaWallet.executeCallback($id, "user rejected signature",null);',
                        );
                        Navigator.pop(context);
                      },
                    );
                  },
                );
              },
              initialUserScripts: UnmodifiableListView([
                UserScript(
                  source: widget.provider + initJs,
                  injectionTime: UserScriptInjectionTime.AT_DOCUMENT_START,
                )
              ]),
              onLoadStart: (InAppWebViewController controller, Uri url) async {
                _browserController.text = url.toString();

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

                _favicon = null;
                _title = '';
                if (url != null) {
                  _url = url.toString();
                  _isSecure = urlIsSecure(url);
                }
                widget.onStateUpdated.call();
                await pref.put(
                  historyKey,
                  jsonEncode(history),
                );
              },
              onUpdateVisitedHistory: (controller, url, isReload) {
                if (url != null) {
                  _url = url.toString();
                  widget.onStateUpdated.call();
                }
              },
              onTitleChanged: (controller, title) {
                _title = title ?? '';
                widget.onStateUpdated.call();
              },
              onProgressChanged: (controller, progress) {
                setState(() {
                  _progress = progress / 100;
                });
              },
              onCreateWindow: (controller, createWindowAction) async {
                widget.onCreateTabRequested(createWindowAction);
                return true;
              },
              onCloseWindow: (controller) {
                widget.onCloseTabRequested();
              },
              onLoadStop: (controller, url) async {
                updateScreenshot();

                if (url != null) {
                  final sslCertificate = await controller.getCertificate();
                  _url = url.toString();
                  _isSecure = sslCertificate != null || urlIsSecure(url);
                }

                final favicons = await _controller?.getFavicons();
                if (favicons != null && favicons.isNotEmpty) {
                  for (final favicon in favicons) {
                    if (_favicon == null) {
                      _favicon = favicon;
                    } else if (favicon.width != null &&
                        (favicon.width ?? 0) > (_favicon?.width ?? 0)) {
                      _favicon = favicon;
                    }
                  }
                }

                widget.onStateUpdated.call();
              },
              onConsoleMessage:
                  (InAppWebViewController controller, ConsoleMessage message) {
                if (kDebugMode) {
                  print(message.toString());
                }
              },
            ),
            _progress < 1.0
                ? LinearProgressIndicator(
                    value: _progress,
                  )
                : Container(),
          ],
        ),
      ),
    ]);
  }

  Future<void> updateScreenshot() async {
    _screenshot = await _controller
        ?.takeScreenshot(
            screenshotConfiguration: ScreenshotConfiguration(
                compressFormat: CompressFormat.JPEG, quality: 20))
        ?.timeout(
          const Duration(milliseconds: 1500),
          onTimeout: () => null,
        );
  }

  Future<void> pause() async {
    if (!kIsWeb) {
      if (defaultTargetPlatform == TargetPlatform.iOS) {
        await _controller?.setAllMediaPlaybackSuspended(suspended: true);
      } else if (defaultTargetPlatform == TargetPlatform.android) {
        await _controller?.pause();
      }
    }
  }

  Future<void> resume() async {
    if (!kIsWeb) {
      if (defaultTargetPlatform == TargetPlatform.iOS) {
        await _controller?.setAllMediaPlaybackSuspended(suspended: false);
      } else if (defaultTargetPlatform == TargetPlatform.android) {
        await _controller?.resume();
      }
    }
  }

  Future<bool> canGoBack() async {
    return await _controller?.canGoBack() ?? false;
  }

  Future<void> goBack() async {
    if (await canGoBack()) {
      await _controller?.goBack();
    }
  }

  Future<bool> canGoForward() async {
    return await _controller?.canGoForward() ?? false;
  }

  Future<void> goForward() async {
    if (await canGoForward()) {
      await _controller?.goForward();
    }
  }

  static bool urlIsSecure(Uri url) {
    return (url.scheme == "https") || isLocalizedContent(url);
  }

  static bool isLocalizedContent(Uri url) {
    return (url.scheme == "file" ||
        url.scheme == "chrome" ||
        url.scheme == "data" ||
        url.scheme == "javascript" ||
        url.scheme == "about");
  }
}
