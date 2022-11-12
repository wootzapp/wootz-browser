import 'dart:convert';

import 'package:awesome_dialog/awesome_dialog.dart';
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
  final savedUrl = ValueNotifier<List>([]);
  int removals = 0;

  @override
  initState() {
    super.initState();
    savedUrl.value = widget.data;
  }

  @override
  Widget build(BuildContext context) {
    final urlWidgets = <Widget>[];
    for (int i = 0; i < savedUrl.value.length; i++) {
      Map urlDetails = savedUrl.value[i];
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
            setState(() {});
          },
          key: UniqueKey(),
          direction: DismissDirection.endToStart,
          confirmDismiss: (DismissDirection direction) async {
            if (direction.name == 'endToStart') {
              final pref = Hive.box(secureStorageKey);
              final List currentArrayState = [...savedUrl.value];
              currentArrayState.removeAt(i);
              savedUrl.value = currentArrayState;
              await pref.put(widget.savedKey, jsonEncode(savedUrl.value));
              return true;
            }
            return false;
          },
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              InkWell(
                onTap: () async {
                  await dappWidget(context, urlDetails['url']);
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
                  if (mounted) {
                    setState(() {});
                  }
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
                ...urlWidgets,
                ValueListenableBuilder(
                  valueListenable: savedUrl,
                  builder: ((_, List savedUrl_, __) {
                    if (savedUrl_.isEmpty) {
                      return Text(
                        widget.emptyText,
                        style: const TextStyle(fontSize: 18),
                      );
                    }
                    return Container();
                  }),
                )
              ],
            ),
          ),
        ),
      ),
    );
  }
}
