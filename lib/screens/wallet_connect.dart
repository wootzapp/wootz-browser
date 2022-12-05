import 'dart:convert';

import 'package:cached_network_image/cached_network_image.dart';
import 'package:cryptowallet/screens/wallet_connect_preview.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:cryptowallet/utils/wc_connector.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:hive/hive.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:wallet_connect/wallet_connect.dart';
import '../components/loader.dart';
import '../main.dart';
import '../utils/app_config.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

import '../utils/qr_scan_view.dart';

class WalletConnect extends StatefulWidget {
  const WalletConnect({Key key}) : super(key: key);

  @override
  _WalletConnectState createState() => _WalletConnectState();
}

class _WalletConnectState extends State<WalletConnect> {
  Box prefs;
  TextEditingController _textEditingController;
  String connectedWebsiteUrl = "";
  List previousSessions;

  @override
  void initState() {
    prefs = Hive.box(secureStorageKey);
    if (prefs.get(wcSessionKey) != null) {
      previousSessions = jsonDecode(prefs.get(wcSessionKey)) as List;
      previousSessions = previousSessions.reversed.toList();
    }
    super.initState();
    _textEditingController = TextEditingController();
  }

  @override
  void dispose() {
    _textEditingController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('WalletConnect'),
      ),
      body: RefreshIndicator(
        onRefresh: () async {
          await Future.delayed(const Duration(seconds: 2));
          if (mounted) {
            setState(() {});
          }
        },
        child: SafeArea(
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Padding(
              padding: const EdgeInsets.all(25.0),
              child: ValueListenableBuilder(
                  valueListenable: Hive.box(secureStorageKey)
                      .listenable(keys: [wcSessionKey]),
                  builder: (context, _, __) {
                    return Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        SizedBox(
                          width: MediaQuery.of(context).size.width * 0.85,
                          height: 50,
                          child: ElevatedButton(
                            style: ButtonStyle(
                              backgroundColor:
                                  MaterialStateProperty.resolveWith(
                                      (states) => appBackgroundblue),
                              shape: MaterialStateProperty.resolveWith(
                                (states) => RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(10),
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
                                    AppLocalizations.of(context).connectViAQR,
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
                                  WcConnector.qrScanHandler(value);
                                }
                              });
                            },
                          ),
                        ),
                        const SizedBox(height: 20),
                        Container(
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
                          width: MediaQuery.of(context).size.width * 0.85,
                          height: 50,
                          child: ElevatedButton(
                            style: ButtonStyle(
                              elevation: MaterialStateProperty.resolveWith(
                                (states) => 0,
                              ),
                              backgroundColor:
                                  MaterialStateProperty.resolveWith(
                                (states) =>
                                    Theme.of(context).scaffoldBackgroundColor,
                              ),
                              shape: MaterialStateProperty.resolveWith(
                                (states) => RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(10),
                                ),
                              ),
                            ),
                            child: Align(
                              alignment: Alignment.center,
                              child: Text(
                                AppLocalizations.of(context).connectViACode,
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
                                      AppLocalizations.of(context).pasteCode,
                                  pageBuilder: (context, _, __) {
                                    return SimpleDialog(
                                      title: Text(
                                        AppLocalizations.of(context).pasteCode,
                                      ),
                                      titlePadding: const EdgeInsets.fromLTRB(
                                          16.0, 16.0, 16.0, .0),
                                      contentPadding:
                                          const EdgeInsets.all(16.0),
                                      children: [
                                        TextFormField(
                                          controller: _textEditingController,
                                          decoration: InputDecoration(
                                            label: Text(
                                              AppLocalizations.of(context)
                                                  .enterCode,
                                            ),

                                            focusedBorder:
                                                const OutlineInputBorder(
                                                    borderRadius:
                                                        BorderRadius.all(
                                                            Radius.circular(
                                                                10.0)),
                                                    borderSide:
                                                        BorderSide.none),
                                            border: const OutlineInputBorder(
                                                borderRadius: BorderRadius.all(
                                                    Radius.circular(10.0)),
                                                borderSide: BorderSide.none),
                                            enabledBorder:
                                                const OutlineInputBorder(
                                                    borderRadius:
                                                        BorderRadius.all(
                                                            Radius.circular(
                                                                10.0)),
                                                    borderSide:
                                                        BorderSide.none), // you
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
                                                  Navigator.pop(context),
                                              child: Text(
                                                AppLocalizations.of(context)
                                                    .confirm,
                                              ),
                                            ),
                                          ],
                                        ),
                                      ],
                                    );
                                  }).then((_) {
                                if (_textEditingController.text.isNotEmpty) {
                                  WcConnector.qrScanHandler(
                                    _textEditingController.text,
                                  );
                                  _textEditingController.clear();
                                }
                              });
                            },
                          ),
                        ),
                        if (WcConnector.wcClient.isConnected) ...[
                          const SizedBox(height: 20),
                          SizedBox(
                            width: MediaQuery.of(context).size.width * 0.85,
                            height: 50,
                            child: ElevatedButton(
                              style: ButtonStyle(
                                backgroundColor:
                                    MaterialStateProperty.resolveWith(
                                        (states) => red),
                                shape: MaterialStateProperty.resolveWith(
                                  (states) => RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(10),
                                  ),
                                ),
                              ),
                              child: Align(
                                alignment: Alignment.center,
                                child: Text(
                                  AppLocalizations.of(context).killSession,
                                  style: const TextStyle(
                                    fontSize: 16,
                                    fontWeight: FontWeight.bold,
                                    color: Colors.white,
                                  ),
                                  textAlign: TextAlign.center,
                                ),
                              ),
                              onPressed: () async {
                                await WcConnector.wcClient.killSession();
                                await WcConnector.removedCurrentSession();
                              },
                            ),
                          ),
                        ],
                        const SizedBox(height: 20),
                        if (previousSessions != null)
                          for (int i = 0; i < previousSessions.length; i++) ...[
                            Dismissible(
                              onDismissed: (DismissDirection direction) {
                                if (mounted) {
                                  setState(() {});
                                }
                              },
                              direction: DismissDirection.endToStart,
                              secondaryBackground: Container(
                                color: Colors.red,
                                margin:
                                    const EdgeInsets.symmetric(horizontal: 15),
                                alignment: Alignment.centerRight,
                                child: const Padding(
                                  padding: EdgeInsets.only(right: 10),
                                  child: Icon(
                                    Icons.delete,
                                    color: Colors.white,
                                  ),
                                ),
                              ),
                              background: Container(
                                color: Colors.blue,
                                margin:
                                    const EdgeInsets.symmetric(horizontal: 15),
                                alignment: Alignment.centerLeft,
                                child: const Padding(
                                  padding: EdgeInsets.all(10),
                                  child: Icon(
                                    Icons.edit,
                                    color: Colors.white,
                                  ),
                                ),
                              ),
                              key: UniqueKey(),
                              confirmDismiss:
                                  (DismissDirection direction) async {
                                if (direction.name == 'endToStart') {
                                  try {
                                    if (prefs.get(wcSessionKey) == null) {
                                      return false;
                                    }
                                    List sessions =
                                        jsonDecode(prefs.get(wcSessionKey))
                                            as List;

                                    sessions.removeAt(i);
                                    previousSessions = sessions;
                                    await prefs.put(
                                        wcSessionKey, jsonEncode(sessions));
                                    return true;
                                  } catch (_) {
                                    return false;
                                  }
                                }
                                return false;
                              },
                              child: GestureDetector(
                                onTap: () async {
                                  Navigator.push(
                                    context,
                                    MaterialPageRoute(
                                      builder: (ctx) => WalletConnectPreview(
                                          data: previousSessions[i]),
                                    ),
                                  );
                                },
                                child: Card(
                                  shape: RoundedRectangleBorder(
                                      borderRadius: BorderRadius.circular(10)),
                                  child: Container(
                                    padding: const EdgeInsets.all(10),
                                    child: Row(
                                      mainAxisAlignment:
                                          MainAxisAlignment.spaceBetween,
                                      children: [
                                        Expanded(
                                          child: Row(
                                            children: [
                                              if (previousSessions[i]
                                                              ['remotePeerMeta']
                                                          ['icons'] !=
                                                      null &&
                                                  previousSessions[i][
                                                                  'remotePeerMeta']
                                                              ['icons']
                                                          .length !=
                                                      0)
                                                Container(
                                                  height: 50.0,
                                                  width: 50.0,
                                                  padding:
                                                      const EdgeInsets.only(
                                                          bottom: 8.0),
                                                  child: CachedNetworkImage(
                                                    imageUrl: ipfsTohttp(
                                                      previousSessions[i]
                                                              ['remotePeerMeta']
                                                          ['icons'][0],
                                                    ),
                                                    placeholder:
                                                        (context, url) =>
                                                            Column(
                                                      mainAxisAlignment:
                                                          MainAxisAlignment
                                                              .center,
                                                      mainAxisSize:
                                                          MainAxisSize.min,
                                                      children: const [
                                                        SizedBox(
                                                          width: 20,
                                                          height: 20,
                                                          child: Loader(
                                                            color:
                                                                appPrimaryColor,
                                                          ),
                                                        )
                                                      ],
                                                    ),
                                                    errorWidget:
                                                        (context, url, error) =>
                                                            const Icon(
                                                      Icons.error,
                                                      color: Colors.red,
                                                    ),
                                                  ),
                                                )
                                              else
                                                const SizedBox(
                                                  height: 50.0,
                                                  width: 50.0,
                                                ),
                                              const SizedBox(
                                                width: 10,
                                              ),
                                              Flexible(
                                                child: Column(
                                                  crossAxisAlignment:
                                                      CrossAxisAlignment.start,
                                                  children: [
                                                    Text(
                                                      previousSessions[i]
                                                              ['remotePeerMeta']
                                                          ['name'],
                                                      style: const TextStyle(
                                                        fontWeight:
                                                            FontWeight.bold,
                                                      ),
                                                      maxLines: 1,
                                                      overflow:
                                                          TextOverflow.ellipsis,
                                                    ),
                                                    Text(
                                                      previousSessions[i]
                                                              ['remotePeerMeta']
                                                          ['url'],
                                                      maxLines: 1,
                                                      overflow:
                                                          TextOverflow.ellipsis,
                                                    ),
                                                  ],
                                                ),
                                              ),
                                            ],
                                          ),
                                        ),
                                        const Icon(Icons.arrow_forward_ios)
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                            ),
                            const Divider(
                              height: 10,
                            ),
                          ],
                      ],
                    );
                  }),
            ),
          ),
        ),
      ),
    );
  }
}
