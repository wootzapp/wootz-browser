import 'dart:convert';

import 'package:awesome_dialog/awesome_dialog.dart';
import 'package:cryptowallet/components/portfolio.dart';
import 'package:cryptowallet/main.dart';
import 'package:cryptowallet/screens/wallet_main_body.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/material.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:provider/provider.dart';

import '../models/provider.dart';
import '../utils/navigator_service.dart';
import '../utils/rpc_urls.dart';
import 'package:multi_value_listenable_builder/multi_value_listenable_builder.dart';

class ViewProfiles extends StatefulWidget {
  final List data;
  String currentPhrase;
  ViewProfiles({Key key, this.data, this.currentPhrase}) : super(key: key);
  @override
  State<ViewProfiles> createState() => _ViewProfilesState();
}

class _ViewProfilesState extends State<ViewProfiles> {
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  TextEditingController walletName = TextEditingController();
  // RxList seedList = [].obs;
  final seedList = ValueNotifier<List>([]);

  // RxBool toggler = false.obs;
  final toggler = ValueNotifier<bool>(false);

  @override
  void dispose() {
    walletName.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final userDefaultData = Provider.of<ProviderClass>(context);
    return Scaffold(
      key: _scaffoldKey,
      appBar: AppBar(
          title: Text(
        // AppLocalizations.of(context)!.wallet,
        'Profiles',
      )),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(15),
          child: MultiValueListenableBuilder(
              valueListenables: [seedList, toggler],
              builder: (context, value, child) {
                if (seedList.value == null || seedList.value.isEmpty) {
                  seedList.value = widget.data;
                }
                List<Widget> userProfiles = <Widget>[];
                if (seedList.value != null) {
                  for (int index = 0; index < seedList.value.length; index++) {
                    userProfiles.add(Dismissible(
                      onDismissed: (DismissDirection direction) {
                        toggler.value = !toggler.value;
                      },
                      key: UniqueKey(),
                      confirmDismiss: (DismissDirection direction) async {
                        if (direction.name == 'endToStart') {
                          return await _deleteProfile(index);
                        }
                        return await _editProfileName(index);
                      },
                      child: Row(
                        children: [
                          Container(
                            width: 300,
                            child: GestureDetector(
                              onTap: () async {
                                // final pref = Hive.box(secureStorageKey);
                                // await pref.put(
                                //   currentMmenomicKey,
                                //   seedList.value[index]['phrase'],
                                // );
                                // await pref.put(
                                //   currentUserWalletNameKey,
                                //   seedList.value[index]['name'],
                                // );
                                // Navigator.pop(context);
                                Navigator.of(context).push(MaterialPageRoute(
                                    builder: (_) => WalletMainBody()));

                                // RestartWidget.restartApp(context);
                              },
                              child: Container(
                                color: Colors.transparent,
                                width: double.infinity,
                                height: 70,
                                child: Card(
                                  shape: RoundedRectangleBorder(
                                      borderRadius: BorderRadius.circular(10)),
                                  margin: const EdgeInsets.symmetric(
                                      horizontal: 15, vertical: 5),
                                  child: Center(
                                      child: Padding(
                                    padding: const EdgeInsets.all(10.0),
                                    child: Row(
                                      mainAxisAlignment:
                                          MainAxisAlignment.spaceBetween,
                                      children: [
                                        Flexible(
                                          child: Text(
                                            // seedList.value[index]['name'] ??
                                            //     '${AppLocalizations.of(context)!.mainWallet} ${seedList.value.indexOf(seedList.value[index]) + 1}',
                                            seedList.value[index]['name'] ??
                                                'Main Profile ${seedList.value.indexOf(seedList.value[index]) + 1}',
                                            style: const TextStyle(
                                              fontSize: 20,
                                            ),
                                            overflow: TextOverflow.ellipsis,
                                          ),
                                        ),
                                      ],
                                    ),
                                  )),
                                ),
                              ),
                            ),
                          ),
                          GestureDetector(
                            onTap: () async {
                              final pref = Hive.box(secureStorageKey);
                              await pref.put(
                                currentMmenomicKey,
                                seedList.value[index]['phrase'],
                              );

                              await pref.put(
                                currentUserWalletNameKey,
                                seedList.value[index]['name'],
                              );
                              userDefaultData.setDefaultProfile();
                              setState(() {
                                // print('box clicked');
                                widget.currentPhrase =
                                    seedList.value[index]['phrase'];
                              });
                              // build(context);
                            },
                            child: Container(
                              decoration: BoxDecoration(
                                  shape: BoxShape.circle,
                                  color: widget.currentPhrase ==
                                          seedList.value[index]['phrase']
                                      ? Colors.blue
                                      : Colors.grey),
                              child: Padding(
                                padding: const EdgeInsets.all(2),
                                child: widget.currentPhrase ==
                                        seedList.value[index]['phrase']
                                    ? const Icon(
                                        Icons.check,
                                        size: 20,
                                        color: Colors.white,
                                      )
                                    : Container(
                                        width: 20,
                                        height: 20,
                                      ),
                              ),
                            ),
                          ),
                        ],
                      ),
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
                      background: Container(
                        color: Colors.blue,
                        margin: const EdgeInsets.symmetric(horizontal: 15),
                        alignment: Alignment.centerLeft,
                        child: const Padding(
                          padding: EdgeInsets.all(10),
                          child: Icon(
                            Icons.edit,
                            color: Colors.white,
                          ),
                        ),
                      ),
                    ));
                  }
                }
                return Column(
                  children: userProfiles,
                );
              }),
        ),
      ),
    );
  }

  Future<bool> _editProfileName(int index) async {
    // Get.closeAllSnackbars();
    ScaffoldMessenger.of(context).removeCurrentSnackBar();

    final pref = Hive.box(secureStorageKey);
    List mnemonicsList = json.decode(pref.get(mnemonicListKey));
    String currentmnemonic = pref.get(currentMmenomicKey);

    final result = await AwesomeDialog(
      showCloseIcon: false,
      context: context,
      closeIcon: const Icon(
        Icons.close,
      ),
      onDissmissCallback: (_) {},
      autoDismiss: false,
      animType: AnimType.SCALE,
      dialogType: DialogType.INFO,
      keyboardAware: true,
      body: Padding(
        padding: const EdgeInsets.all(8.0),
        child: Column(
          children: <Widget>[
            Text(
              AppLocalizations.of(context).editWalletName,
            ),
            const SizedBox(
              height: 10,
            ),
            Material(
              elevation: 0,
              color: Colors.blueGrey.withAlpha(40),
              child: TextFormField(
                controller: walletName
                  ..text = (mnemonicsList[index]['name'] ?? ''),
                autofocus: true,
                minLines: 1,
                decoration: InputDecoration(
                  border: InputBorder.none,
                  labelText: AppLocalizations.of(context).walletName,
                  prefixIcon: const Icon(Icons.text_fields),
                ),
              ),
            ),
            const SizedBox(
              height: 10,
            ),
            AnimatedButton(
              isFixedHeight: false,
              text: AppLocalizations.of(context).ok,
              pressEvent: () async {
                FocusManager.instance.primaryFocus?.unfocus();

                if (walletName.text.trim().isEmpty) {
                  // Get.back(result: false);
                  Navigator.of(context).pop(false);
                  return;
                }

                mnemonicsList[index]['name'] = walletName.text.trim();
                if (mnemonicsList[index]['phrase'] == currentmnemonic) {
                  await pref.put(
                    currentUserWalletNameKey,
                    mnemonicsList[index]['name'],
                  );
                }
                await pref.put(
                  mnemonicListKey,
                  json.encode(mnemonicsList),
                );
                seedList.value = json.decode(pref.get(mnemonicListKey));

                // Get.back(result: true);
                Navigator.of(context).pop(true);
              },
            )
          ],
        ),
      ),
    ).show();
    return result ?? false;
  }

  Future<bool> _deleteProfile(int index) async {
    // Get.closeAllSnackbars();
    ScaffoldMessenger.of(context).removeCurrentSnackBar();
    final pref = Hive.box(secureStorageKey);
    List mnemonicsList = json.decode(pref.get(mnemonicListKey));

    if (pref.get(currentMmenomicKey).toString().toLowerCase() ==
        mnemonicsList[index]['phrase'].toString().toLowerCase()) {
      // Get.snackbar(
      //   '',
      //   'Can not delete currently used wallet',
      //   colorText: Colors.white,
      //   backgroundColor: Colors.red,
      // );
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: const Text('Can not delete currently used wallet'),
        backgroundColor: Colors.red,
        action: SnackBarAction(
          label: 'OK',
          onPressed: () {},
          textColor: Colors.white,
        ),
      ));

      return false;
    }
    bool deleted = await AwesomeDialog(
      context: context,
      dialogType: DialogType.WARNING,
      animType: AnimType.BOTTOMSLIDE,
      autoDismiss: false,
      closeIcon: const Icon(
        Icons.close,
      ),
      onDissmissCallback: (type) {},
      dismissOnTouchOutside: true,
      title: AppLocalizations.of(context).confirmWalletDelete,
      btnOkText: AppLocalizations.of(context).delete,
      btnOkColor: Colors.red,
      btnCancelColor: appBackgroundblue,
      desc: AppLocalizations.of(context).confirmWalletDeleteDescription,
      btnCancelOnPress: () {
        // Get.back(result: false);
        Navigator.of(context).pop(false);
      },
      btnOkOnPress: () async {
        if (await authenticate(context)) {
          mnemonicsList.removeAt(index);

          await pref.put(
            mnemonicListKey,
            json.encode(mnemonicsList),
          );

          seedList.value = mnemonicsList;
          // Get.back(result: true);
          Navigator.of(context).pop(true);
        } else {
          // Get.back(result: false);
          Navigator.of(context).pop(false);
        }
      },
    ).show();

    return deleted;
  }
}
