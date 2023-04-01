import 'dart:collection';
import 'dart:convert';
import 'dart:isolate';
import 'dart:math';
import 'dart:ui';
import 'package:cryptowallet/api/notification_api.dart';
import 'package:cryptowallet/components/user_details_placeholder.dart';
import 'package:cryptowallet/models/provider.dart';
import 'package:cryptowallet/utils/wallet_black.dart';
import 'package:cryptowallet/utils/wc_connector.dart';
import 'package:flutter_downloader/flutter_downloader.dart';

import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:eth_sig_util/eth_sig_util.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:http/http.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';
import 'package:vibration/vibration.dart';
import 'package:wallet_connect/wallet_connect.dart';
import 'package:web3dart/crypto.dart';
import 'package:web3dart/web3dart.dart';

import 'package:cryptowallet/utils/qr_scan_view.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';

import '../components/loader.dart';
import '../screens/main_screen.dart';
import '../screens/security.dart';
import '../screens/send_token.dart';
import '../screens/transfer_token.dart';
import '../screens/view_wallets.dart';
import '../screens/wallet_main_body.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import '../utils/app_config.dart';
import '../utils/web_notifications.dart';
import 'js_model.dart';
import 'dart:collection';
import 'dart:convert';
import 'dart:ffi';
import 'dart:isolate';
import 'dart:ui';
import 'package:cryptowallet/api/notification_api.dart';
import 'package:cryptowallet/utils/wc_connector.dart';
import 'package:flutter_downloader/flutter_downloader.dart';
import 'package:solana/solana.dart' as solana;
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:eth_sig_util/eth_sig_util.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:hex/hex.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:http/http.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:url_launcher/url_launcher.dart';
import 'package:vibration/vibration.dart';
import 'package:wallet_connect/wallet_connect.dart';
import 'package:web3dart/crypto.dart';
import 'package:web3dart/web3dart.dart';

class WebViewTab extends StatefulWidget {
  final String url;
  final int windowId;
  final String provider;
  final String init;
  final String data;
  final String webNotifier;

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

