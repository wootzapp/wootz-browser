import 'dart:convert';

import 'package:awesome_dialog/awesome_dialog.dart';
import 'package:cryptowallet/screens/dapp.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:hive/hive.dart';
import 'package:multi_value_listenable_builder/multi_value_listenable_builder.dart';
// import 'package:get/get.dart';

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
  // RxList savedUrl = ([]).obs;
  final savedUrl = ValueNotifier<List>([]);
  int removals = 0;
  // RxBool toggler = false.obs;
  final toggler = ValueNotifier<bool>(false);

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
                  // Get.back(result: false);
                  Navigator.of(context).pop(false);
                },
                btnOkOnPress: () async {
                  final pref = Hive.box(secureStorageKey);
                  await pref.delete(widget.savedKey);
                  savedUrl.value = [];
                  // Get.back();
                  Navigator.of(context).pop();
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
                MultiValueListenableBuilder(
                    valueListenables: [savedUrl, toggler],
                    builder: (context, value, child) {
                      final urlWidgets = <Widget>[];
                      for (int i = 0; i < savedUrl.value.length; i++) {
                        Map urlDetails = savedUrl.value[i];
                        if (urlDetails == null) continue;
                        urlWidgets.add(
                          Dismissible(
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
                            background: Container(),
                            onDismissed: (DismissDirection direction) {
                              toggler.value = !toggler.value;
                            },
                            key: UniqueKey(),
                            direction: DismissDirection.endToStart,
                            confirmDismiss: (DismissDirection direction) async {
                              if (direction.name == 'endToStart') {
                                final pref = Hive.box(secureStorageKey);
                                final List currentArrayState = [
                                  ...savedUrl.value
                                ];
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
                                    Widget nextWidget = await dappWidget(
                                      context,
                                      urlDetails['url'],
                                    );

                                    if (widget.savedKey == historyKey) {
                                      // Get.back(result: urlDetails['url']);
                                      Navigator.of(context)
                                          .pop(urlDetails['url']);
                                    } else {
                                      Navigator.of(context).push(
                                          MaterialPageRoute(
                                              builder: (_) => nextWidget));
                                      // Get.to(
                                      //   nextWidget,
                                      //   transition: Transition.leftToRight,
                                      // );
                                    }
                                  },
                                  child: Padding(
                                    padding: const EdgeInsets.all(8.0),
                                    child: Column(
                                      crossAxisAlignment:
                                          CrossAxisAlignment.start,
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
