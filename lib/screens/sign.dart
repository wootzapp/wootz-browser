import 'dart:convert';
import 'dart:core';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:cached_network_image/cached_network_image.dart';
import 'package:flutter/material.dart';
import 'package:flutter/src/foundation/key.dart';
import 'package:flutter/src/widgets/container.dart';
import 'package:flutter/src/widgets/framework.dart';
import 'package:hive/hive.dart';
import 'package:wallet_connect/models/message_type.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

import '../components/loader.dart';
import '../utils/app_config.dart';
import '../utils/json_viewer.dart';
import '../utils/rpc_urls.dart';
import '../utils/slide_up_panel.dart';

class SignMessage extends StatefulWidget {
  final String data;
  final List<String> raw;
  final String networkIcon;
  final String name;
  final Function onConfirm;
  final Function onReject;
  final String messageType;
  final String decoded;
  final DateTime dateTime;
  final int chainId;
  final dynamic uri;
  final String version;
  final String topic;
  final String method;
  const SignMessage({
    Key key,
    this.method,
    this.raw,
    this.data,
    this.networkIcon,
    this.name,
    this.onConfirm,
    this.onReject,
    this.messageType,
    this.decoded,
    this.dateTime,
    this.chainId,
    this.uri,
    this.version,
    this.topic,
  }) : super(key: key);

  @override
  State<SignMessage> createState() => _SignMessageState();
}

class _SignMessageState extends State<SignMessage>
    with TickerProviderStateMixin {
  String walletName;
  TabController _tabController;
  void initstate() {
    super.initState();
    _tabController = TabController(length: 3, vsync: this);
  }

  String getWalletName() {
    final pref = Hive.box(secureStorageKey);
    final mnemonics = jsonDecode(pref.get(mnemonicListKey)) as List;
    final currentMnemonic = pref.get(currentMmenomicKey);
    int ind = -1;
    for (int i = 0; i < mnemonics.length; i++) {
      if (mnemonics[i]['phrase'] == currentMnemonic) {
        ind = i;
      }
    }
    print('ind $ind');
    walletName = pref.get(currentUserWalletNameKey) ??
        '${AppLocalizations.of(context).mainWallet} ${ind + 1}';
    return walletName;
  }

  Widget getRawData() {
    // print(widget.raw);
    return Column(
        children: widget.raw.map((item) {
      return Container(
        padding: const EdgeInsets.all(10),
        child: Text(
          item,
        ),
      );
    }).toList());
  }

  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 3,
      child: Scaffold(
        body: Padding(
          padding: const EdgeInsets.only(left: 25.0, right: 25, bottom: 25),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.start,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Container(
                padding: const EdgeInsets.only(bottom: 8.0),
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    const Align(
                      alignment: Alignment.centerRight,
                      child: IconButton(
                        onPressed: null,
                        icon: Icon(
                          Icons.close,
                          color: Colors.transparent,
                        ),
                      ),
                    ),
                    Text(
                      AppLocalizations.of(context).signMessage,
                      style: const TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 20.0,
                      ),
                    ),
                    Align(
                      alignment: Alignment.centerRight,
                      child: IconButton(
                        onPressed: () {
                          if (Navigator.canPop(context)) {
                            widget.onReject();
                          }
                        },
                        icon: const Icon(Icons.close),
                      ),
                    ),
                  ],
                ),
              ),
              if (widget.networkIcon != null)
                Container(
                  height: 50.0,
                  width: 50.0,
                  padding: const EdgeInsets.only(bottom: 8.0),
                  child: CachedNetworkImage(
                    imageUrl: ipfsTohttp(widget.networkIcon),
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
              if (widget.name != null)
                Text(
                  widget.name,
                  style: const TextStyle(
                    fontWeight: FontWeight.normal,
                    fontSize: 16.0,
                  ),
                ),
              Container(
                width: double.infinity,
                height: 50,
                child: TabBar(
                  isScrollable: true,
                  controller: _tabController,
                  tabs: const [
                    Tab(
                      child: Text('data'),
                    ),
                    Tab(
                      child: Text('raw'),
                    ),
                    Tab(child: Text('credentials'))
                  ],
                ),
              ),
              const SizedBox(
                height: 10,
              ),
              Container(
                width: double.infinity,
                height: 400,
                child: TabBarView(
                  controller: _tabController,
                  children: [
                    Theme(
                      data: Theme.of(context)
                          .copyWith(dividerColor: Colors.transparent),
                      child: Padding(
                        padding: const EdgeInsets.only(bottom: 8.0),
                        child: ExpansionTile(
                          initiallyExpanded: true,
                          tilePadding: EdgeInsets.zero,
                          title: Text(
                            AppLocalizations.of(context).message,
                            style: const TextStyle(
                              fontWeight: FontWeight.bold,
                              fontSize: 18.0,
                            ),
                          ),
                          children: [
                            if (widget.messageType == typedMessageSignKey)
                              JsonViewer(
                                json.decode(widget.decoded),
                                fontSize: 16,
                              )
                            else
                              Text(
                                widget.decoded,
                                style: const TextStyle(fontSize: 16.0),
                              ),
                          ],
                        ),
                      ),
                    ),
                    getRawData(),
                    Column(
                      children: [
                        Text(
                          'WalletName : ${getWalletName()}',
                          style: const TextStyle(fontSize: 16),
                          textAlign: TextAlign.left,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        Text(
                          'method : ${widget.method}',
                          style: const TextStyle(fontSize: 16),
                          textAlign: TextAlign.left,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        Text(
                          'messageType : ${widget.messageType} sign',
                          style: const TextStyle(fontSize: 16),
                          textAlign: TextAlign.left,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        Text(
                          'chainId : ${widget.chainId}',
                          style: const TextStyle(fontSize: 16),
                          textAlign: TextAlign.left,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        Text(
                          'uri : ${widget.uri}',
                          style: const TextStyle(fontSize: 16),
                          textAlign: TextAlign.left,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        Text(
                          'version : ${widget.version}',
                          style: const TextStyle(fontSize: 16),
                          textAlign: TextAlign.left,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        Text(
                          'Issued At : ${widget.dateTime.toString()}',
                          style: const TextStyle(fontSize: 16),
                          textAlign: TextAlign.left,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                      ],
                    ),
                  ],
                ),
              ),
              Row(
                children: [
                  Expanded(
                    child: TextButton(
                      style: TextButton.styleFrom(
                        foregroundColor: Colors.white,
                        backgroundColor: const Color(0xff007bff),
                      ),
                      onPressed: () async {
                        if (await authenticate(context)) {
                          widget.onConfirm();
                          // Navigator.pop(context);
                        } else {
                          widget.onReject();
                          // Navigator.pop(context);
                        }
                      },
                      child: Text(
                        AppLocalizations.of(context).sign,
                        style: const TextStyle(fontSize: 18),
                      ),
                    ),
                  ),
                  const SizedBox(width: 16.0),
                  Expanded(
                    child: TextButton(
                      style: TextButton.styleFrom(
                        foregroundColor: Colors.white,
                        backgroundColor: const Color(0xff007bff),
                      ),
                      onPressed: () {
                        widget.onReject();
                        // Navigator.pop(context);
                      },
                      child: Text(
                        AppLocalizations.of(context).reject,
                        style: const TextStyle(fontSize: 18),
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
