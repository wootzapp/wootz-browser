import 'dart:convert';

import 'package:awesome_dialog/awesome_dialog.dart';
import 'package:cached_network_image/cached_network_image.dart';
import 'package:cryptowallet/components/loader.dart';
import 'package:cryptowallet/main.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/wc_connector.dart';
import 'package:flutter/material.dart';
import 'package:hive/hive.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:intl/intl.dart';
import 'package:wallet_connect/wallet_connect.dart';

import '../utils/rpc_urls.dart';

class WalletConnectPreview extends StatefulWidget {
  final Map data;
  final String? currentPhrase;
  const WalletConnectPreview({Key? key, required this.data, this.currentPhrase})
      : super(key: key);
  @override
  State<WalletConnectPreview> createState() => _WalletConnectPreviewState();
}

class _WalletConnectPreviewState extends State<WalletConnectPreview> {
  String networkName = '';
  String networkIcon = '';
  late List icons;
  late DateTime trnDate;
  @override
  initState() {
    super.initState();
    icons = widget.data['remotePeerMeta']['icons'];
    trnDate = DateFormat("yyyy-MM-dd hh:mm:ss").parse(widget.data['date']);
    List evmValues = getEVMBlockchains().values.toList();
    List evmKeys = getEVMBlockchains().keys.toList();
    for (int i = 0; i < evmValues.length; i++) {
      if (evmValues[i]['chainId'] == widget.data['chainId']) {
        networkName = evmKeys[i];
        networkIcon = evmValues[i]['image'];
        break;
      }
    }
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('WalletConnect'),
      ),
      body: SafeArea(
        child: Padding(
            padding: const EdgeInsets.all(15),
            child: Column(
              children: [
                Card(
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(10)),
                  child: Padding(
                    padding: const EdgeInsets.all(20.0),
                    child: Column(children: [
                      Row(
                        children: [
                          if (icons != null && icons.isNotEmpty)
                            Container(
                              // height: 50.0,
                              width: 50.0,
                              padding: const EdgeInsets.only(bottom: 8.0),
                              child: CachedNetworkImage(
                                imageUrl: ipfsTohttp(
                                  icons[0],
                                ),
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
                                errorWidget: (context, url, error) =>
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
                          if (icons.isNotEmpty)
                            const SizedBox(
                              width: 10,
                            ),
                          Flexible(
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  widget.data['remotePeerMeta']['name'],
                                  style: const TextStyle(
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16,
                                  ),
                                  maxLines: 1,
                                  overflow: TextOverflow.ellipsis,
                                ),
                                Text(
                                  widget.data['remotePeerMeta']['url'],
                                  style: const TextStyle(
                                    color: Colors.grey,
                                    fontSize: 15,
                                  ),
                                  maxLines: 1,
                                  overflow: TextOverflow.ellipsis,
                                ),
                              ],
                            ),
                          )
                        ],
                      )
                    ]),
                  ),
                ),
                const SizedBox(
                  height: 10,
                ),
                Card(
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(10)),
                  child: Padding(
                    padding: const EdgeInsets.all(20.0),
                    child: Column(children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceBetween,
                        children: [
                          const Text(
                            'Connected',
                            style: TextStyle(
                              fontWeight: FontWeight.bold,
                              fontSize: 16,
                            ),
                          ),
                          Text(
                            '${trnDate.day} ${months[trnDate.month - 1]}, ${trnDate.hour}:${trnDate.minute}',
                            style: const TextStyle(
                              color: Colors.grey,
                              fontSize: 16,
                            ),
                          )
                        ],
                      )
                    ]),
                  ),
                ),
                const SizedBox(
                  height: 10,
                ),
                Card(
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(10)),
                  child: Padding(
                    padding: const EdgeInsets.all(20.0),
                    child: Row(
                      children: [
                        CircleAvatar(
                          backgroundImage: AssetImage(networkIcon),
                          backgroundColor: Theme.of(context).backgroundColor,
                        ),
                        const SizedBox(
                          width: 20,
                        ),
                        Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Text(
                              networkName,
                              style: const TextStyle(
                                fontWeight: FontWeight.bold,
                                fontSize: 16,
                              ),
                            ),
                            Text(
                              ellipsify(
                                  str: widget.data['address'], maxLength: 20),
                              style: const TextStyle(
                                color: Colors.grey,
                                fontSize: 16,
                              ),
                            )
                          ],
                        )
                      ],
                    ),
                  ),
                ),
                const SizedBox(
                  height: 10,
                ),
                InkWell(
                  onTap: () async {
                    await WcConnector.wcClient.disconnect();
                    await WcConnector.removedCurrentSession();
                  },
                  child: const Text(
                    'Disconnect',
                    style: TextStyle(
                      color: red,
                      fontSize: 16,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                )
              ],
            )),
      ),
    );
  }
}
