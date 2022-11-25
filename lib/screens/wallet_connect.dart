import 'dart:convert';
import 'dart:typed_data';

import 'package:cached_network_image/cached_network_image.dart';
import 'package:cryptowallet/components/wallet_logo.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:decimal/decimal.dart';
import 'package:eth_sig_util/eth_sig_util.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:get/state_manager.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart' as http;
import 'package:wallet_connect/wallet_connect.dart';
import '../components/loader.dart';
import '../utils/app_config.dart';
import 'package:web3dart/crypto.dart';
import 'package:web3dart/web3dart.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

import '../utils/qr_scan_view.dart';

class WalletConnect extends StatefulWidget {
  final String wcLink;
  const WalletConnect({Key key, this.wcLink}) : super(key: key);

  @override
  _WalletConnectState createState() => _WalletConnectState();
}

class _WalletConnectState extends State<WalletConnect> {
  WCClient _wcClient;
  Box _prefs;
  TextEditingController _textEditingController;
  String walletAddress, privateKey;
  int chainId;
  WCSessionStore _sessionStore;
  Web3Client _web3client;
  String currencySymbol;
  RxBool toggler = false.obs;
  String connectedWebsiteUrl = "";

  @override
  void initState() {
    super.initState();
    _initialize();
    if (widget.wcLink != null) {
      _qrScanHandler(widget.wcLink);
    }
  }

  @override
  void dispose() {
    if (_wcClient.isConnected) {
      _wcClient.killSession();
      _wcClient.disconnect();
    }
    _textEditingController.dispose();
    super.dispose();
  }

