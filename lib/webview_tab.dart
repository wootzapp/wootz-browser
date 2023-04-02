import 'dart:collection';
import 'dart:convert';

import 'package:cryptowallet/screens/js_model.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:cryptowallet/utils/wc_connector.dart';
import 'package:cryptowallet/utils/web_notifications.dart';
import 'package:eth_sig_util/eth_sig_util.dart';
import 'package:eth_sig_util/util/utils.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:cryptowallet/main.dart';
import 'package:cryptowallet/models/webview_model.dart';
import 'package:cryptowallet/util.dart';
import 'package:flutter_downloader/flutter_downloader.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:path_provider/path_provider.dart';
import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';
import 'package:vibration/vibration.dart';
import 'package:wallet_connect/wallet_connect.dart';
import 'package:web3dart/web3dart.dart';

import 'api/notification_api.dart';
import 'javascript_console_result.dart';
import 'long_press_alert_dialog.dart';
import 'models/browser_model.dart';
import 'models/provider.dart';

final webViewTabStateKey = GlobalKey<_WebViewTabState>();

class WebViewTab extends StatefulWidget {
  const WebViewTab({Key key, this.webViewModel}) : super(key: key);

  final WebViewModel webViewModel;

  @override
  State<WebViewTab> createState() => _WebViewTabState();
}

class _WebViewTabState extends State<WebViewTab> with WidgetsBindingObserver {
  InAppWebViewController _webViewController;
  PullToRefreshController _pullToRefreshController;
  FindInteractionController _findInteractionController;
  bool _isWindowClosed = false;
  WebNotificationController webNotificationController;
  final TextEditingController _httpAuthUsernameController =
      TextEditingController();
  final TextEditingController _httpAuthPasswordController =
      TextEditingController();
  List<UserScript> webNotification;
  final jsonNotification =
      jsonEncode(WebNotificationPermissionDb.getPermissions());

  String _addChain(int chainId, String rpcUrl, String sendingAddress) {
    String source = '''
        window.ethereum.setConfig({
          ethereum:{
            chainId: $chainId,
            rpcUrl: "$rpcUrl",
            address: "$sendingAddress"
            }
          }
        )
        ''';
    return source;
  }

  Future<void> _sendError(String network, String message, int methodId) {
    String script = "window.$network.sendError($methodId, \"$message\")";
    return _webViewController.evaluateJavascript(source: script);
  }

  Future<void> _sendResult(String network, String message, int methodId) {
    String script = "window.$network.sendResponse($methodId, \"$message\")";
    debugPrint(script);
    return _webViewController
        .evaluateJavascript(source: script)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
  }

  getWZlinks_(String url_) async {
    Uri link = Uri.parse(url_);
    final rpcUrl = link.queryParameters['rpc-url'];
    final name = link.queryParameters['network-name'];
    final chainId = link.queryParameters['chain-id'];
    final currencySymbol = link.queryParameters['currency-symbol'];
    final blockExplorer = link.queryParameters['block-explorer-url'];
    final args = [
      null,
      json.encode({
        "chainId": chainId.toString(),
        "chainName": name,
        "nativeCurrency": {
          "name": name,
          "symbol": currencySymbol,
          "decimals": 18
        },
        "rpcUrls": [rpcUrl],
        "blockExplorerUrls": [blockExplorer],
        "chainType": name
      })
    ];
    try {
      await addEthereumChainLogic(args);
    } catch (_) {}
  }

