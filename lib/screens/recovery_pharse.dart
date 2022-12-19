import 'package:blockfrost/blockfrost.dart';
import 'package:bs58check/bs58check.dart';
import 'package:cryptowallet/google_drive/file.dart';
import 'package:cryptowallet/screens/confirm_seed_phrase.dart';
import 'package:cryptowallet/utils/alt_ens.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/slide_up_panel.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:bip39/bip39.dart' as bip39;
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:get/get.dart';
import 'package:hex/hex.dart';
import 'package:ntcdcrypto/ntcdcrypto.dart';
import 'package:path_provider/path_provider.dart';
import 'package:screenshot_callback/screenshot_callback.dart';

import '../utils/rpc_urls.dart';

class RecoveryPhrase extends StatefulWidget {
  final String data;
  final bool verify;
  final bool add;
  const RecoveryPhrase({Key key, this.data, this.verify, this.add})
      : super(key: key);

  @override
  _RecoveryPhraseState createState() => _RecoveryPhraseState();
}

class _RecoveryPhraseState extends State<RecoveryPhrase>
    with WidgetsBindingObserver {
  // disallow screenshots
  ScreenshotCallback screenshotCallback = ScreenshotCallback();
  RxBool invisiblemnemonic = false.obs;
  RxBool securitydialogOpen = false.obs;
  String data_;
  final scaffoldKey = GlobalKey<ScaffoldState>();
  String chooseLength = "12";
  Map bip39Strength = {
    "128": "12",
    "160": "15",
    "192": "18",
    "224": "21",
    "256": "24"
  };

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) async {
    super.didChangeAppLifecycleState(state);
    switch (state) {
      case AppLifecycleState.resumed:
        if (invisiblemnemonic.value) {
          invisiblemnemonic.value = false;
          if (await authenticate(context)) {
            await disEnableScreenShot();

            securitydialogOpen.value = false;
          } else {
            SystemNavigator.pop();
          }
        }
        break;
      case AppLifecycleState.paused:
        if (!securitydialogOpen.value) {
          invisiblemnemonic.value = true;
          securitydialogOpen.value = true;
        }
        break;
      default:
        break;
    }
  }

  bool successSaving = false;
  @override
  void initState() {
    super.initState();
    data_ = widget.data;
    screenshotCallback.addListener(() {
      showDialogWithMessage(
        context: context,
        message: AppLocalizations.of(context).youCantScreenshot,
      );
    });
    WidgetsBinding.instance?.addObserver(this);
    disEnableScreenShot();
  }

  @override
  void dispose() {
    WidgetsBinding.instance?.removeObserver(this);
    enableScreenShot();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(AppLocalizations.of(context).yourSecretPhrase),
      ),
      key: scaffoldKey,
      body: Obx(
        () {
          List mmemonic = data_.split(' ');
          int currentIndex = 0;
          return securitydialogOpen.value
              ? Container()
              : SafeArea(
                  child: SingleChildScrollView(
                    child: Padding(
                      padding: const EdgeInsets.all(25),
                      child: Column(
                        children: [
                          Text(
                            AppLocalizations.of(context).writeDownYourmnemonic,
                            textAlign: TextAlign.center,
                            style: const TextStyle(color: Colors.grey),
                          ),
                          const SizedBox(
                            height: 20,
                          ),
                          for (int i = 0; i < mmemonic.length ~/ 3; i++) ...[
                            Row(
                              children: [
                                Expanded(
                                  child: Card(
                                    child: Padding(
                                      padding: const EdgeInsets.all(8.0),
                                      child: Text.rich(
                                        TextSpan(
                                          text: '${(currentIndex + 1)}. ',
                                          style: const TextStyle(
                                              color: Colors.grey),
                                          children: [
                                            TextSpan(
                                              text: mmemonic[currentIndex++],
                                              style: TextStyle(
                                                color: Theme.of(context)
                                                    .textTheme
                                                    .bodyLarge
                                                    .color,
                                              ),
                                            ),
                                          ],
                                        ),
                                      ),
                                    ),
                                  ),
                                ),
                                Expanded(
                                  child: Card(
                                    child: Padding(
                                      padding: const EdgeInsets.all(8.0),
                                      child: Text.rich(
                                        TextSpan(
                                          text: '${(currentIndex + 1)}. ',
                                          style: const TextStyle(
                                              color: Colors.grey),
                                          children: [
                                            TextSpan(
                                              text: mmemonic[currentIndex++],
                                              style: TextStyle(
                                                color: Theme.of(context)
                                                    .textTheme
                                                    .bodyLarge
                                                    .color,
                                              ),
                                            ),
                                          ],
                                        ),
                                      ),
                                    ),
                                  ),
                                ),
                                Expanded(
                                  child: Card(
                                    child: Padding(
                                      padding: const EdgeInsets.all(8.0),
                                      child: Text.rich(
                                        TextSpan(
                                          text: '${(currentIndex + 1)}. ',
                                          style: const TextStyle(
                                              color: Colors.grey),
                                          children: [
                                            TextSpan(
                                              text: mmemonic[currentIndex++],
                                              style: TextStyle(
                                                color: Theme.of(context)
                                                    .textTheme
                                                    .bodyLarge
                                                    .color,
                                              ),
                                            ),
                                          ],
                                        ),
                                      ),
                                    ),
                                  ),
                                ),
                              ],
                            ),
                            const SizedBox(
                              height: 15,
                            )
                          ],
                          GestureDetector(
                            onTap: () {
                              slideUpPanel(
                                context,
                                Column(
                                  children: [
                                    const SizedBox(
                                      height: 20,
                                    ),
                                    const Text(
                                      'Choose your number of words.',
                                      textAlign: TextAlign.center,
                                      style: TextStyle(fontSize: 20),
                                    ),
                                    const SizedBox(
                                      height: 20,
                                    ),
                                    for (int i = 0;
                                        i < bip39Strength.length;
                                        i++) ...[
                                      GestureDetector(
                                        onTap: () {
                                          setState(() {
                                            chooseLength = bip39Strength.values
                                                .toList()[i];
                                            data_ = bip39.generateMnemonic(
                                              strength: int.parse(
                                                bip39Strength.keys.toList()[i],
                                              ),
                                            );
                                            Navigator.pop(context);
                                          });
                                        },
                                        child: Card(
                                          child: Padding(
                                            padding: const EdgeInsets.all(15.0),
                                            child: Row(
                                              mainAxisSize: MainAxisSize.min,
                                              children: [
                                                Text(
                                                  '${bip39Strength.values.toList()[i]} words',
                                                  style: const TextStyle(
                                                      fontSize: 18),
                                                ),
                                                const SizedBox(
                                                  width: 10,
                                                ),
                                                if (bip39Strength.values
                                                        .toList()[i] ==
                                                    chooseLength.toString())
                                                  Container(
                                                    decoration:
                                                        const BoxDecoration(
                                                            shape:
                                                                BoxShape.circle,
                                                            color: Colors.blue),
                                                    child: const Padding(
                                                      padding:
                                                          EdgeInsets.all(2),
                                                      child: Icon(
                                                        Icons.check,
                                                        size: 20,
                                                        color: Colors.white,
                                                      ),
                                                    ),
                                                  )
                                              ],
                                            ),
                                          ),
                                        ),
                                      ),
                                    ]
                                  ],
                                ),
                              );
                            },
                            child: SizedBox(
                              width: MediaQuery.of(context).size.width * .5,
                              child: Card(
                                child: Padding(
                                  padding: const EdgeInsets.all(10.0),
                                  child: Text('$chooseLength words'),
                                ),
                              ),
                            ),
                          ),
                          GestureDetector(
                            onTap: () async {
                              // copy to clipboard
                              await Clipboard.setData(ClipboardData(
                                text: data_,
                              ));
                              Get.snackbar(
                                '',
                                AppLocalizations.of(context).copiedToClipboard,
                              );
                            },
                            child: Card(
                              shape: RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(10)),
                              color: colorForAddress,
                              child: Padding(
                                padding: const EdgeInsets.all(10),
                                child: Row(
                                  mainAxisSize: MainAxisSize.min,
                                  children: [
                                    Text(
                                      AppLocalizations.of(context).copy,
                                      textAlign: TextAlign.center,
                                      style: const TextStyle(
                                        fontSize: 14,
                                        color: Colors.black,
                                      ),
                                    ),
                                    const Icon(Icons.copy)
                                  ],
                                ),
                              ),
                            ),
                          ),
                          const SizedBox(
                            height: 15,
                          ),
                          Container(
                            decoration: BoxDecoration(
                                borderRadius:
                                    const BorderRadius.all(Radius.circular(10)),
                                color: Colors.red[100]),
                            child: Padding(
                              padding: const EdgeInsets.all(10),
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.center,
                                mainAxisAlignment: MainAxisAlignment.center,
                                children: [
                                  Text(
                                    AppLocalizations.of(context)
                                        .doNotShareYourmnemonic,
                                    style: const TextStyle(
                                        color: Colors.red,
                                        fontWeight: FontWeight.bold),
                                  ),
                                  Text(
                                      AppLocalizations.of(context)
                                          .ifSomeoneHasYourmnemonic,
                                      textAlign: TextAlign.center,
                                      style: const TextStyle(
                                        color: Colors.red,
                                      )),
                                ],
                              ),
                            ),
                          ),
                          const SizedBox(
                            height: 40,
                          ),
                          if (widget.verify == null) ...[
                            SizedBox(
                              width: double.infinity,
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
                                onPressed: () {
                                  Get.to(
                                    Confirmmnemonic(
                                      mmenomic: data_.split(' '),
                                    ),
                                  );
                                },
                                child: Padding(
                                  padding: const EdgeInsets.all(15),
                                  child: Text(
                                    AppLocalizations.of(context).continue_,
                                    style: const TextStyle(
                                      color: Colors.white,
                                      fontWeight: FontWeight.bold,
                                      fontSize: 16,
                                    ),
                                  ),
                                ),
                              ),
                            ),
                            const SizedBox(
                              height: 20,
                            ),
                            Container(
                              color: Colors.transparent,
                              width: double.infinity,
                              child: ElevatedButton(
                                style: ButtonStyle(
                                  backgroundColor:
                                      MaterialStateProperty.resolveWith(
                                    (states) => Colors.white,
                                  ),
                                  shape: MaterialStateProperty.resolveWith(
                                    (states) => RoundedRectangleBorder(
                                      borderRadius: BorderRadius.circular(10),
                                    ),
                                  ),
                                ),
                                onPressed: () async {
                                  if (successSaving) {
                                    Get.snackbar(
                                      '',
                                      AppLocalizations.of(context).alreadySaved,
                                      backgroundColor: Colors.green,
                                      colorText: Colors.white,
                                    );
                                    return;
                                  }
                                  SSS sss = SSS();
                                  List secretShares = sss.create(
                                    minShemirShare,
                                    minShemirShare,
                                    data_,
                                    isShemirBase64,
                                  );
                                  String filehash = sha3(data_);
                                  try {
                                    for (int i = 0;
                                        i < secretShares.length;
                                        i++) {
                                      final date = DateTime.now();
                                      final keyFile =
                                          'wootzapp_key_${i + 1}_${filehash}_$date.wz'
                                              .replaceAll(':', '')
                                              .split(' ')
                                              .join('_');

                                      await FileReader.writeDownloadFile(
                                        keyFile,
                                        secretShares[i],
                                      );
                                    }

                                    Get.snackbar(
                                      '',
                                      AppLocalizations.of(context)
                                          .savedToDownloadFolder,
                                      backgroundColor: Colors.green,
                                      colorText: Colors.white,
                                    );
                                    successSaving = true;
                                  } catch (e) {
                                    Get.snackbar(
                                      '',
                                      AppLocalizations.of(context)
                                          .errorTryAgain,
                                      backgroundColor: Colors.red,
                                      colorText: Colors.white,
                                    );
                                  }
                                },
                                child: Padding(
                                  padding: const EdgeInsets.all(15),
                                  child: Text(
                                    AppLocalizations.of(context)
                                        .convertToShemirSecret,
                                    style: const TextStyle(
                                      color: appBackgroundblue,
                                      fontWeight: FontWeight.bold,
                                      fontSize: 16,
                                    ),
                                  ),
                                ),
                              ),
                            ),
                          ]
                        ],
                      ),
                    ),
                  ),
                );
        },
      ),
    );
  }
}
