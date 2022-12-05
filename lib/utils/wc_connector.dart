import 'package:cached_network_image/cached_network_image.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/navigator_service.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:hive/hive.dart';
import 'package:wallet_connect/wallet_connect.dart';
import 'package:web3dart/web3dart.dart';
import 'dart:convert';
import 'package:cryptowallet/components/loader.dart';
import 'package:eth_sig_util/eth_sig_util.dart';
import 'package:flutter/foundation.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:intl/intl.dart';
import 'package:web3dart/crypto.dart';
import 'package:web3dart/web3dart.dart' hide Wallet;
import 'package:http/http.dart' as http;
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class WcConnector {
  BuildContext context;
  static WCClient wcClient;
  static Box _prefs;
  String _walletAddress, _privateKey;
  int _chainId;
  Web3Client _web3client;
  String _currencySymbol;
  WcConnector() {
    context = NavigationService.navigatorKey.currentContext;
    _prefs = Hive.box(secureStorageKey);
    wcClient = WCClient(
      onSessionRequest: _onSessionRequest,
      onFailure: _onSessionError,
      onDisconnect: _onSessionClosed,
      onEthSign: _onSign,
      onEthSignTransaction: _onSignTransaction,
      onEthSendTransaction: _onSendTransaction,
      onCustomRequest: (int id, String request) {
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(
          content: Text('id $id request $request.'),
        ));
      },
      onConnect: _onConnect,
      onWalletSwitchNetwork: _onSwitchNetwork,
    );
    wcReconnect();
  }

  static qrScanHandler(String value) async {
    if (value == null) return;
    try {
      final session = WCSession.from(value);
      if (session == WCSession.empty()) return;
      final peerMeta = WCPeerMeta(
        name: walletName,
        url: walletURL,
        description: walletAbbr,
        icons: [walletIconURL],
      );

      await wcClient.connectNewSession(session: session, peerMeta: peerMeta);
    } catch (_) {}
  }

  static Future removedCurrentSession() async {
    try {
      String wcSessions = _prefs.get(wcSessionKey);
      if (wcSessions != null) {
        List sessions_ = jsonDecode(wcSessions);
        for (int i = 0; i < sessions_.length; i++) {
          Map session = sessions_[i];
          bool sameTopic = session['session']['topic'] ==
              wcClient.sessionStore.session.topic;
          bool sameKey =
              session['session']['key'] == wcClient.sessionStore.session.key;

          if (sameTopic && sameKey) {
            sessions_.removeAt(i);
            await _prefs.put(wcSessionKey, jsonEncode(sessions_));
          }
        }
      }
    } catch (_) {}
  }

  static Future wcReconnect() async {
    String wcSessions = _prefs.get(wcSessionKey);
    if (wcSessions != null && !wcClient.isConnected) {
      List sessions_ = jsonDecode(wcSessions);
      for (Map session in sessions_) {
        try {
          await wcClient.connectFromSessionStore(
            WCSessionStore.fromJson(session),
          );
        } catch (_) {}
      }
    }
  }

  setSigningDetails(int chainId) async {
    Map blockChainData = getEthereumDetailsFromChainId(chainId);
    final _mnemonic = _prefs.get(currentMmenomicKey);
    final response = await getEthereumFromMemnomic(
      _mnemonic,
      blockChainData['coinType'],
    );
    _walletAddress = response['eth_wallet_address'];
    _privateKey = response['eth_wallet_privateKey'];
    _web3client = Web3Client(
      blockChainData['rpc'],
      http.Client(),
    );
    _currencySymbol = blockChainData['symbol'];
  }

  _onConnect() {}

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
                            _chainId = blockChainData['chainId'];
                            await setSigningDetails(_chainId);
                            wcClient.approveSession(
                              accounts: [_walletAddress],
                              chainId: blockChainData['chainId'],
                            );
                            List sessions = [];

                            sessions.add(
                              wcClient.sessionStore.toJson()
                                ..addAll(
                                  {
                                    'date': DateFormat("yyyy-MM-dd HH:mm:ss")
                                        .format(
                                      DateTime.now(),
                                    ),
                                    'address': _walletAddress
                                  },
                                ),
                            );
                            await _prefs.put(
                              wcSessionKey,
                              jsonEncode(sessions),
                            );

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
                      wcClient.rejectSession();
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

  _onSessionError(dynamic message) {
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

  _onSessionClosed(int code, String reason) {
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
          await setSigningDetails(wcClient.chainId);
          final creds = EthPrivateKey.fromHex(_privateKey);
          final tx = await _web3client.signTransaction(
            creds,
            wcEthTxToWeb3Tx(ethereumTransaction),
            chainId: wcClient.chainId,
          );
          wcClient.approveRequest<String>(
            id: id,
            result: bytesToHex(tx),
          );
        } catch (e) {
          wcClient.rejectRequest(id: id);
        } finally {
          Navigator.pop(context);
        }
      },
      onReject: () {
        wcClient.rejectRequest(id: id);
        Navigator.pop(context);
      },
    );
  }

  _onSwitchNetwork(int id, int chainIdNew) async {
    final currentChainIdData = getEthereumDetailsFromChainId(wcClient.chainId);
    final switchChainIdData = getEthereumDetailsFromChainId(chainIdNew);

    if (_chainId == chainIdNew) {
      wcClient.rejectRequest(id: id);
      return;
    }

    if (switchChainIdData == null) {
      wcClient.rejectRequest(id: id);
      return;
    }
    switchEthereumChain(
      context: context,
      currentChainIdData: currentChainIdData,
      switchChainIdData: switchChainIdData,
      onConfirm: () async {
        await setSigningDetails(wcClient.chainId);
        _web3client = Web3Client(
          switchChainIdData['rpc'],
          http.Client(),
        );
        await wcClient.updateSession(chainId: chainIdNew);
        wcClient.approveRequest<void>(id: id, result: null);
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(
          content: Text('Changed network to $chainIdNew.'),
        ));
        _chainId = chainIdNew;
        Navigator.pop(context);
      },
      onReject: () async {
        wcClient.rejectRequest(id: id);
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
        await setSigningDetails(wcClient.chainId);
        try {
          final creds = EthPrivateKey.fromHex(_privateKey);
          final txhash = await _web3client.sendTransaction(
            creds,
            wcEthTxToWeb3Tx(ethereumTransaction),
            chainId: wcClient.chainId,
          );
          debugPrint('txhash $txhash');
          wcClient.approveRequest<String>(
            id: id,
            result: txhash,
          );
        } catch (e) {
          wcClient.rejectRequest(id: id);
        } finally {
          Navigator.pop(context);
        }
      },
      onReject: () {
        wcClient.rejectRequest(id: id);
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
    List icons = wcClient.remotePeerMeta.icons;

    await signTransaction(
      gasPriceInWei_: ethereumTransaction.gasPrice,
      to: ethereumTransaction.to,
      from: ethereumTransaction.from,
      txData: ethereumTransaction.data,
      valueInWei_: ethereumTransaction.value,
      gasInWei_: ethereumTransaction.gas,
      networkIcon: icons.isNotEmpty ? icons[0] : null,
      context: context,
      blockChainCurrencySymbol: _currencySymbol,
      name: wcClient.remotePeerMeta.name,
      onConfirm: onConfirm,
      onReject: onReject,
      title: title,
      chainId: wcClient.chainId,
    );
  }

  _onSign(
    int id,
    WCEthereumSignMessage ethereumSignMessage,
  ) async {
    List icon = wcClient.remotePeerMeta.icons;
    String messageType = '';
    if (ethereumSignMessage.type == WCSignType.PERSONAL_MESSAGE) {
      messageType = personalSignKey;
    } else if (ethereumSignMessage.type == WCSignType.MESSAGE) {
      messageType = normalSignKey;
    } else if (ethereumSignMessage.type == WCSignType.TYPED_MESSAGE) {
      messageType = typedMessageSignKey;
    }

    await signMessage(
      messageType: messageType,
      context: context,
      data: ethereumSignMessage.data,
      networkIcon: icon.isNotEmpty ? icon[0] : null,
      name: wcClient.remotePeerMeta.name,
      onConfirm: () async {
        await setSigningDetails(wcClient.chainId);
        String signedDataHex;
        final credentials = EthPrivateKey.fromHex(_privateKey);
        if (ethereumSignMessage.type == WCSignType.TYPED_MESSAGE) {
          signedDataHex = EthSigUtil.signTypedData(
            privateKey: _privateKey,
            jsonData: ethereumSignMessage.data,
            version: TypedDataVersion.V4,
          );
        } else if (ethereumSignMessage.type == WCSignType.PERSONAL_MESSAGE) {
          Uint8List signedData = await credentials.signPersonalMessage(
            txDataToUintList(
              ethereumSignMessage.data,
            ),
          );
          signedDataHex = bytesToHex(signedData, include0x: true);
        } else if (ethereumSignMessage.type == WCSignType.MESSAGE) {
          try {
            signedDataHex = EthSigUtil.signMessage(
              privateKey: _privateKey,
              message: txDataToUintList(
                ethereumSignMessage.data,
              ),
            );
          } catch (e) {
            Uint8List signedData = await credentials.signPersonalMessage(
              txDataToUintList(
                ethereumSignMessage.data,
              ),
            );
            signedDataHex = bytesToHex(signedData, include0x: true);
          }
        }
        debugPrint('SIGNED $signedDataHex');
        wcClient.approveRequest<String>(
          id: id,
          result: signedDataHex,
        );
        Navigator.pop(context);
      },
      onReject: () {
        wcClient.rejectRequest(id: id);
        Navigator.pop(context);
      },
    );
  }
}