  _initialize() async {
    _wcClient = WCClient(
      onSessionRequest: _onSessionRequest,
      onFailure: _onSessionError,
      onDisconnect: _onSessionClosed,
      onEthSign: _onSign,
      onEthSignTransaction: _onSignTransaction,
      onEthSendTransaction: _onSendTransaction,
      onCustomRequest: (_, __) {},
      onConnect: _onConnect,
      onWalletSwitchNetwork: _onSwitchNetwork,
    );

    _textEditingController = TextEditingController();
    _prefs = Hive.box(secureStorageKey);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('$walletAbbr WalletConnect'),
      ),
      body: RefreshIndicator(
        onRefresh: () async {
          await Future.delayed(const Duration(seconds: 2));
        },
        child: SafeArea(
          child: SingleChildScrollView(
              physics: const AlwaysScrollableScrollPhysics(),
              child: Obx(() {
                return Column(
                  children: [
                    Visibility(
                      child: Text(toggler.value.toString()),
                      visible: false,
                    ),
                    Padding(
                      padding: const EdgeInsets.all(25.0),
                      child: Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          const WalletLogo(),
                          const SizedBox(
                            height: 30,
                          ),
                          const Text(
                            ' Wallet Connect',
                            style: TextStyle(
                              fontSize: 30,
                              fontWeight: FontWeight.bold,
                            ),
                            textAlign: TextAlign.center,
                          ),
                          const SizedBox(height: 50),
                          SizedBox(
                            width: MediaQuery.of(context).size.width * 0.85,
                            height: 50,
                            child: ElevatedButton(
                              style: ButtonStyle(
                                backgroundColor:
                                    MaterialStateProperty.resolveWith(
                                  (states) => Theme.of(context).brightness ==
                                          Brightness.dark
                                      ? const Color(0xff5D5E81)
                                      : const Color(0xffEBF3FF),
                                ),
                                shape: MaterialStateProperty.resolveWith(
                                  (states) => RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(10),
                                  ),
                                ),
                              ),
                              child: Align(
                                alignment: Alignment.center,
                                child: Text.rich(
                                  TextSpan(
                                    text: 'Connect Previous Session?',
                                    style: TextStyle(
                                      fontSize: 16,
                                      fontWeight: FontWeight.bold,
                                      color: Theme.of(context)
                                          .primaryTextTheme
                                          .bodyLarge
                                          .color,
                                    ),
                                  ),
                                ),
                              ),
                              onPressed: () {
                                _connectToPreviousSession();
                              },
                            ),
                          ),
                          const SizedBox(height: 20),
                          _wcClient.isConnected
                              ? SizedBox(
                                  width:
                                      MediaQuery.of(context).size.width * 0.85,
                                  height: 50,
                                  child: ElevatedButton(
                                    style: ButtonStyle(
                                      backgroundColor:
                                          MaterialStateProperty.resolveWith(
                                              (states) => appBackgroundblue),
                                      shape: MaterialStateProperty.resolveWith(
                                        (states) => RoundedRectangleBorder(
                                          borderRadius:
                                              BorderRadius.circular(10),
                                        ),
                                      ),
                                    ),
                                    child: Align(
                                      alignment: Alignment.center,
                                      child: Text(
                                        AppLocalizations.of(context)
                                            .killSession,
                                        style: const TextStyle(
                                          fontSize: 16,
                                          fontWeight: FontWeight.bold,
                                          color: Colors.white,
                                        ),
                                        textAlign: TextAlign.center,
                                      ),
                                    ),
                                    onPressed: () {
                                      _wcClient.killSession();
                                    },
                                  ),
                                )
                              : SizedBox(
                                  width:
                                      MediaQuery.of(context).size.width * 0.85,
                                  height: 50,
                                  child: ElevatedButton(
                                    style: ButtonStyle(
                                      backgroundColor:
                                          MaterialStateProperty.resolveWith(
                                              (states) => appBackgroundblue),
                                      shape: MaterialStateProperty.resolveWith(
                                        (states) => RoundedRectangleBorder(
                                          borderRadius:
                                              BorderRadius.circular(10),
                                        ),
                                      ),
                                    ),
                                    child: Align(
                                      alignment: Alignment.center,
                                      child: Row(
                                        mainAxisAlignment:
                                            MainAxisAlignment.spaceBetween,
                                        children: [
                                          SvgPicture.asset(
                                            'assets/Qrcode.svg',
                                            color: Colors.transparent,
                                          ),
                                          Text(
                                            AppLocalizations.of(context)
                                                .connectViAQR,
                                            style: const TextStyle(
                                                fontSize: 16,
                                                fontWeight: FontWeight.bold,
                                                color: Colors.white),
                                            textAlign: TextAlign.center,
                                          ),
                                          SvgPicture.asset('assets/Qrcode.svg'),
                                        ],
                                      ),
                                    ),
                                    onPressed: () {
                                      Navigator.push(
                                        context,
                                        MaterialPageRoute(
                                          builder: (_) => const QRScanView(),
                                        ),
                                      ).then((value) {
                                        if (value != null) {
                                          _qrScanHandler(value);
                                        }
                                      });
                                    },
                                  ),
                                ),
                          _wcClient.isConnected
                              ? Container()
                              : const SizedBox(height: 20),
                          _wcClient.isConnected
                              ? Container()
                              : Container(
                                  decoration: BoxDecoration(
                                    border: Border.all(
                                      color: Theme.of(context)
                                          .primaryTextTheme
                                          .bodyLarge
                                          .color,
                                    ),
                                    borderRadius: const BorderRadius.all(
                                      Radius.circular(10),
                                    ),
                                  ),
                                  width:
                                      MediaQuery.of(context).size.width * 0.85,
                                  height: 50,
                                  child: ElevatedButton(
                                    style: ButtonStyle(
                                      elevation:
                                          MaterialStateProperty.resolveWith(
                                        (states) => 0,
                                      ),
                                      backgroundColor:
                                          MaterialStateProperty.resolveWith(
                                        (states) => Theme.of(context)
                                            .scaffoldBackgroundColor,
                                      ),
                                      shape: MaterialStateProperty.resolveWith(
                                        (states) => RoundedRectangleBorder(
                                          borderRadius:
                                              BorderRadius.circular(10),
                                        ),
                                      ),
                                    ),
                                    child: Align(
                                      alignment: Alignment.center,
                                      child: Text(
                                        AppLocalizations.of(context)
                                            .connectViACode,
                                        style: TextStyle(
                                          fontSize: 16,
                                          fontWeight: FontWeight.bold,
                                          color: Theme.of(context)
                                              .primaryTextTheme
                                              .bodyLarge
                                              .color,
                                        ),
                                      ),
                                    ),
                                    onPressed: () {
                                      showGeneralDialog(
                                          context: context,
                                          barrierDismissible: true,
                                          barrierLabel:
                                              AppLocalizations.of(context)
                                                  .pasteCode,
                                          pageBuilder: (context, _, __) {
                                            return SimpleDialog(
                                              title: Text(
                                                AppLocalizations.of(context)
                                                    .pasteCode,
                                              ),
                                              titlePadding:
                                                  const EdgeInsets.fromLTRB(
                                                      16.0, 16.0, 16.0, .0),
                                              contentPadding:
                                                  const EdgeInsets.all(16.0),
                                              children: [
                                                TextFormField(
                                                  controller:
                                                      _textEditingController,
                                                  decoration: InputDecoration(
                                                    label: Text(
                                                      AppLocalizations.of(
                                                              context)
                                                          .enterCode,
                                                    ),

                                                    focusedBorder:
                                                        const OutlineInputBorder(
                                                            borderRadius:
                                                                BorderRadius.all(
                                                                    Radius.circular(
                                                                        10.0)),
                                                            borderSide:
                                                                BorderSide
                                                                    .none),
                                                    border: const OutlineInputBorder(
                                                        borderRadius:
                                                            BorderRadius.all(
                                                                Radius.circular(
                                                                    10.0)),
                                                        borderSide:
                                                            BorderSide.none),
                                                    enabledBorder:
                                                        const OutlineInputBorder(
                                                            borderRadius:
                                                                BorderRadius.all(
                                                                    Radius.circular(
                                                                        10.0)),
                                                            borderSide: BorderSide
                                                                .none), // you
                                                    filled: true,
                                                  ),
                                                ),
                                                const SizedBox(height: 16.0),
                                                Row(
                                                  mainAxisAlignment:
                                                      MainAxisAlignment.end,
                                                  children: [
                                                    TextButton(
                                                      onPressed: () =>
                                                          Navigator.pop(
                                                              context),
                                                      child: Text(
                                                        AppLocalizations.of(
                                                                context)
                                                            .confirm,
                                                      ),
                                                    ),
                                                  ],
                                                ),
                                              ],
                                            );
                                          }).then((_) {
                                        if (_textEditingController
                                            .text.isNotEmpty) {
                                          _qrScanHandler(
                                              _textEditingController.text);
                                          _textEditingController.clear();
                                        }
                                      });
                                    },
                                  ),
                                ),
                          const SizedBox(height: 20),
                          _wcClient.isConnected
                              ? Text(
                                  '${AppLocalizations.of(context).connectedTo} $connectedWebsiteUrl')
                              : Container()
                        ],
                      ),
                    ),
                  ],
                );
              })),
        ),
      ),
    );
  }

  _qrScanHandler(String value) {
    final session = WCSession.from(value);
    debugPrint('session $session');
    final peerMeta = WCPeerMeta(
      name: walletName,
      url: walletURL,
      description: walletAbbr,
      icons: [walletIconURL],
    );
    _wcClient.connectNewSession(session: session, peerMeta: peerMeta);
  }

  _connectToPreviousSession() {
    final _sessionSaved = _prefs.get('session');
    debugPrint('_sessionSaved $_sessionSaved');
    _sessionStore = _sessionSaved != null
        ? WCSessionStore.fromJson(jsonDecode(_sessionSaved))
        : null;
    if (_sessionStore != null) {
      debugPrint('_sessionStore $_sessionStore');
      _wcClient.connectFromSessionStore(_sessionStore);
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(AppLocalizations.of(context).noPreviousSessionFound),
        ),
      );
    }
  }

  _onConnect() {
    toggler.value = !toggler.value;
  }

  _onSessionRequest(int id, WCPeerMeta peerMeta) {
    showDialog(
      barrierDismissible: false,
      context: context,
      builder: (_) {
        return SimpleDialog(
          title: Column(
            children: [
              if (peerMeta.icons.isNotEmpty)
                Container(
                  height: 100.0,
                  width: 100.0,
                  padding: const EdgeInsets.only(bottom: 8.0),
                  child: CachedNetworkImage(
                    imageUrl: ipfsTohttp(peerMeta.icons.first),
                    placeholder: (context, url) => Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      mainAxisSize: MainAxisSize.min,
                      children: const [
                        SizedBox(
                          width: 20,
                          height: 20,
                          child: Loader(
                            color: appPrimaryColor,
                          ),
                        )
                      ],
                    ),
                    errorWidget: (context, url, error) => const Icon(
                      Icons.error,
                      color: Colors.red,
                    ),
                  ),
                ),
              Text(peerMeta.name),
            ],
          ),
          contentPadding: const EdgeInsets.fromLTRB(16.0, 12.0, 16.0, 16.0),
          children: [
            if (peerMeta.description.isNotEmpty)
              Padding(
                padding: const EdgeInsets.only(bottom: 8.0),
                child: Text(peerMeta.description),
              ),
            if (peerMeta.url.isNotEmpty)
              Padding(
                padding: const EdgeInsets.only(bottom: 8.0),
                child: Text(
                    '${AppLocalizations.of(context).connectedTo} ${peerMeta.url}'),
              ),
            Row(
              children: [
                Expanded(
                  child: TextButton(
                    style: TextButton.styleFrom(
                      backgroundColor: Theme.of(context).colorScheme.secondary,
                    ),
                    onPressed: () async {
                      showBlockChainDialog(
                          context: context,
                          onTap: (blockChainData) async {
                            currencySymbol = blockChainData['symbol'];
                            final mnemonic = Hive.box(secureStorageKey)
                                .get(currentMmenomicKey);
                            final response = await getEthereumFromMemnomic(
                              mnemonic,
                              blockChainData['coinType'],
                            );
                            chainId = blockChainData['chainId'];
                            walletAddress = response['eth_wallet_address'];
                            privateKey = response['eth_wallet_privateKey'];
                            _web3client = Web3Client(
                              blockChainData['rpc'],
                              http.Client(),
                            );
                            _wcClient.approveSession(
                              accounts: [walletAddress],
                              chainId: blockChainData['chainId'],
                            );
                            _sessionStore = _wcClient.sessionStore;
                            await _prefs.put(
                              'session',
                              jsonEncode(_wcClient.sessionStore.toJson()),
                            );

                            connectedWebsiteUrl = peerMeta.url;
                            toggler.value = !toggler.value;
                            int count = 0;
                            Navigator.popUntil(context, (route) {
                              return count++ == 2;
                            });
                          });
                    },
                    child: Text(AppLocalizations.of(context).confirm),
                  ),
                ),
                const SizedBox(width: 16.0),
                Expanded(
                  child: TextButton(
                    style: TextButton.styleFrom(
                      backgroundColor: Theme.of(context).colorScheme.secondary,
                    ),
                    onPressed: () {
                      _wcClient.rejectSession();
                      Navigator.pop(context);
                    },
                    child: Text(AppLocalizations.of(context).reject),
                  ),
                ),
              ],
            ),
          ],
        );
      },
    );
  }

  _onSessionError(dynamic message) async {
    toggler.value = !toggler.value;
    showDialog(
      barrierDismissible: false,
      context: context,
      builder: (_) {
        return SimpleDialog(
          title: Text(AppLocalizations.of(context).error),
          contentPadding: const EdgeInsets.fromLTRB(16.0, 12.0, 16.0, 16.0),
          children: [
            Padding(
              padding: const EdgeInsets.only(bottom: 8.0),
              child: Text(
                  '${AppLocalizations.of(context).someErrorOccured}. $message'),
            ),
            Row(
              children: [
                TextButton(
                  style: TextButton.styleFrom(
                    backgroundColor: Theme.of(context).colorScheme.secondary,
                  ),
                  onPressed: () {
                    Navigator.pop(context);
                  },
                  child: Text(AppLocalizations.of(context).close),
                ),
              ],
            ),
          ],
        );
      },
    );
  }

  _onSessionClosed(int code, String reason) async {
    await _prefs.delete('session');
    toggler.value = !toggler.value;
    showDialog(
      barrierDismissible: false,
      context: context,
      builder: (_) {
        return SimpleDialog(
          title: Text(AppLocalizations.of(context).sessionEnded),
          contentPadding: const EdgeInsets.fromLTRB(16.0, 12.0, 16.0, 16.0),
          children: [
            Padding(
              padding: const EdgeInsets.only(bottom: 8.0),
              child: Text(
                  '${AppLocalizations.of(context).someErrorOccured}. ERROR CODE: $code'),
            ),
            if (reason != null)
              Padding(
                padding: const EdgeInsets.only(bottom: 8.0),
                child: Text(
                    '${AppLocalizations.of(context).failureReason}: $reason'),
              ),
            Row(
              children: [
                TextButton(
                  style: TextButton.styleFrom(
                    backgroundColor: Theme.of(context).colorScheme.secondary,
                  ),
                  onPressed: () {
                    Navigator.pop(context);
                  },
                  child: Text(AppLocalizations.of(context).close),
                ),
              ],
            ),
          ],
        );
      },
    );
  }

  _onSignTransaction(
    int id,
    WCEthereumTransaction ethereumTransaction,
  ) {
    _onTransaction(
      id: id,
      ethereumTransaction: ethereumTransaction,
      title: AppLocalizations.of(context).signTransaction,
      onConfirm: () async {
        try {
          final creds = EthPrivateKey.fromHex(privateKey);
          final tx = await _web3client.signTransaction(
            creds,
            _wcEthTxToWeb3Tx(ethereumTransaction),
            chainId: _wcClient.chainId,
          );
          _wcClient.approveRequest<String>(
            id: id,
            result: bytesToHex(tx),
          );
        } catch (e) {
          _wcClient.rejectRequest(id: id);
        } finally {
          Navigator.pop(context);
        }
      },
      onReject: () {
        _wcClient.rejectRequest(id: id);
        Navigator.pop(context);
      },
    );
  }

  _onSwitchNetwork(int id, int chainIdNew) async {
    final currentChainIdData = getEthereumDetailsFromChainId(chainId);
    final switchChainIdData = getEthereumDetailsFromChainId(chainIdNew);
    switchEthereumChain(
      context: context,
      currentChainIdData: currentChainIdData,
      switchChainIdData: switchChainIdData,
      onConfirm: () async {
        _web3client = Web3Client(
          switchChainIdData['rpc'],
          http.Client(),
        );
        await _wcClient.updateSession(chainId: chainIdNew);
        _wcClient.approveRequest<void>(id: id, result: null);
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(
          content: Text('Changed network to $chainIdNew.'),
        ));
        chainId = chainIdNew;
        Navigator.pop(context);
      },
      onReject: () async {
        _wcClient.rejectRequest(id: id);
        Navigator.pop(context);
      },
    );
  }

  _onSendTransaction(
    int id,
    WCEthereumTransaction ethereumTransaction,
  ) {
    _onTransaction(
      id: id,
      ethereumTransaction: ethereumTransaction,
      title: AppLocalizations.of(context).sendTransaction,
      onConfirm: () async {
        try {
          final creds = EthPrivateKey.fromHex(privateKey);
          final txhash = await _web3client.sendTransaction(
            creds,
            _wcEthTxToWeb3Tx(ethereumTransaction),
            chainId: _wcClient.chainId,
          );
          debugPrint('txhash $txhash');
          _wcClient.approveRequest<String>(
            id: id,
            result: txhash,
          );
        } catch (e) {
          _wcClient.rejectRequest(id: id);
        } finally {
          Navigator.pop(context);
        }
      },
      onReject: () {
        _wcClient.rejectRequest(id: id);
        Navigator.pop(context);
      },
    );
  }

  _onTransaction({
    int id,
    WCEthereumTransaction ethereumTransaction,
    String title,
    VoidCallback onConfirm,
    VoidCallback onReject,
  }) async {
    List icons = _wcClient.remotePeerMeta.icons;
    await signTransaction(
      gasPriceInWei_: ethereumTransaction.gasPrice,
      to: ethereumTransaction.to,
      from: ethereumTransaction.from,
      txData: ethereumTransaction.data,
      valueInWei_: ethereumTransaction.value,
      gasInWei_: ethereumTransaction.gas,
      networkIcon: icons.isNotEmpty ? icons[0] : null,
      context: context,
      blockChainCurrencySymbol: currencySymbol,
      name: _wcClient.remotePeerMeta.name,
      onConfirm: onConfirm,
      onReject: onReject,
      title: title,
      chainId: _wcClient.chainId,
    );
  }

  _onSign(
    int id,
    WCEthereumSignMessage ethereumSignMessage,
  ) async {
    List icon = _wcClient.remotePeerMeta.icons;
    await signMessage(
      messageType: ethereumSignMessage.type == WCSignType.PERSONAL_MESSAGE
          ? personalSignKey
          : '',
      context: context,
      data: ethereumSignMessage.data,
      networkIcon: icon.isNotEmpty ? icon[0] : null,
      name: _wcClient.remotePeerMeta.name,
      onConfirm: () async {
        String signedDataHex;
        final credentials = EthPrivateKey.fromHex(privateKey);
        if (ethereumSignMessage.type == WCSignType.TYPED_MESSAGE) {
          signedDataHex = EthSigUtil.signTypedData(
            privateKey: privateKey,
            jsonData: ethereumSignMessage.data,
            version: TypedDataVersion.V4,
          );
        } else {
          Uint8List signedData = await credentials.signPersonalMessage(
            txDataToUintList(
              ethereumSignMessage.data,
            ),
          );
          signedDataHex = bytesToHex(signedData, include0x: true);
        }
        debugPrint('SIGNED $signedDataHex');
        _wcClient.approveRequest<String>(
          id: id,
          result: signedDataHex,
        );
        Navigator.pop(context);
      },
      onReject: () {
        _wcClient.rejectRequest(id: id);
        Navigator.pop(context);
      },
    );
  }

  Transaction _wcEthTxToWeb3Tx(WCEthereumTransaction ethereumTransaction) {
    return Transaction(
      from: EthereumAddress.fromHex(ethereumTransaction.from),
      to: EthereumAddress.fromHex(ethereumTransaction.to),
      maxGas: ethereumTransaction.gasLimit != null
          ? int.tryParse(ethereumTransaction.gasLimit)
          : null,
      gasPrice: ethereumTransaction.gasPrice != null
          ? EtherAmount.inWei(BigInt.parse(ethereumTransaction.gasPrice))
          : null,
      value: EtherAmount.inWei(BigInt.parse(ethereumTransaction.value ?? '0')),
      data: hexToBytes(ethereumTransaction.data),
      nonce: ethereumTransaction.nonce != null
          ? int.tryParse(ethereumTransaction.nonce)
          : null,
    );
  }
}
