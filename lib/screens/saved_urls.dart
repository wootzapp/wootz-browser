import 'dart:convert';

import 'package:awesome_dialog/awesome_dialog.dart';
import 'package:cryptowallet/screens/dapp.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:hive/hive.dart';
import 'package:get/get.dart';

import '../utils/app_config.dart';

class SavedUrls extends StatefulWidget {
  final List data;
  final String title;
  final String emptyText;
  final String savedKey;
  const SavedUrls(
    this.title,
    this.emptyText,
    this.savedKey, {
    Key key,
    this.data,
  }) : super(key: key);
  @override
  State<SavedUrls> createState() => _SavedUrlsState();
}

class _SavedUrlsState extends State<SavedUrls> {
  RxList savedUrl = ([]).obs;
  int removals = 0;
  RxBool toggler = false.obs;

  @override
  initState() {
    super.initState();
    savedUrl.value = widget.data;
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
          widget.title,
        ),
        actions: [
          IconButton(
            onPressed: () async {
              await AwesomeDialog(
                context: context,
                dialogType: DialogType.WARNING,
                animType: AnimType.BOTTOMSLIDE,
                autoDismiss: false,
                closeIcon: const Icon(
                  Icons.close,
                ),
                onDissmissCallback: (type) {},
                dismissOnTouchOutside: true,
                title: AppLocalizations.of(context).clearBrowsingData,
                btnOkText: AppLocalizations.of(context).delete,
                btnOkColor: Colors.red,
                btnCancelColor: appBackgroundblue,
                btnCancelOnPress: () {
                  Get.back(result: false);
                },
                btnOkOnPress: () async {
                  final pref = Hive.box(secureStorageKey);
                  await pref.delete(widget.savedKey);
                  savedUrl.value = [];
                  Get.back();
                },
              ).show();
            },
            icon: const Icon(Icons.delete),
          ),
        ],
      ),
      body: SafeArea(
        child: SingleChildScrollView(
          child: Padding(
            padding: const EdgeInsets.all(25),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Obx(() {
                  final urlWidgets = <Widget>[];
                  for (int i = 0; i < savedUrl.length; i++) {
                    Map urlDetails = savedUrl[i];
                    if (urlDetails == null) continue;
                    urlWidgets.add(
                      Dismissible(
                        secondaryBackground: Container(
                          color: Colors.red,
                          margin: const EdgeInsets.symmetric(horizontal: 15),
                          alignment: Alignment.centerRight,
                          child: const Padding(
                            padding: EdgeInsets.only(right: 10),
                            child: Icon(
                              Icons.delete,
                              color: Colors.white,
                            ),
                          ),
                        ),
                        background: Container(),
                        onDismissed: (DismissDirection direction) {
                          toggler.value = !toggler.value;
                        },
                        key: UniqueKey(),
                        direction: DismissDirection.endToStart,
                        confirmDismiss: (DismissDirection direction) async {
                          if (direction.name == 'endToStart') {
                            final pref = Hive.box(secureStorageKey);
                            final List currentArrayState = [...savedUrl];
                            currentArrayState.removeAt(i);
                            savedUrl.value = currentArrayState;
                            await pref.put(
                                widget.savedKey, jsonEncode(savedUrl));
                            return true;
                          }
                          return false;
                        },
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            InkWell(
                              onTap: () async {
                                final pref = Hive.box(secureStorageKey);
                                bool hasWallet =
                                    pref.get(currentMmenomicKey) != null;

                                Widget nextWidget;
                                if (hasWallet) {
                                  nextWidget = await dappWidget(
                                    context,
                                    urlDetails['url'],
                                  );
                                } else {
                                  nextWidget = Dapp(
                                    provider: '',
                                    init: '',
                                    data: urlDetails['url'],
                                  );
                                }

                                if (widget.savedKey == historyKey) {
                                  Get.back(result: urlDetails['url']);
                                } else {
                                  Get.to(
                                    nextWidget,
                                    transition: Transition.leftToRight,
                                  );
                                }
                              },
                              child: Padding(
                                padding: const EdgeInsets.all(8.0),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    Text(
                                      urlDetails['title'],
                                      style: const TextStyle(fontSize: 18),
                                      overflow: TextOverflow.ellipsis,
                                    ),
                                    const SizedBox(
                                      height: 5,
                                    ),
                                    Text(
                                      urlDetails['url'],
                                      style: const TextStyle(
                                        fontSize: 16,
                                        color: Colors.grey,
                                      ),
                                      overflow: TextOverflow.ellipsis,
                                    ),
                                  ],
                                ),
                              ),
                            ),
                            const Divider(),
                          ],
                        ),
                      ),
                    );
                  }
                  if (urlWidgets.isEmpty) {
                    return Text(
                      widget.emptyText,
                      style: const TextStyle(fontSize: 18),
                    );
                  }
                  return Column(
                    children: urlWidgets,
                  );
                }),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