  addEthereumChainLogic(args) async {
    int chainId = pref.get(dappChainIdKey);

    final id = args[0];

    final dataValue = json.decode(args[1]);

    final switchChainId = BigInt.parse(dataValue['chainId']).toInt();

    final currentChainIdData = getEthereumDetailsFromChainId(chainId);

    Map switchChainIdData = getEthereumDetailsFromChainId(switchChainId);

    bool switchNetwork = true;

    if (switchChainIdData == null) {
      switchNetwork = false;

      List blockExplorers = dataValue['blockExplorerUrls'];
      String blockExplorer = '';
      if (blockExplorers.isNotEmpty) {
        blockExplorer = blockExplorers[0];
        if (blockExplorer.endsWith('/')) {
          blockExplorer = blockExplorer.substring(0, blockExplorer.length - 1);
        }
      }
      List rpcUrl = dataValue['rpcUrls'];

      Map addBlockChain = {};
      if (pref.get(newEVMChainKey) != null) {
        addBlockChain = Map.from(jsonDecode(pref.get(newEVMChainKey)));
      }

      switchChainIdData = {
        "rpc": rpcUrl.isNotEmpty ? rpcUrl[0] : null,
        'chainId': switchChainId,
        'blockExplorer': blockExplorers.isNotEmpty
            ? '$blockExplorer/tx/$transactionhashTemplateKey'
            : null,
        'symbol': dataValue['nativeCurrency']['symbol'],
        'default': dataValue['nativeCurrency']['symbol'],
        'image': 'assets/ethereum-2.png',
        'coinType': 60
      };

      Map details = {
        dataValue['chainName']: switchChainIdData,
      };
      addBlockChain.addAll(details);

      await addEthereumChain(
        context: context,
        jsonObj: json.encode(
          Map.from({
            'name': dataValue['chainName'],
          })
            ..addAll(switchChainIdData)
            ..remove('image')
            ..remove('coinType'),
        ),
        onConfirm: () async {
          try {
            const id = 83;
            final response = await post(
              Uri.parse(switchChainIdData['rpc']),
              body: json.encode(
                {
                  "jsonrpc": "2.0",
                  "method": "eth_chainId",
                  "params": [],
                  "id": id
                },
              ),
              headers: {"Content-Type": "application/json"},
            );
            String responseBody = response.body;

            if (response.statusCode ~/ 100 == 4 ||
                response.statusCode ~/ 100 == 5) {
              if (kDebugMode) {
                print(responseBody);
              }
              throw Exception(responseBody);
            }

            final jsonResponse = json.decode(responseBody);

            final chainIdResponse =
                BigInt.parse(jsonResponse['result']).toInt();

            if (jsonResponse['id'] != id) {
              throw Exception('invalid eth_chainId');
            } else if (chainIdResponse != switchChainId) {
              throw Exception('chain Id different with eth_chainId');
            }

            await pref.put(
              newEVMChainKey,
              jsonEncode(addBlockChain),
            );

            switchNetwork = true;
            Navigator.pop(context);
          } catch (e) {
            // final error = e.toString().replaceAll('"', '\'');

            Navigator.pop(context);
          }
        },
        onReject: () async {
          Navigator.pop(context);
        },
      );
    }
    if (switchNetwork) {
      switchEthereumChain(
        context: context,
        currentChainIdData: currentChainIdData,
        switchChainIdData: switchChainIdData,
        onConfirm: () async {
          await changeBrowserChainId_(
            switchChainIdData['chainId'],
            switchChainIdData['rpc'],
          );

          Navigator.pop(context);
        },
        onReject: () async {
          Navigator.pop(context);
        },
      );
    }
  }

  Future _switchWeb3ChainRequest({
    Map currentChainIdData,
    Map switchChainIdData,
    int switchChainId,
    String initString,
    JsCallbackModel jsData,
    bool haveNotExecuted = true,
  }) async {
    switchEthereumChain(
      context: context,
      currentChainIdData: currentChainIdData,
      switchChainIdData: switchChainIdData,
      onConfirm: () async {
        initJs = await changeBlockChainAndReturnInit(
          switchChainId,
          switchChainIdData['rpc'],
        );
        await _sendCustomResponse(initString);
        await _emitChange(switchChainId);
        await _sendNull(
          "ethereum",
          jsData.id ?? 0,
        );

        Navigator.pop(context);
      },
      onReject: () async {
        if (haveNotExecuted) {
          _sendError("ethereum", 'canceled', jsData.id ?? 0);
        }

        Navigator.pop(context);
      },
    );
  }

  Future _emitChange(int chainId) {
    final chain16 = "0x${chainId.toRadixString(16)}";
    String script = "trustwallet.ethereum.emitChainChanged(\"$chain16\");";
    return _webViewController
        .evaluateJavascript(source: script)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
  }

  Future<void> _sendNull(String network, int methodId) {
    String script = "window.$network.sendResponse($methodId, null)";
    debugPrint(script);
    return _webViewController
        .evaluateJavascript(source: script)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
  }

  Future<void> _sendCustomResponse(String response) {
    return _webViewController
        .evaluateJavascript(source: response)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
  }

  Future<void> _sendResults(
      String network, List<String> messages, int methodId) {
    String message = messages.join(",");
    String script = "window.$network.sendResponse($methodId, \"$message\")";
    return _webViewController.evaluateJavascript(source: script);
  }

  String initJs = '';

  changeBrowserChainId_(int chainId, String rpc) async {
    if (_webViewController == null) return;
    initJs = await changeBlockChainAndReturnInit(
      chainId,
      rpc,
    );

    await _webViewController?.removeAllUserScripts();
    await _webViewController?.addUserScripts(userScripts: [
      UserScript(
        source: '${provider}$initJs',
        injectionTime: UserScriptInjectionTime.AT_DOCUMENT_START,
      ),
      ...webNotification
    ]);
    await _webViewController?.reload();
  }

  @override
  void initState() {
    WidgetsBinding.instance.addObserver(this);
    super.initState();

    _pullToRefreshController = kIsWeb
        ? null
        : PullToRefreshController(
            settings: PullToRefreshSettings(color: Colors.blue),
            onRefresh: () async {
              if (defaultTargetPlatform == TargetPlatform.android) {
                _webViewController?.reload();
              } else if (defaultTargetPlatform == TargetPlatform.iOS) {
                _webViewController?.loadUrl(
                    urlRequest:
                        URLRequest(url: await _webViewController?.getUrl()));
              }
            },
          );

    webNotification = [
      UserScript(
          source: webNotifer,
          injectionTime: UserScriptInjectionTime.AT_DOCUMENT_START),
      UserScript(source: """
    (function(window) {
      var notificationPermissionDb = $jsonNotification;
      if (notificationPermissionDb[window.location.host] === 'granted') {
        Notification._permission = 'granted';
      } 
    })(window);
    """, injectionTime: UserScriptInjectionTime.AT_DOCUMENT_START)
    ];

    _findInteractionController = FindInteractionController();
    initJs = init;
  }