  FocusNode get focus {
    final state = (key as GlobalKey).currentState as _WebViewTabState;
    return state?._focus;
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

  const WebViewTab({
    GlobalKey key,
    this.url,
    this.onStateUpdated,
    this.onCloseTabRequested,
    this.onCreateTabRequested,
    this.data,
    this.provider,
    this.webNotifier,
    this.init,
    this.windowId,
  })  : assert(url != null || windowId != null),
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
  dynamic viewId = 0;
  final _browserController = TextEditingController();
  Uint8List _screenshot;
  String _url = '';
  bool _isSecure;
  String _title = '';
  Favicon _favicon;
  double _progress = 0;
  bool webLoadin = false;
  String initJs = '';
  bool isFocused = false;
  bool checkProfile = false;
  String tabProfile = pref.get(currentMmenomicKey);
  int tabChainId = 1;
  // Map chainDetails = getEthereumDetailsFromChainId(1);
  FocusNode _focus;
  final jsonNotification =
      jsonEncode(WebNotificationPermissionDb.getPermissions());
  WebNotificationController webNotificationController;
  List<UserScript> webNotification;

  final ReceivePort _port = ReceivePort();
  final TextEditingController _httpAuthUsernameController =
      TextEditingController();
  final TextEditingController _httpAuthPasswordController =
      TextEditingController();
  PullToRefreshController _pullToRefreshController;

  @override
  void initState() {
    super.initState();
    _focus = FocusNode();
    _focus.addListener(_onFocusChange);
    WidgetsBinding.instance.addPostFrameCallback((_) {
      widget.onStateUpdated.call();
    });

    initJs = widget.init;
    webNotification = [
      UserScript(
          source: widget.webNotifier,
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
    WidgetsBinding.instance.addObserver(this);
    IsolateNameServer.registerPortWithName(
        _port.sendPort, 'downloader_send_port');
    _port.listen((dynamic data) {
      String id = data[0];
      DownloadTaskStatus status = data[1];
      int progress = data[2];
      if (kDebugMode) {
        print("Download progress: $progress%");
      }
      if (status == DownloadTaskStatus.complete) {
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(
          content: Text("Download $id completed!"),
        ));
      }
    });
    _pullToRefreshController = kIsWeb
        ? null
        : PullToRefreshController(
            settings: PullToRefreshSettings(color: Colors.blue),
            onRefresh: () async {
              if (defaultTargetPlatform == TargetPlatform.android) {
                _controller?.reload();
              } else if (defaultTargetPlatform == TargetPlatform.iOS) {
                _controller?.loadUrl(
                    urlRequest: URLRequest(url: await _controller?.getUrl()));
              }
            },
          );
    FlutterDownloader.registerCallback(downloadCallback);
    _url = widget.url ?? '';
  }

  void loadUserData(bool firstTime) {
    if (kDebugMode) {
      print('windowId inside loadUserData ${viewId}');
    }

    // Future.delayed(Duration.zero, () {
    final tabUserData = Provider.of<ProviderClass>(context, listen: false);

    if (firstTime) {
      tabUserData.setDefaultProfile();
      tabUserData.addTabUserCred(viewId);
    }

    // });
    setState(() {
      tabUserData.setCurrentWindowId(viewId);
      tabProfile = tabUserData.tabUserCred[viewId]['profile'];
      tabChainId = tabUserData.tabUserCred[viewId]['chain'];

      tabUserData.setCheckAnyProfilePresent();
      checkProfile = tabUserData.checkAnyProfilePresent;
      // getChainDetails(tabChainId);
    });
  }

  // void getChainDetails(int chainId) {
  //   chainDetails = getEthereumDetailsFromChainId(chainId);
  //   print('getChainDetails-> ${chainDetails['image']}');
  // }

  void setRedirectUrl(
      InAppWebViewController controller, dynamic redirectUrl) async {
    Uri uri = blockChainToHttps(redirectUrl.trim());
    await controller.loadUrl(urlRequest: URLRequest(url: WebUri.uri(uri)));
  }

  @pragma('vm:entry-point')
  static void downloadCallback(
      String id, DownloadTaskStatus status, int progress) {
    final SendPort send =
        IsolateNameServer.lookupPortByName('downloader_send_port');
    send?.send([id, status, progress]);
  }

  @override
  void dispose() {
    _controller = null;
    WidgetsBinding.instance.removeObserver(this);
    IsolateNameServer.removePortNameMapping('downloader_send_port');
    _httpAuthUsernameController.dispose();
    _httpAuthPasswordController.dispose();
    _focus.removeListener(_onFocusChange);
    _focus.dispose();
    super.dispose();
  }

  void _onFocusChange() {
    isFocused = _focus.hasFocus;
    debugPrint("Focus: ${_focus.hasFocus.toString()}");
    setState(() {});
  }

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
    return _controller.evaluateJavascript(source: script);
  }

  Future<void> _sendResult(String network, String message, int methodId) {
    String script = "window.$network.sendResponse($methodId, \"$message\")";
    debugPrint(script);
    return _controller
        .evaluateJavascript(source: script)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
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
    return _controller
        .evaluateJavascript(source: script)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
  }

  Future<void> _sendNull(String network, int methodId) {
    String script = "window.$network.sendResponse($methodId, null)";
    debugPrint(script);
    return _controller
        .evaluateJavascript(source: script)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
  }

  Future<void> _sendCustomResponse(String response) {
    return _controller
        .evaluateJavascript(source: response)
        .then((value) => debugPrint(value))
        .onError((error, stackTrace) => debugPrint(error.toString()));
  }

  Future<void> _sendResults(
      String network, List<String> messages, int methodId) {
    String message = messages.join(",");
    String script = "window.$network.sendResponse($methodId, \"$message\")";
    return _controller.evaluateJavascript(source: script);
  }

  changeBrowserChainId_(int chainId, String rpc) async {
    if (_controller == null) return;
    initJs = await changeBlockChainAndReturnInit(
      chainId,
      rpc,
    );

    await _controller?.removeAllUserScripts();
    await _controller?.addUserScripts(userScripts: [
      UserScript(
        source: '${widget.provider}$initJs',
        injectionTime: UserScriptInjectionTime.AT_DOCUMENT_START,
      ),
      ...webNotification
    ]);
    await _controller?.reload();
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

  @override
  Widget build(BuildContext context) {
    // print('tabChainId from webViewTab $tabChainId');
    // print('tabProfile from webViewTab $tabProfile');
    Box pref = Hive.box(secureStorageKey);
    int index = webLoadin ? 1 : 0;
    if (isFocused) index = 2;
    List historyData = [];
    String history = pref.get(historyKey);
    if (history != null) {
      historyData = json.decode(history);
    }

    return Consumer<ProviderClass>(builder: (context, state, child) {
      print('viewId inside webviewTab $viewId');
      if (state.tabUserCred[viewId] != null) {
        tabProfile = state.tabUserCred[viewId]['profile'];
        tabChainId = state.tabUserCred[viewId]['chain'];
        print(
            'tabChainId from webViewTab ${state.tabUserCred[viewId]['profile']}');
        print(
            'tabProfile from webViewTab ${state.tabUserCred[viewId]['chain']}');
      }

      return Scaffold(
        body: SafeArea(
          child: IndexedStack(
            index: index,
            children: [
              const WalletBlack(),
              Column(children: <Widget>[
                Expanded(
                  child: Stack(
                    children: [
                      Container(
                        color: Colors.white,
                      ),
                      InAppWebView(
                        windowId: widget.windowId,
                        pullToRefreshController: _pullToRefreshController,
                        initialSettings: InAppWebViewSettings(
                          useShouldOverrideUrlLoading: true,
                          forceDark:
                              Theme.of(context).brightness == Brightness.dark
                                  ? ForceDark.ON
                                  : ForceDark.OFF,
                          javaScriptCanOpenWindowsAutomatically: true,
                          supportMultipleWindows: true,
                          isFraudulentWebsiteWarningEnabled: true,
                          safeBrowsingEnabled: true,
                          mediaPlaybackRequiresUserGesture: false,
                          allowsInlineMediaPlayback: true,
                          useOnDownloadStart: true,
                          useHybridComposition: true,
                        ),
                        onPermissionRequest: (controller, request) async {
                          final resources = <PermissionResourceType>[];
                          if (request.resources
                              .contains(PermissionResourceType.CAMERA)) {
                            final cameraStatus =
                                await Permission.camera.request();
                            if (!cameraStatus.isDenied) {
                              resources.add(PermissionResourceType.CAMERA);
                            }
                          }
                          if (request.resources
                              .contains(PermissionResourceType.MICROPHONE)) {
                            final microphoneStatus =
                                await Permission.microphone.request();
                            if (!microphoneStatus.isDenied) {
                              resources.add(PermissionResourceType.MICROPHONE);
                            }
                          }
                          // only for iOS and macOS
                          if (request.resources.contains(
                              PermissionResourceType.CAMERA_AND_MICROPHONE)) {
                            final cameraStatus =
                                await Permission.camera.request();
                            final microphoneStatus =
                                await Permission.microphone.request();
                            if (!cameraStatus.isDenied &&
                                !microphoneStatus.isDenied) {
                              resources.add(
                                  PermissionResourceType.CAMERA_AND_MICROPHONE);
                            }
                          }

                          return PermissionResponse(
                            resources: resources,
                            action: resources.isEmpty
                                ? PermissionResponseAction.DENY
                                : PermissionResponseAction.GRANT,
                          );
                        },
                        onUpdateVisitedHistory: (controller, url, isReload) {
                          if (url != null) {
                            _url = url.toString();

                            _browserController.text = _url;
                            widget.onStateUpdated.call();
                          }
                        },
                        onDownloadStartRequest:
                            (controller, downloadStartRequest) async {
                          await downloadFile(
                            downloadStartRequest.url.toString(),
                            downloadStartRequest.suggestedFilename,
                          );
                        },
                        shouldOverrideUrlLoading: (
                          InAppWebViewController controller,
                          NavigationAction shouldOverrideUrl,
                        ) async {
                          Uri url = shouldOverrideUrl.request.url;
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

                          if (!kIsWeb &&
                              defaultTargetPlatform == TargetPlatform.iOS) {
                            final shouldPerformDownload =
                                shouldOverrideUrl.shouldPerformDownload ??
                                    false;
                            final url = shouldOverrideUrl.request.url;
                            if (shouldPerformDownload && url != null) {
                              await downloadFile(url.toString());
                              return NavigationActionPolicy.DOWNLOAD;
                            }
                          }
                          if (url_.contains('wc?uri=')) {
                            final wcUri = Uri.parse(
                              Uri.decodeFull(
                                Uri.parse(url_).queryParameters['uri'],
                              ),
                            );

                            final session = WCSession.from(wcUri.toString());

                            if (session != WCSession.empty()) {
                              if (!checkProfile) {
                                await showModalCreateWallet();
                              } else {
                                await WcConnector.qrScanHandler(url_);
                              }
                            } else {
                              await WcConnector.wcReconnect();
                              // await wcReconnect();
                            }

                            return NavigationActionPolicy.CANCEL;
                          } else if (url_.startsWith('wc:')) {
                            final session = WCSession.from(url_);

                            if (session != WCSession.empty()) {
                              if (!checkProfile) {
                                await showModalCreateWallet();
                              } else {
                                await WcConnector.qrScanHandler(url_);
                              }
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
                          webNotificationController =
                              WebNotificationController(controller);
                          _controller.addJavaScriptHandler(
                            handlerName: 'Notification.requestPermission',
                            callback: (arguments) async {
                              final permission =
                                  await onNotificationRequestPermission();
                              return permission.name.toLowerCase();
                            },
                          );

                          _controller.addJavaScriptHandler(
                            handlerName: 'Notification.show',
                            callback: (arguments) {
                              if (_controller != null) {
                                final notification = WebNotification.fromJson(
                                    arguments[0], _controller);
                                onShowNotification(notification);
                              }
                            },
                          );

                          _controller.addJavaScriptHandler(
                            handlerName: 'Notification.close',
                            callback: (arguments) {
                              final notificationId = arguments[0];
                              onCloseNotification(notificationId);
                            },
                          );

                          _controller.addJavaScriptHandler(
                              handlerName: 'CryptoHandler',
                              callback: (callback) async {
                                final jsData = JsCallbackModel.fromJson(
                                    json.decode(callback[0]));

                                final mnemonic = pref.get(currentMmenomicKey);
                                if (jsData.network == 'solana') {
                                  final solanaResponse =
                                      await getSolanaFromMemnomic(mnemonic);
                                  final sendingAddress =
                                      solanaResponse['address'];

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
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError(
                                              "solana", error, jsData.id ?? 0);
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
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError(
                                              "solana", error, jsData.id ?? 0);
                                        }
                                        break;
                                      }
                                    case "signRawTransactionMulti":
                                      {
                                        final data =
                                            JsSolanaTransactionObject.fromJson(
                                                jsData.object ?? {});

                                        try {} catch (e) {
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError(
                                              "solana", error, jsData.id ?? 0);
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
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError(
                                              "solana", error, jsData.id ?? 0);
                                        }

                                        break;
                                      }
                                  }
                                } else if (jsData.network == 'ethereum') {
                                  int chainId = pref.get(dappChainIdKey);

                                  final blockChainDetails =
                                      getEthereumDetailsFromChainId(chainId);
                                  final rpc = blockChainDetails['rpc'];
                                  final web3Response =
                                      await getEthereumFromMemnomic(
                                    mnemonic,
                                    blockChainDetails['coinType'],
                                  );

                                  final privateKey =
                                      web3Response['eth_wallet_privateKey'];
                                  final credentials =
                                      EthPrivateKey.fromHex(privateKey);

                                  final sendingAddress =
                                      web3Response['eth_wallet_address'];
                                  switch (jsData.name) {
                                    case "signTransaction":
                                      {
                                        final data =
                                            JsTransactionObject.fromJson(
                                                jsData.object ?? {});
                                        await signTransaction(
                                          gasPriceInWei_: null,
                                          to: data.to,
                                          from: sendingAddress,
                                          txData: data.data,
                                          valueInWei_: data.value,
                                          gasInWei_: null,
                                          networkIcon: null,
                                          context: context,
                                          blockChainCurrencySymbol:
                                              blockChainDetails['symbol'],
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
                                                      ? EthereumAddress.fromHex(
                                                          data.to)
                                                      : null,
                                                  value: data.value != null
                                                      ? EtherAmount.inWei(
                                                          BigInt.parse(
                                                              data.value),
                                                        )
                                                      : null,
                                                  nonce: data.nonce != null
                                                      ? int.parse(data.nonce)
                                                      : null,
                                                  data: txDataToUintList(
                                                      data.data),
                                                  gasPrice: data.gasPrice !=
                                                          null
                                                      ? EtherAmount.inWei(
                                                          BigInt.parse(
                                                              data.gasPrice))
                                                      : null,
                                                ),
                                                chainId: chainId,
                                              );

                                              final response = await client
                                                  .sendRawTransaction(
                                                      signedTransaction);

                                              _sendResult("ethereum", response,
                                                  jsData.id ?? 0);
                                            } catch (e) {
                                              final error = e
                                                  .toString()
                                                  .replaceAll('"', '\'');
                                              _sendError("ethereum", error,
                                                  jsData.id ?? 0);
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
                                        final data = JsDataModel.fromJson(
                                            jsData.object ?? {});

                                        await signMessage(
                                          context: context,
                                          messageType: personalSignKey,
                                          data: data.data,
                                          networkIcon: null,
                                          name: null,
                                          onConfirm: () async {
                                            try {
                                              List signedData =
                                                  await credentials
                                                      .signPersonalMessage(
                                                txDataToUintList(data.data),
                                              );

                                              _sendResult(
                                                "ethereum",
                                                bytesToHex(signedData,
                                                    include0x: true),
                                                jsData.id ?? 0,
                                              );
                                            } catch (e) {
                                              final error = e
                                                  .toString()
                                                  .replaceAll('"', '\'');
                                              _sendError("ethereum", error,
                                                  jsData.id ?? 0);
                                            } finally {
                                              Navigator.pop(context);
                                            }
                                          },
                                          onReject: () {
                                            _sendError(
                                                "ethereum",
                                                'user rejected signature',
                                                jsData.id ?? 0);
                                            Navigator.pop(context);
                                          },
                                        );
                                        break;
                                      }
                                    case "signMessage":
                                      {
                                        try {
                                          final data = JsDataModel.fromJson(
                                              jsData.object ?? {});

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
                                                  signedDataHex =
                                                      EthSigUtil.signMessage(
                                                    privateKey: privateKey,
                                                    message: txDataToUintList(
                                                        data.data),
                                                  );
                                                } catch (e) {
                                                  Uint8List signedData =
                                                      await credentials
                                                          .signPersonalMessage(
                                                    txDataToUintList(data.data),
                                                  );
                                                  signedDataHex = bytesToHex(
                                                      signedData,
                                                      include0x: true);
                                                }
                                                _sendResult(
                                                    "ethereum",
                                                    signedDataHex,
                                                    jsData.id ?? 0);
                                              } catch (e) {
                                                final error = e
                                                    .toString()
                                                    .replaceAll('"', '\'');
                                                _sendError("ethereum", error,
                                                    jsData.id ?? 0);
                                              } finally {
                                                Navigator.pop(context);
                                              }
                                            },
                                            onReject: () {
                                              _sendError(
                                                  "ethereum",
                                                  'user rejected signature',
                                                  jsData.id ?? 0);
                                              Navigator.pop(context);
                                            },
                                          );
                                        } catch (e) {
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError("ethereum", error,
                                              jsData.id ?? 0);
                                        }
                                        break;
                                      }
                                    case "signTypedMessage":
                                      {
                                        final data =
                                            JsEthSignTypedData.fromJson(
                                                jsData.object ?? {});

                                        final typedChainId = BigInt.parse(
                                                jsonDecode(data.raw)['domain']
                                                        ['chainId']
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
                                              String signedDataHex =
                                                  EthSigUtil.signTypedData(
                                                privateKey: privateKey,
                                                jsonData: data.raw,
                                                version: TypedDataVersion.V4,
                                              );
                                              _sendResult(
                                                  "ethereum",
                                                  signedDataHex,
                                                  jsData.id ?? 0);
                                            } catch (e) {
                                              final error = e
                                                  .toString()
                                                  .replaceAll('"', '\'');
                                              _sendError("ethereum", error,
                                                  jsData.id ?? 0);
                                            } finally {
                                              Navigator.pop(context);
                                            }
                                          },
                                          onReject: () {
                                            _sendError(
                                                "ethereum",
                                                'user rejected signature',
                                                jsData.id ?? 0);
                                            Navigator.pop(context);
                                          },
                                        );

                                        break;
                                      }
                                    case "ecRecover":
                                      {
                                        final data = JsEcRecoverObject.fromJson(
                                            jsData.object ?? {});

                                        try {
                                          final signature = EthSigUtil
                                              .recoverPersonalSignature(
                                            message:
                                                txDataToUintList(data.message),
                                            signature: data.signature,
                                          );
                                          _sendResult("ethereum", signature,
                                              jsData.id ?? 0);
                                        } catch (e) {
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError("ethereum", error,
                                              jsData.id ?? 0);
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
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError("ethereum", error,
                                              jsData.id ?? 0);
                                        }

                                        break;
                                      }
                                    case "watchAsset":
                                      {
                                        final data = JsWatchAsset.fromJson(
                                            jsData.object ?? {});

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
                                            'decimals':
                                                data.decimals.toString(),
                                            'contractAddress': data.contract,
                                            'network':
                                                blockChainDetails['name'],
                                            'rpc': blockChainDetails['rpc'],
                                            'chainId':
                                                blockChainDetails['chainId'],
                                            'coinType':
                                                blockChainDetails['coinType'],
                                            'blockExplorer': blockChainDetails[
                                                'blockExplorer'],
                                          };
                                          if (kDebugMode) {
                                            print(assetDetails);
                                          }
                                          throw Exception('not Implemented');
                                          // _sendResult("ethereum", '', jsData.id ?? 0);
                                        } catch (e) {
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError("ethereum", error,
                                              jsData.id ?? 0);
                                        }
                                        break;
                                      }
                                    case "addEthereumChain":
                                      {
                                        final data =
                                            JsAddEthereumChain.fromJson(
                                                jsData.object ?? {});

                                        try {
                                          final switchChainId =
                                              BigInt.parse(data.chainId)
                                                  .toInt();

                                          final currentChainIdData =
                                              getEthereumDetailsFromChainId(
                                                  chainId);

                                          Map switchChainIdData =
                                              getEthereumDetailsFromChainId(
                                                  switchChainId);

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
                                            List blockExplorers =
                                                data.blockExplorerUrls;
                                            String blockExplorer = '';
                                            if (blockExplorers.isNotEmpty) {
                                              blockExplorer = blockExplorers[0];
                                              if (blockExplorer.endsWith('/')) {
                                                blockExplorer =
                                                    blockExplorer.substring(
                                                        0,
                                                        blockExplorer.length -
                                                            1);
                                              }
                                            }
                                            List rpcUrl = data.rpcUrls;

                                            Map addBlockChain = {};
                                            if (pref.get(newEVMChainKey) !=
                                                null) {
                                              addBlockChain = Map.from(
                                                  jsonDecode(pref
                                                      .get(newEVMChainKey)));
                                            }

                                            switchChainIdData = {
                                              "rpc": rpcUrl.isNotEmpty
                                                  ? rpcUrl[0]
                                                  : null,
                                              'chainId': switchChainId,
                                              'blockExplorer': blockExplorers
                                                      .isNotEmpty
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
                                                    Uri.parse(switchChainIdData[
                                                        'rpc']),
                                                    body: json.encode(
                                                      {
                                                        "jsonrpc": "2.0",
                                                        "method": "eth_chainId",
                                                        "params": [],
                                                        "id": id
                                                      },
                                                    ),
                                                    headers: {
                                                      "Content-Type":
                                                          "application/json"
                                                    },
                                                  );
                                                  String responseBody =
                                                      response.body;
                                                  if (response.statusCode ~/
                                                              100 ==
                                                          4 ||
                                                      response.statusCode ~/
                                                              100 ==
                                                          5) {
                                                    if (kDebugMode) {
                                                      print(responseBody);
                                                    }
                                                    throw Exception(
                                                        responseBody);
                                                  }

                                                  final jsonResponse =
                                                      json.decode(responseBody);

                                                  final chainIdResponse =
                                                      BigInt.parse(jsonResponse[
                                                              'result'])
                                                          .toInt();

                                                  if (jsonResponse['id'] !=
                                                      id) {
                                                    throw Exception(
                                                        'invalid eth_chainId');
                                                  } else if (chainIdResponse !=
                                                      switchChainId) {
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
                                                  final error = e
                                                      .toString()
                                                      .replaceAll('"', '\'');
                                                  _sendError("ethereum", error,
                                                      jsData.id ?? 0);
                                                  Navigator.pop(context);
                                                }
                                              },
                                              onReject: () async {
                                                _sendError("ethereum",
                                                    'canceled', jsData.id ?? 0);

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
                                              currentChainIdData:
                                                  currentChainIdData,
                                              switchChainIdData:
                                                  switchChainIdData,
                                              switchChainId: switchChainId,
                                              initString: initString,
                                              jsData: jsData,
                                              haveNotExecuted: haveNotExecuted,
                                            );
                                          }
                                        } catch (e) {
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError("ethereum", error,
                                              jsData.id ?? 0);
                                        }
                                        break;
                                      }
                                    case "switchEthereumChain":
                                      {
                                        final data =
                                            JsAddEthereumChain.fromJson(
                                                jsData.object ?? {});
                                        try {
                                          final switchChainId =
                                              BigInt.parse(data.chainId)
                                                  .toInt();

                                          final currentChainIdData =
                                              getEthereumDetailsFromChainId(
                                                  chainId);

                                          final switchChainIdData =
                                              getEthereumDetailsFromChainId(
                                                  switchChainId);

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
                                              currentChainIdData:
                                                  currentChainIdData,
                                              switchChainIdData:
                                                  switchChainIdData,
                                              switchChainId: switchChainId,
                                              initString: initString,
                                              jsData: jsData,
                                            );
                                          }
                                        } catch (e) {
                                          final error = e
                                              .toString()
                                              .replaceAll('"', '\'');
                                          _sendError("ethereum", error,
                                              jsData.id ?? 0);
                                        }
                                        break;
                                      }
                                    default:
                                      {
                                        _sendError(
                                            jsData.network.toString(),
                                            "Operation not supported",
                                            jsData.id ?? 0);
                                        break;
                                      }
                                  }
                                }
                              });
                        },
                        initialUserScripts: UnmodifiableListView([
                          UserScript(
                            source: '${widget.provider}$initJs',
                            injectionTime:
                                UserScriptInjectionTime.AT_DOCUMENT_START,
                          ),
                          ...webNotification
                        ]),
                        onLoadStart:
                            (InAppWebViewController controller, Uri url) async {
                          setState(() {
                            webLoadin = true;
                          });
                          // setUserAgent(controller);
                          // await controller.setSettings(
                          //   settings: InAppWebViewSettings(
                          //       userAgent:
                          //           'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36'),
                          // );

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
                        onTitleChanged: (controller, title) {
                          _title = title ?? '';
                          widget.onStateUpdated.call();
                        },
                        onProgressChanged: (controller, progress) {
                          setState(() {
                            _progress = progress / 100;
                          });
                          if (progress == 100) {
                            _pullToRefreshController.endRefreshing();
                          }
                        },
                        onCreateWindow: (controller, createWindowAction) async {
                          WebUri url = createWindowAction.request.url;

                          if (url != null) {
                            String url_ = url.toString();

                            if (url_.contains('wc?uri=')) {
                              final wcUri = Uri.parse(
                                Uri.decodeFull(
                                  Uri.parse(url_).queryParameters['uri'],
                                ),
                              );

                              final session = WCSession.from(wcUri.toString());

                              if (session != WCSession.empty()) {
                                if (!checkProfile) {
                                  await showModalCreateWallet();
                                } else {
                                  await WcConnector.qrScanHandler(
                                      wcUri.toString());
                                }
                              } else {
                                await WcConnector.wcReconnect();
                              }
                              return false;
                            } else if (url_.startsWith('wc:')) {
                              final session = WCSession.from(url_);

                              if (session != WCSession.empty()) {
                                if (!checkProfile) {
                                  await showModalCreateWallet();
                                } else {
                                  await WcConnector.qrScanHandler(url_);
                                }
                              } else {
                                await WcConnector.wcReconnect();
                              }
                              return false;
                            }
                          }
                          widget.onCreateTabRequested(createWindowAction);
                          return true;
                        },
                        onCloseWindow: (controller) {
                          widget.onCloseTabRequested();
                        },
                        onLoadStop: (controller, url) async {
                          // final pref = Hive.box(secureStorageKey);

                          // controller.clearCache();
                          await controller.setSettings(
                            settings: InAppWebViewSettings(
                                userAgent:
                                    'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36'),
                          );

                          updateScreenshot();
                          _pullToRefreshController.endRefreshing();
                          if (url != null) {
                            final sslCertificate =
                                await controller.getCertificate();
                            _url = url.toString();
                            _isSecure =
                                sslCertificate != null || urlIsSecure(url);
                          }

                          final favicons = await _controller?.getFavicons();
                          if (favicons != null && favicons.isNotEmpty) {
                            for (final favicon in favicons) {
                              if (_favicon == null) {
                                _favicon = favicon;
                              } else if (favicon.width != null &&
                                  (favicon.width ?? 0) >
                                      (_favicon?.width ?? 0)) {
                                _favicon = favicon;
                              }
                            }
                          }

                          widget.onStateUpdated.call();
                        },
                        onConsoleMessage: (InAppWebViewController controller,
                            ConsoleMessage message) {
                          if (kDebugMode) {
                            print(message.toString());
                          }
                        },
                        onReceivedHttpAuthRequest:
                            (controller, challenge) async {
                          var action = await createHttpAuthDialog(challenge);
                          return HttpAuthResponse(
                              username: _httpAuthUsernameController.text.trim(),
                              password: _httpAuthPasswordController.text,
                              action: action,
                              permanentPersistence: true);
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
              ]),
              SingleChildScrollView(
                child: Column(
                  children: [
                    const SizedBox(
                      height: 5,
                    ),
                    for (int i = 0; i < historyData.length && i < 5; i++)
                      if (historyData[i] != null)
                        GestureDetector(
                          onTap: () async {
                            FocusManager.instance.primaryFocus?.unfocus();
                            _controller.loadUrl(
                              urlRequest: URLRequest(
                                url: WebUri(
                                  historyData[i]['url'],
                                ),
                              ),
                            );
                          },
                          child: Padding(
                            padding: const EdgeInsets.only(bottom: 2.0),
                            child: Card(
                              child: Padding(
                                padding: const EdgeInsets.all(20),
                                child: Row(
                                  mainAxisAlignment:
                                      MainAxisAlignment.spaceBetween,
                                  children: [
                                    Row(
                                      children: [
                                        const Icon(
                                          FontAwesomeIcons.globe,
                                          size: 30,
                                        ),
                                        const SizedBox(
                                          width: 10,
                                        ),
                                        Column(
                                          crossAxisAlignment:
                                              CrossAxisAlignment.start,
                                          children: [
                                            Text(
                                              ellipsify(
                                                str: historyData[i]['title'],
                                                maxLength: 20,
                                              ),
                                              style:
                                                  const TextStyle(fontSize: 14),
                                              overflow: TextOverflow.ellipsis,
                                            ),
                                            Text(
                                              ellipsify(
                                                str: historyData[i]['url'],
                                                maxLength: 20,
                                              ),
                                              style:
                                                  const TextStyle(fontSize: 13),
                                              overflow: TextOverflow.ellipsis,
                                            ),
                                          ],
                                        ),
                                      ],
                                    ),
                                    Row(
                                      children: [
                                        IconButton(
                                          onPressed: () {},
                                          icon: Transform.rotate(
                                            angle: 225 * pi / 180,
                                            child: const Icon(
                                                Icons.arrow_forward_sharp),
                                          ),
                                        ),
                                        IconButton(
                                          onPressed: () {},
                                          icon: const Icon(Icons.star),
                                        ),
                                      ],
                                    )
                                  ],
                                ),
                              ),
                            ),
                          ),
                        ),
                    Card(
                      child: Padding(
                        padding: const EdgeInsets.all(10),
                        child: Column(children: [
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              const Text('Transactions history'),
                              Row(
                                children: const [
                                  Text('See all'),
                                  Icon(Icons.arrow_forward),
                                ],
                              )
                            ],
                          )
                        ]),
                      ),
                    ),
                    Card(
                      child: Padding(
                        padding: const EdgeInsets.all(10),
                        child: Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              GestureDetector(
                                onTap: () async {
                                  String data = await Navigator.push(
                                      context,
                                      MaterialPageRoute(
                                        builder: (_) => const QRScanView(),
                                      ));
                                  // Get.to(
                                  //   const QRScanView(),
                                  // );
                                  if (data == null) {
                                    // Get.snackbar(
                                    //   '',
                                    //   eIP681ProcessingErrorMsg,
                                    //   colorText: Colors.white,
                                    //   backgroundColor: Colors.red,
                                    // );

                                    ScaffoldMessenger.of(context)
                                        .showSnackBar(SnackBar(
                                      content:
                                          const Text(eIP681ProcessingErrorMsg),
                                      backgroundColor: Colors.red,
                                      action: SnackBarAction(
                                        textColor: Colors.white,
                                        label: 'OK',
                                        onPressed: () {},
                                      ),
                                    ));

                                    return;
                                  }
                                  showDialog(
                                      barrierDismissible: false,
                                      context: context,
                                      builder: (context) {
                                        return AlertDialog(
                                            backgroundColor: Colors.white,
                                            content: Column(
                                              mainAxisAlignment:
                                                  MainAxisAlignment.center,
                                              mainAxisSize: MainAxisSize.min,
                                              children: const [
                                                SizedBox(
                                                  width: 35,
                                                  height: 35,
                                                  child: Loader(),
                                                ),
                                              ],
                                            ));
                                      });

                                  Map scannedData = await processEIP681(data);

                                  if (Navigator.canPop(context)) {
                                    // Get.back();
                                    Navigator.of(context).pop();
                                  }

                                  if (scannedData['success']) {
                                    await Navigator.of(context)
                                        .push(MaterialPageRoute(
                                      builder: (_) => SendToken(
                                        data: scannedData['msg'],
                                      ),
                                    ));
                                    // Get.to();
                                    return;
                                  }
                                  // Get.snackbar(
                                  //   '',
                                  //   scannedData['msg'],
                                  //   colorText: Colors.white,
                                  //   backgroundColor: Colors.red,
                                  // );

                                  ScaffoldMessenger.of(context)
                                      .showSnackBar(SnackBar(
                                    content: scannedData['msg'],
                                    backgroundColor: Colors.red,
                                    action: SnackBarAction(
                                      textColor: Colors.white,
                                      label: 'OK',
                                      onPressed: () {},
                                    ),
                                  ));
                                },
                                child: SizedBox(
                                  width: 100,
                                  child: Column(
                                    children: const [
                                      Icon(Icons.qr_code),
                                      Text(
                                        'Scan',
                                        style: TextStyle(fontSize: 12),
                                      ),
                                    ],
                                  ),
                                ),
                              ),
                              SizedBox(
                                width: 100,
                                child: Column(
                                  children: const [
                                    Icon(Icons.search),
                                    Text(
                                      'Search Engine',
                                      style: TextStyle(fontSize: 12),
                                    ),
                                  ],
                                ),
                              ),
                              GestureDetector(
                                onTap: () async {
                                  final pref = Hive.box(secureStorageKey);
                                  // bool hasWallet =
                                  //     pref.get(currentMmenomicKey) != null;
                                  bool hasWallet = checkProfile;

                                  bool hasPasscode =
                                      pref.get(userUnlockPasscodeKey) != null;
                                  Widget dappWidget;
                                  // Get.back();
                                  Navigator.of(context).pop();

                                  if (hasWallet) {
                                    dappWidget = const WalletMainBody();
                                  } else if (hasPasscode) {
                                    dappWidget = const MainScreen();
                                  } else {
                                    dappWidget = const Security();
                                  }
                                  await Navigator.of(context).push(
                                      MaterialPageRoute(
                                          builder: (_) => dappWidget));
                                  // Get.to(dappWidget);
                                },
                                child: SizedBox(
                                  width: 100,
                                  child: Column(
                                    children: const [
                                      Icon(FontAwesomeIcons.wallet),
                                      Text(
                                        'My wallet',
                                        style: TextStyle(fontSize: 12),
                                        textAlign: TextAlign.center,
                                      ),
                                    ],
                                  ),
                                ),
                              ),
                            ]),
                      ),
                    ),
                  ],
                ),
              )
            ],
          ),
        ),
      );
    });
  }

  Future<WebNotificationPermission> onNotificationRequestPermission() async {
    final url = await _controller.getUrl();

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
          (await _controller?.getUrl()).toString() + iconUrl.toString());
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

  Future<void> updateScreenshot() async {
    _screenshot = (await _controller
        ?.takeScreenshot(
            screenshotConfiguration: ScreenshotConfiguration(
                compressFormat: CompressFormat.JPEG, quality: 20))
        ?.timeout(
          const Duration(milliseconds: 1500),
          onTimeout: () => null,
        ));
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

  void setUserAgent(InAppWebViewController controller) async {
    final defaultUserAgent = await InAppWebViewController.getDefaultUserAgent();
    if (kDebugMode) {
      print("Default User Agent: $defaultUserAgent");
    }

    String newUserAgent;

    // String newUserAgent;

    // newUserAgent =
    //     'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36';

    // await controller.setSettings(
    //     settings: InAppWebViewSettings(userAgent: newUserAgent));
  }

  void selectProfile() async {
    print('select Profile');
    final tabUserData = Provider.of<ProviderClass>(context, listen: false);
    await showProfileDialog(
      onTap: (phrase, name) {
        print('name inside showProfileDialog $name');
        tabUserData.changeTabUserCred(viewId, phrase, null);
        Navigator.of(context).pop();
        loadUserData(false);
        selectBlockChain();
      },
      context: context,
      selectedProfile: tabProfile,
    );
  }

  void selectBlockChain() async {
    print('selectBlockChain');
    final tabUserData = Provider.of<ProviderClass>(context, listen: false);
    await showBlockChainDialog(
        onTap: (blockChainData) {
          _controller.clearCache();
          _controller.reload();
          int _chainId = blockChainData['chainId'];
          print('_chainId inside showBlockChainDialog $_chainId');

          tabUserData.changeTabUserCred(viewId, '', _chainId);
          // tabUserData.get
          Navigator.of(context).pop();
          loadUserData(false);
          // _controller.clearCache();
          // _controller.reload();
        },
        context: context,
        selectedChainId: tabChainId);
  }

  Future<dynamic> showModalCreateWallet() async {
    // print('hi from showModalCreateWallet');

    final pref = Hive.box(secureStorageKey);
    // bool hasWallet = pref.get(currentMmenomicKey) != null;

    bool hasPasscode = pref.get(userUnlockPasscodeKey) != null;
    Widget dappWidget;

    if (checkProfile) {
      dappWidget = const WalletMainBody();
    } else if (hasPasscode) {
      dappWidget = const MainScreen();
    } else {
      dappWidget = const Security();
    }

    if (!checkProfile) {
      return showModalBottomSheet(
        context: context,
        builder: (BuildContext context) {
          return Container(
            height: 200,
            decoration: const BoxDecoration(
              color: Colors.white,
              borderRadius: BorderRadius.only(
                topLeft: Radius.circular(20.0),
                topRight: Radius.circular(20.0),
              ),
            ),
            child: Padding(
              padding: const EdgeInsets.all(20.0),
              child: Column(
                children: <Widget>[
                  const Text(
                    'This site needs a web3 wallet. Would you like to create one now ?',
                  ),
                  const SizedBox(height: 20.0),
                  TextButton(
                    onPressed: () async {
                      var currentUrl = await _controller?.getUrl();
                      // var currentUrl = WebViewTab.currentUrl;
                      // print(currentUrl);
                      await pref.put('redirectUrl', currentUrl.toString());
                      Navigator.of(context)
                          .push(MaterialPageRoute(builder: (_) => dappWidget));
                    },
                    child: Text(AppLocalizations.of(context).createNewWallet),
                  ),
                ],
              ),
            ),
          );
        },
      );
    }
  }
}