  @override
  void dispose() {
    _webViewController = null;
    widget.webViewModel.webViewController = null;
    widget.webViewModel.pullToRefreshController = null;
    widget.webViewModel.findInteractionController = null;

    _httpAuthUsernameController.dispose();
    _httpAuthPasswordController.dispose();

    WidgetsBinding.instance.removeObserver(this);

    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (_webViewController != null && Util.isAndroid()) {
      if (state == AppLifecycleState.paused) {
        pauseAll();
      } else {
        resumeAll();
      }
    }
  }

  void pauseAll() {
    if (Util.isAndroid()) {
      _webViewController?.pause();
    }
    pauseTimers();
  }

  void resumeAll() {
    if (Util.isAndroid()) {
      _webViewController?.resume();
    }
    resumeTimers();
  }

  void pause() {
    if (Util.isAndroid()) {
      _webViewController?.pause();
    }
  }

  void resume() {
    if (Util.isAndroid()) {
      _webViewController?.resume();
    }
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
      child: _buildWebView(),
    );
  }

  InAppWebView _buildWebView() {
    Box pref = Hive.box(secureStorageKey);
    var browserModel = Provider.of<BrowserModel>(context, listen: true);
    var settings = browserModel.getSettings();
    var currentWebViewModel = Provider.of<WebViewModel>(context, listen: true);

    if (Util.isAndroid()) {
      InAppWebViewController.setWebContentsDebuggingEnabled(
          settings.debuggingEnabled);
    }

    var initialSettings = widget.webViewModel.settings;
    initialSettings.useOnDownloadStart = true;
    initialSettings.useOnLoadResource = true;
    initialSettings.useShouldOverrideUrlLoading = true;
    initialSettings.javaScriptCanOpenWindowsAutomatically = true;
    initialSettings.userAgent =
        "Mozilla/5.0 (Linux; Android 9; LG-H870 Build/PKQ1.190522.001) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/83.0.4103.106 Mobile Safari/537.36";
    initialSettings.transparentBackground = true;

    initialSettings.safeBrowsingEnabled = true;
    initialSettings.disableDefaultErrorPage = true;
    initialSettings.supportMultipleWindows = true;
    initialSettings.verticalScrollbarThumbColor =
        const Color.fromRGBO(0, 0, 0, 0.5);
    initialSettings.horizontalScrollbarThumbColor =
        const Color.fromRGBO(0, 0, 0, 0.5);

    initialSettings.allowsLinkPreview = false;
    initialSettings.isFraudulentWebsiteWarningEnabled = true;
    initialSettings.disableLongPressContextMenuOnLinks = true;
    initialSettings.allowingReadAccessTo = WebUri('file://$WEB_ARCHIVE_DIR/');

    return InAppWebView(
      initialUrlRequest: URLRequest(url: widget.webViewModel.url),
      initialSettings: initialSettings,
      windowId: widget.webViewModel.windowId,
      pullToRefreshController: _pullToRefreshController,
      findInteractionController: _findInteractionController,
      initialUserScripts: UnmodifiableListView([
        UserScript(
          source: provider + initJs,
          injectionTime: UserScriptInjectionTime.AT_DOCUMENT_START,
        ),
        ...webNotification
      ]),
      onWebViewCreated: (controller) async {
        initialSettings.transparentBackground = false;
        await controller.setSettings(settings: initialSettings);

        _webViewController = controller;
        widget.webViewModel.webViewController = controller;
        widget.webViewModel.pullToRefreshController = _pullToRefreshController;
        widget.webViewModel.findInteractionController =
            _findInteractionController;

        webNotificationController = WebNotificationController(controller);
        _webViewController.addJavaScriptHandler(
          handlerName: 'Notification.requestPermission',
          callback: (arguments) async {
            final permission = await onNotificationRequestPermission();
            return permission.name.toLowerCase();
          },
        );

        _webViewController.addJavaScriptHandler(
          handlerName: 'Notification.show',
          callback: (arguments) {
            if (_webViewController != null) {
              final notification =
                  WebNotification.fromJson(arguments[0], _webViewController);
              onShowNotification(notification);
            }
          },
        );

        _webViewController.addJavaScriptHandler(
          handlerName: 'Notification.close',
          callback: (arguments) {
            final notificationId = arguments[0];
            onCloseNotification(notificationId);
          },
        );

        _webViewController.addJavaScriptHandler(
            handlerName: 'CryptoHandler',
            callback: (callback) async {
              final jsData = JsCallbackModel.fromJson(json.decode(callback[0]));

              final mnemonic = pref.get(currentMmenomicKey);
              if (jsData.network == 'solana') {
                final solanaResponse = await getSolanaFromMemnomic(mnemonic);
                final sendingAddress = solanaResponse['address'];

                switch (jsData.name) {
                  case "signMessage":
                    {
                      try {
                        //FIXME:
                        // final data = JsSolanaTransactionObject.fromJson(
                        //     jsData.object ?? {});
                        // final tx = await solanaKeyPair.signMessage();

                        // _sendResult(
                        //     "solana", tx.encode(), jsData.id ?? 0);
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("solana", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  case "signRawTransaction":
                    {
                      try {
                        //FIXME:
                        // final data = JsSolanaTransactionObject.fromJson(
                        //     jsData.object ?? {});
                        // final tx = await solana.signTransaction(
                        //   null,
                        //   null,
                        //   [solanaKeyPair],
                        // );

                        // _sendResult(
                        //     "solana", tx.encode(), jsData.id ?? 0);
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("solana", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  case "signRawTransactionMulti":
                    {
                      final data = JsSolanaTransactionObject.fromJson(
                          jsData.object ?? {});

                      try {} catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("solana", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  case "requestAccounts":
                    {
                      try {
                        final setAddress =
                            "trustwallet.solana.setAddress(\"$sendingAddress\");";

                        String callback =
                            "trustwallet.solana.sendResponse(${jsData.id}, [\"$sendingAddress\"])";

                        await _sendCustomResponse(setAddress);

                        await _sendCustomResponse(callback);
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("solana", error, jsData.id ?? 0);
                      }

                      break;
                    }
                }
              } else if (jsData.network == 'ethereum') {
                int chainId = pref.get(dappChainIdKey);

                final blockChainDetails =
                    getEthereumDetailsFromChainId(chainId);
                final rpc = blockChainDetails['rpc'];
                final web3Response = await getEthereumFromMemnomic(
                  mnemonic,
                  blockChainDetails['coinType'],
                );

                final privateKey = web3Response['eth_wallet_privateKey'];
                final credentials = EthPrivateKey.fromHex(privateKey);

                final sendingAddress = web3Response['eth_wallet_address'];
                switch (jsData.name) {
                  case "signTransaction":
                    {
                      final data =
                          JsTransactionObject.fromJson(jsData.object ?? {});
                      await signTransaction(
                        gasPriceInWei_: null,
                        to: data.to,
                        from: sendingAddress,
                        txData: data.data,
                        valueInWei_: data.value,
                        gasInWei_: null,
                        networkIcon: null,
                        context: context,
                        blockChainCurrencySymbol: blockChainDetails['symbol'],
                        name: '',
                        onConfirm: () async {
                          try {
                            final client = Web3Client(
                              rpc,
                              Client(),
                            );

                            final signedTransaction =
                                await client.signTransaction(
                              credentials,
                              Transaction(
                                to: data.to != null
                                    ? EthereumAddress.fromHex(data.to)
                                    : null,
                                value: data.value != null
                                    ? EtherAmount.inWei(
                                        BigInt.parse(data.value),
                                      )
                                    : null,
                                nonce: data.nonce != null
                                    ? int.parse(data.nonce)
                                    : null,
                                data: txDataToUintList(data.data),
                                gasPrice: data.gasPrice != null
                                    ? EtherAmount.inWei(
                                        BigInt.parse(data.gasPrice))
                                    : null,
                              ),
                              chainId: chainId,
                            );

                            final response = await client
                                .sendRawTransaction(signedTransaction);

                            _sendResult("ethereum", response, jsData.id ?? 0);
                          } catch (e) {
                            final error = e.toString().replaceAll('"', '\'');
                            _sendError("ethereum", error, jsData.id ?? 0);
                          } finally {
                            Navigator.pop(context);
                          }
                        },
                        onReject: () async {
                          _sendError(
                            "ethereum",
                            'user rejected transaction',
                            jsData.id ?? 0,
                          );
                          Navigator.pop(context);
                        },
                        title: 'Sign Transaction',
                        chainId: chainId,
                      );

                      break;
                    }
                  case "signPersonalMessage":
                    {
                      final data = JsDataModel.fromJson(jsData.object ?? {});

                      await signMessage(
                        context: context,
                        messageType: personalSignKey,
                        data: data.data,
                        networkIcon: null,
                        name: null,
                        onConfirm: () async {
                          try {
                            List signedData =
                                await credentials.signPersonalMessage(
                              txDataToUintList(data.data),
                            );

                            _sendResult(
                              "ethereum",
                              bytesToHex(signedData, include0x: true),
                              jsData.id ?? 0,
                            );
                          } catch (e) {
                            final error = e.toString().replaceAll('"', '\'');
                            _sendError("ethereum", error, jsData.id ?? 0);
                          } finally {
                            Navigator.pop(context);
                          }
                        },
                        onReject: () {
                          _sendError("ethereum", 'user rejected signature',
                              jsData.id ?? 0);
                          Navigator.pop(context);
                        },
                      );
                      break;
                    }
                  case "signMessage":
                    {
                      try {
                        final data = JsDataModel.fromJson(jsData.object ?? {});

                        String signedDataHex;

                        await signMessage(
                          context: context,
                          messageType: normalSignKey,
                          data: data.data,
                          networkIcon: null,
                          name: null,
                          onConfirm: () async {
                            try {
                              try {
                                signedDataHex = EthSigUtil.signMessage(
                                  privateKey: privateKey,
                                  message: txDataToUintList(data.data),
                                );
                              } catch (e) {
                                Uint8List signedData =
                                    await credentials.signPersonalMessage(
                                  txDataToUintList(data.data),
                                );
                                signedDataHex =
                                    bytesToHex(signedData, include0x: true);
                              }
                              _sendResult(
                                  "ethereum", signedDataHex, jsData.id ?? 0);
                            } catch (e) {
                              final error = e.toString().replaceAll('"', '\'');
                              _sendError("ethereum", error, jsData.id ?? 0);
                            } finally {
                              Navigator.pop(context);
                            }
                          },
                          onReject: () {
                            _sendError("ethereum", 'user rejected signature',
                                jsData.id ?? 0);
                            Navigator.pop(context);
                          },
                        );
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("ethereum", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  case "signTypedMessage":
                    {
                      final data =
                          JsEthSignTypedData.fromJson(jsData.object ?? {});

                      final typedChainId = BigInt.parse(
                              jsonDecode(data.raw)['domain']['chainId']
                                  .toString())
                          .toInt();

                      if (typedChainId != chainId) {
                        _sendError(
                          "ethereum",
                          "Provided chainId $typedChainId must match the active chainId $chainId",
                          jsData.id ?? 0,
                        );
                        return;
                      }

                      await signMessage(
                        context: context,
                        messageType: typedMessageSignKey,
                        data: data.raw,
                        networkIcon: null,
                        name: null,
                        onConfirm: () async {
                          try {
                            String signedDataHex = EthSigUtil.signTypedData(
                              privateKey: privateKey,
                              jsonData: data.raw,
                              version: TypedDataVersion.V4,
                            );
                            _sendResult(
                                "ethereum", signedDataHex, jsData.id ?? 0);
                          } catch (e) {
                            final error = e.toString().replaceAll('"', '\'');
                            _sendError("ethereum", error, jsData.id ?? 0);
                          } finally {
                            Navigator.pop(context);
                          }
                        },
                        onReject: () {
                          _sendError("ethereum", 'user rejected signature',
                              jsData.id ?? 0);
                          Navigator.pop(context);
                        },
                      );

                      break;
                    }
                  case "ecRecover":
                    {
                      final data =
                          JsEcRecoverObject.fromJson(jsData.object ?? {});

                      try {
                        final signature = EthSigUtil.recoverPersonalSignature(
                          message: txDataToUintList(data.message),
                          signature: data.signature,
                        );
                        _sendResult("ethereum", signature, jsData.id ?? 0);
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("ethereum", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  case "requestAccounts":
                    {
                      try {
                        final setAddress =
                            "window.ethereum.setAddress(\"$sendingAddress\");";

                        String callback =
                            "window.ethereum.sendResponse(${jsData.id}, [\"$sendingAddress\"])";

                        await _sendCustomResponse(setAddress);

                        await _sendCustomResponse(callback);
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("ethereum", error, jsData.id ?? 0);
                      }

                      break;
                    }
                  case "watchAsset":
                    {
                      final data = JsWatchAsset.fromJson(jsData.object ?? {});

                      try {
                        if (data.decimals == null) {
                          throw Exception(
                            'invalid asset decimals',
                          );
                        }
                        if (data.symbol == null) {
                          throw Exception(
                            'invalid asset symbol',
                          );
                        }
                        validateAddress(
                          {'rpc': blockChainDetails['rpc']},
                          data.contract,
                        );
                        final assetDetails = {
                          'name': data.symbol,
                          'symbol': data.symbol,
                          'decimals': data.decimals.toString(),
                          'contractAddress': data.contract,
                          'network': blockChainDetails['name'],
                          'rpc': blockChainDetails['rpc'],
                          'chainId': blockChainDetails['chainId'],
                          'coinType': blockChainDetails['coinType'],
                          'blockExplorer': blockChainDetails['blockExplorer'],
                        };
                        if (kDebugMode) {
                          print(assetDetails);
                        }
                        throw Exception('not Implemented');
                        // _sendResult("ethereum", '', jsData.id ?? 0);
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("ethereum", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  case "addEthereumChain":
                    {
                      final data =
                          JsAddEthereumChain.fromJson(jsData.object ?? {});

                      try {
                        final switchChainId =
                            BigInt.parse(data.chainId).toInt();

                        final currentChainIdData =
                            getEthereumDetailsFromChainId(chainId);

                        Map switchChainIdData =
                            getEthereumDetailsFromChainId(switchChainId);

                        if (chainId == switchChainId) {
                          _sendNull(
                            "ethereum",
                            jsData.id ?? 0,
                          );
                          return;
                        }

                        bool switchNetwork = true;
                        bool haveNotExecuted = true;

                        if (switchChainIdData == null) {
                          switchNetwork = false;
                          haveNotExecuted = false;
                          List blockExplorers = data.blockExplorerUrls;
                          String blockExplorer = '';
                          if (blockExplorers.isNotEmpty) {
                            blockExplorer = blockExplorers[0];
                            if (blockExplorer.endsWith('/')) {
                              blockExplorer = blockExplorer.substring(
                                  0, blockExplorer.length - 1);
                            }
                          }
                          List rpcUrl = data.rpcUrls;

                          Map addBlockChain = {};
                          if (pref.get(newEVMChainKey) != null) {
                            addBlockChain =
                                Map.from(jsonDecode(pref.get(newEVMChainKey)));
                          }

                          switchChainIdData = {
                            "rpc": rpcUrl.isNotEmpty ? rpcUrl[0] : null,
                            'chainId': switchChainId,
                            'blockExplorer': blockExplorers.isNotEmpty
                                ? '$blockExplorer/tx/$transactionhashTemplateKey'
                                : null,
                            'symbol': data.symbol,
                            'default': data.symbol,
                            'image': 'assets/ethereum-2.png',
                            'coinType': 60
                          };

                          Map details = {
                            data.chainName: switchChainIdData,
                          };
                          addBlockChain.addAll(details);

                          await addEthereumChain(
                            context: context,
                            jsonObj: json.encode(
                              Map.from({
                                'name': data.chainName,
                              })
                                ..addAll(switchChainIdData)
                                ..remove('image')
                                ..remove('coinType'),
                            ),
                            onConfirm: () async {
                              try {
                                const id = 83;
                                final response = await post(
                                  Uri.parse(switchChainIdData['rpc']),
                                  body: json.encode(
                                    {
                                      "jsonrpc": "2.0",
                                      "method": "eth_chainId",
                                      "params": [],
                                      "id": id
                                    },
                                  ),
                                  headers: {"Content-Type": "application/json"},
                                );
                                String responseBody = response.body;
                                if (response.statusCode ~/ 100 == 4 ||
                                    response.statusCode ~/ 100 == 5) {
                                  if (kDebugMode) {
                                    print(responseBody);
                                  }
                                  throw Exception(responseBody);
                                }

                                final jsonResponse = json.decode(responseBody);

                                final chainIdResponse =
                                    BigInt.parse(jsonResponse['result'])
                                        .toInt();

                                if (jsonResponse['id'] != id) {
                                  throw Exception('invalid eth_chainId');
                                } else if (chainIdResponse != switchChainId) {
                                  throw Exception(
                                      'chain Id different with eth_chainId');
                                }

                                await pref.put(
                                  newEVMChainKey,
                                  jsonEncode(addBlockChain),
                                );

                                switchNetwork = true;
                                Navigator.pop(context);
                              } catch (e) {
                                final error =
                                    e.toString().replaceAll('"', '\'');
                                _sendError("ethereum", error, jsData.id ?? 0);
                                Navigator.pop(context);
                              }
                            },
                            onReject: () async {
                              _sendError(
                                  "ethereum", 'canceled', jsData.id ?? 0);

                              Navigator.pop(context);
                            },
                          );
                        }
                        if (switchNetwork) {
                          final initString = _addChain(
                            switchChainId,
                            switchChainIdData['rpc'],
                            sendingAddress,
                          );
                          await _switchWeb3ChainRequest(
                            currentChainIdData: currentChainIdData,
                            switchChainIdData: switchChainIdData,
                            switchChainId: switchChainId,
                            initString: initString,
                            jsData: jsData,
                            haveNotExecuted: haveNotExecuted,
                          );
                        }
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("ethereum", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  case "switchEthereumChain":
                    {
                      final data =
                          JsAddEthereumChain.fromJson(jsData.object ?? {});
                      try {
                        final switchChainId =
                            BigInt.parse(data.chainId).toInt();

                        final currentChainIdData =
                            getEthereumDetailsFromChainId(chainId);

                        final switchChainIdData =
                            getEthereumDetailsFromChainId(switchChainId);

                        if (chainId == switchChainId) {
                          _sendNull(
                            "ethereum",
                            jsData.id ?? 0,
                          );

                          return;
                        }

                        if (switchChainIdData == null) {
                          _sendError(
                            "ethereum",
                            'unknown chain id',
                            jsData.id ?? 0,
                          );
                        } else {
                          final initString = _addChain(
                            switchChainId,
                            switchChainIdData['rpc'],
                            sendingAddress,
                          );
                          await _switchWeb3ChainRequest(
                            currentChainIdData: currentChainIdData,
                            switchChainIdData: switchChainIdData,
                            switchChainId: switchChainId,
                            initString: initString,
                            jsData: jsData,
                          );
                        }
                      } catch (e) {
                        final error = e.toString().replaceAll('"', '\'');
                        _sendError("ethereum", error, jsData.id ?? 0);
                      }
                      break;
                    }
                  default:
                    {
                      _sendError(jsData.network.toString(),
                          "Operation not supported", jsData.id ?? 0);
                      break;
                    }
                }
              }
            });

        if (Util.isAndroid()) {
          controller.startSafeBrowsing();
        }

        widget.webViewModel.settings = await controller.getSettings();

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
        } else if (widget.webViewModel.needsToCompleteInitialLoad) {
          controller.stopLoading();
        }
      },
      onLoadStop: (controller, url) async {
        _pullToRefreshController?.endRefreshing();

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
                          .toString()
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
          widget.webViewModel.needsToCompleteInitialLoad = false;
          currentWebViewModel.updateWithValue(widget.webViewModel);

          var screenshotData = _webViewController
              ?.takeScreenshot(
                  screenshotConfiguration: ScreenshotConfiguration(
                      compressFormat: CompressFormat.JPEG, quality: 20))
              .timeout(
                const Duration(milliseconds: 1500),
                onTimeout: () => null,
              );
          widget.webViewModel.screenshot = await screenshotData;
        }
      },
      onProgressChanged: (controller, progress) {
        if (progress == 100) {
          _pullToRefreshController?.endRefreshing();
        }

        widget.webViewModel.progress = progress / 100;

        if (isCurrentTab(currentWebViewModel)) {
          currentWebViewModel.updateWithValue(widget.webViewModel);
        }
      },
      onUpdateVisitedHistory: (controller, url, androidIsReload) async {
        widget.webViewModel.url = url;
        widget.webViewModel.title = await _webViewController?.getTitle();

        if (isCurrentTab(currentWebViewModel)) {
          currentWebViewModel.updateWithValue(widget.webViewModel);
        }
      },
      onLongPressHitTestResult: (controller, hitTestResult) async {
        if (LongPressAlertDialog.hitTestResultSupported
            .contains(hitTestResult.type)) {
          var requestFocusNodeHrefResult =
              await _webViewController?.requestFocusNodeHref();

          if (requestFocusNodeHrefResult != null) {
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
        } else if (consoleMessage.messageLevel == ConsoleMessageLevel.WARNING) {
          consoleBackgroundColor = const Color.fromRGBO(255, 251, 227, 1);
          consoleIconData = Icons.report_problem;
          consoleIconColor = Colors.orangeAccent;
        }

        widget.webViewModel.addJavaScriptConsoleResults(JavaScriptConsoleResult(
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
      shouldOverrideUrlLoading: (controller, navigationAction) async {
        Uri url = navigationAction.request.url;
        String url_ = url.toString();
        // print('url clicked $url_');
        List<String> allowedAction = [
          "http",
          "https",
          "file",
          "chrome",
          "data",
          "javascript",
          "about"
        ];

        if (!kIsWeb && defaultTargetPlatform == TargetPlatform.iOS) {
          final shouldPerformDownload =
              navigationAction.shouldPerformDownload ?? false;
          final url = navigationAction.request.url;
          if (shouldPerformDownload && url != null) {
            await downloadFile(url.toString());
            return NavigationActionPolicy.DOWNLOAD;
          }
        }
        if (url_.contains('wz://add-rpc')) {
          await getWZlinks_(url_);
          return NavigationActionPolicy.CANCEL;
        }
        if (url_.contains('wc?uri=')) {
          final wcUri = Uri.parse(
            Uri.decodeFull(
              Uri.parse(url_).queryParameters['uri'],
            ),
          );

          final session = WCSession.from(wcUri.toString());

          if (session != WCSession.empty()) {
            await WcConnector.qrScanHandler(url_);
          } else {
            await WcConnector.wcReconnect();
            // await wcReconnect();
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
      onDownloadStartRequest: (controller, url) async {
        String path = url.url.path;
        String fileName = path.substring(path.lastIndexOf('/') + 1);

        await FlutterDownloader.enqueue(
          url: url.toString(),
          fileName: fileName,
          savedDir: (await getTemporaryDirectory()).path,
          showNotification: true,
          openFileFromNotification: true,
        );
      },
      onReceivedServerTrustAuthRequest: (controller, challenge) async {
        var sslError = challenge.protectionSpace.sslError;
        if (sslError != null && (sslError.code != null)) {
          if (Util.isIOS() && sslError.code == SslErrorType.UNSPECIFIED) {
            return ServerTrustAuthResponse(
                action: ServerTrustAuthResponseAction.PROCEED);
          }
          widget.webViewModel.isSecure = false;
          if (isCurrentTab(currentWebViewModel)) {
            currentWebViewModel.updateWithValue(widget.webViewModel);
          }
          return ServerTrustAuthResponse(
              action: ServerTrustAuthResponseAction.CANCEL);
        }
        return ServerTrustAuthResponse(
            action: ServerTrustAuthResponseAction.PROCEED);
      },
      onReceivedError: (controller, request, error) async {
        var isForMainFrame = request.isForMainFrame ?? false;
        if (!isForMainFrame) {
          return;
        }

        _pullToRefreshController?.endRefreshing();

        if (Util.isIOS() && error.type == WebResourceErrorType.CANCELLED) {
          // NSURLErrorDomain
          return;
        }

        var errorUrl = request.url;

        _webViewController?.loadData(data: """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, user-scalable=no, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <style>
    ${await InAppWebViewController.tRexRunnerCss}
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
    ${await InAppWebViewController.tRexRunnerHtml}
    <div class="interstitial-wrapper">
      <h1>Website not available</h1>
      <p>Could not load web pages at <strong>$errorUrl</strong> because:</p>
      <p>${error.description}</p>
    </div>
</body>
    """, baseUrl: errorUrl, historyUrl: errorUrl);

        widget.webViewModel.url = errorUrl;
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
              url: WebUri("about:blank"),
              windowId: createWindowRequest.windowId),
        );

        browserModel.addTab(webViewTab);

        return true;
      },
      onCloseWindow: (controller) {
        if (_isWindowClosed) {
          return;
        }
        _isWindowClosed = true;
        if (widget.webViewModel.tabIndex != null) {
          browserModel.closeTab(widget.webViewModel.tabIndex);
        }
      },
      onPermissionRequest: (controller, permissionRequest) async {
        return PermissionResponse(
            resources: permissionRequest.resources,
            action: PermissionResponseAction.GRANT);
      },
      onReceivedHttpAuthRequest: (controller, challenge) async {
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

  Future<HttpAuthResponseAction> createHttpAuthDialog(
      URLAuthenticationChallenge challenge) async {
    HttpAuthResponseAction action = HttpAuthResponseAction.CANCEL;

    await showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text("Login"),
          content: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: <Widget>[
              Text(challenge.protectionSpace.host),
              TextField(
                decoration: const InputDecoration(labelText: "Username"),
                controller: _httpAuthUsernameController,
              ),
              TextField(
                decoration: const InputDecoration(labelText: "Password"),
                controller: _httpAuthPasswordController,
                obscureText: true,
              ),
            ],
          ),
          actions: <Widget>[
            ElevatedButton(
              child: const Text("Cancel"),
              onPressed: () {
                action = HttpAuthResponseAction.CANCEL;
                Navigator.of(context).pop();
              },
            ),
            ElevatedButton(
              child: const Text("Ok"),
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

  void onShowTab() async {
    resume();
    if (widget.webViewModel.needsToCompleteInitialLoad) {
      widget.webViewModel.needsToCompleteInitialLoad = false;
      await widget.webViewModel.webViewController
          ?.loadUrl(urlRequest: URLRequest(url: widget.webViewModel.url));
    }
  }

  Future<WebNotificationPermission> onNotificationRequestPermission() async {
    final url = await _webViewController.getUrl();

    if (url != null) {
      final savedPermission =
          WebNotificationPermissionDb.getPermission(url.host);
      if (savedPermission != null) {
        return savedPermission;
      }
    }

    final permission = await showDialog<WebNotificationPermission>(
          context: context,
          builder: (context) {
            return AlertDialog(
              title: Text('${url?.host} wants to show notifications'),
              actions: [
                ElevatedButton(
                    onPressed: () {
                      Navigator.pop<WebNotificationPermission>(
                          context, WebNotificationPermission.DENIED);
                    },
                    child: const Text('Deny')),
                ElevatedButton(
                    onPressed: () {
                      Navigator.pop<WebNotificationPermission>(
                          context, WebNotificationPermission.GRANTED);
                    },
                    child: const Text('Grant'))
              ],
            );
          },
        ) ??
        WebNotificationPermission.DENIED;

    if (url != null) {
      await WebNotificationPermissionDb.savePermission(url.host, permission);
    }

    return permission;
  }

  void onShowNotification(WebNotification notification) async {
    webNotificationController?.notifications[notification.id] = notification;

    Uri iconUrl =
        notification.icon != null && notification.icon.trim().isNotEmpty
            ? Uri.tryParse(notification.icon)
            : null;
    if (iconUrl != null && !iconUrl.hasScheme) {
      iconUrl = Uri.tryParse(
          (await _webViewController?.getUrl()).toString() + iconUrl.toString());
    }

    await NotificationApi.showNotification(
      id: notification.id,
      title: notification.title,
      body: notification.body,
      imageUrl: iconUrl.toString(),
      onclick: (payload) async {
        await notification.dispatchClick();
      },
      onclose: () async {
        await notification.close();
      },
    );

    final vibrate = notification.vibrate;
    final hasVibrator = await Vibration.hasVibrator() ?? false;
    if (hasVibrator && vibrate != null && vibrate.isNotEmpty) {
      if (vibrate.length % 2 != 0) {
        vibrate.add(0);
      }
      final intensities = <int>[];
      for (int i = 0; i < vibrate.length; i++) {
        if (i % 2 == 0 && vibrate[i] > 0) {
          intensities.add(255);
        } else {
          intensities.add(0);
        }
      }
      await Vibration.vibrate(pattern: vibrate, intensities: intensities);
    }
  }

  resetNotificationPermission() async {
    await WebNotificationPermissionDb.clear();
    await webNotificationController?.resetPermission();
  }

  void onCloseNotification(int id) async {
    final notification = webNotificationController?.notifications[id];
    await NotificationApi.closeNotification(id: id);
    if (notification != null) {
      await NotificationApi.closeNotification(id: id);
      webNotificationController?.notifications?.remove(id);
    }
  }

  void onHideTab() async {
    pause();
  }
}
