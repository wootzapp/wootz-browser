import 'dart:convert';
import 'dart:io';

import 'package:bs58check/bs58check.dart';
import 'package:cryptowallet/components/wallet_logo.dart';
import 'package:cryptowallet/main.dart';
import 'package:cryptowallet/utils/alt_ens.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:file_picker/file_picker.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:get/get.dart';
import 'package:hex/hex.dart';
import 'package:hive/hive.dart';
import 'package:bip39/bip39.dart' as bip39;
import 'package:ntcdcrypto/ntcdcrypto.dart';
import 'package:screenshot_callback/screenshot_callback.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:solana/base58.dart';
import '../components/loader.dart';
import '../utils/qr_scan_view.dart';

class ImportWithSecretShare extends StatefulWidget {
  final bool add;
  const ImportWithSecretShare({Key key, this.add}) : super(key: key);
  @override
  State<ImportWithSecretShare> createState() => _ImportWithSecretShareState();
}

class _ImportWithSecretShareState extends State<ImportWithSecretShare>
    with WidgetsBindingObserver {
  final secretSharesContrl = TextEditingController();
  final walletNameController = TextEditingController();
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  RxBool isLoading = false.obs;
  RxBool togglerFile = false.obs;

  // disallow screenshots
  ScreenshotCallback screenshotCallback = ScreenshotCallback();
  bool invisiblemnemonic = false;
  RxBool securitydialogOpen = false.obs;

  List<String> fileContent = [null, null, null];

  @override
  void initState() {
    super.initState();
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
  void didChangeAppLifecycleState(AppLifecycleState state) async {
    super.didChangeAppLifecycleState(state);
    switch (state) {
      case AppLifecycleState.resumed:
        if (invisiblemnemonic) {
          invisiblemnemonic = false;
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
          invisiblemnemonic = true;
          securitydialogOpen.value = true;
        }
        break;
      default:
        break;
    }
  }

  @override
  void dispose() {
    enableScreenShot();
    WidgetsBinding.instance?.removeObserver(this);
    secretSharesContrl.dispose();
    walletNameController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text(
            AppLocalizations.of(context).enterSecretShare,
          ),
          actions: [
            IconButton(
              onPressed: () async {
                String seedPhrase = await Get.to(
                  const QRScanView(),
                );
                if (seedPhrase == null) return;
                secretSharesContrl.text = seedPhrase;
              },
              icon: const Icon(
                Icons.qr_code_scanner,
              ),
            )
          ],
        ),
        key: _scaffoldKey,
        body: Obx(() {
          return securitydialogOpen.value
              ? Container()
              : SafeArea(
                  child: SingleChildScrollView(
                    child: Container(
                      color: Colors.transparent,
                      height: MediaQuery.of(context).size.height,
                      child: Padding(
                        padding: const EdgeInsets.all(25),
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.center,
                          crossAxisAlignment: CrossAxisAlignment.center,
                          children: [
                            const WalletLogo(),
                            const SizedBox(
                              height: 20,
                            ),
                            TextFormField(
                              controller: walletNameController,
                              keyboardType: TextInputType.visiblePassword,
                              decoration: InputDecoration(
                                hintText: AppLocalizations.of(context).name,
                                focusedBorder: const OutlineInputBorder(
                                    borderRadius:
                                        BorderRadius.all(Radius.circular(10.0)),
                                    borderSide: BorderSide.none),
                                border: const OutlineInputBorder(
                                    borderRadius:
                                        BorderRadius.all(Radius.circular(10.0)),
                                    borderSide: BorderSide.none),
                                enabledBorder: const OutlineInputBorder(
                                    borderRadius:
                                        BorderRadius.all(Radius.circular(10.0)),
                                    borderSide: BorderSide.none), // you
                                filled: true,
                              ),
                            ),
                            const SizedBox(
                              height: 20,
                            ),
                            Obx(
                              () => Row(
                                mainAxisAlignment:
                                    MainAxisAlignment.spaceBetween,
                                children: [
                                  Visibility(
                                    child: Text(togglerFile.value.toString()),
                                    visible: false,
                                  ),
                                  for (int i = 0; i < minShemirShare; i++)
                                    GestureDetector(
                                      onTap: () async {
                                        try {
                                          FilePickerResult result =
                                              await FilePicker.platform
                                                  .pickFiles();

                                          if (result != null) {
                                            File file =
                                                File(result.files.single.path);
                                            fileContent[i] = HEX.encode(
                                              base58.decode(
                                                await file.readAsString(),
                                              ),
                                            );
                                            togglerFile.value =
                                                !togglerFile.value;
                                          }
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
                                      child: SizedBox(
                                        child: Card(
                                          color: const Color(0xffF1F1F1),
                                          shape: RoundedRectangleBorder(
                                              borderRadius:
                                                  BorderRadius.circular(10)),
                                          child: Align(
                                            alignment: Alignment.center,
                                            child: Padding(
                                              padding: const EdgeInsets.all(10),
                                              child: Container(
                                                child: Column(
                                                  children: [
                                                    if (fileContent.isEmpty)
                                                      const Icon(
                                                        FontAwesomeIcons.file,
                                                        size: 40,
                                                        color:
                                                            appBackgroundblue,
                                                      )
                                                    else
                                                      Icon(
                                                        fileContent[i] == null
                                                            ? FontAwesomeIcons
                                                                .file
                                                            : FontAwesomeIcons
                                                                .fileAlt,
                                                        size: 40,
                                                        color:
                                                            appBackgroundblue,
                                                      ),
                                                    const SizedBox(
                                                      height: 10,
                                                    ),
                                                    Text(
                                                      'File ${i + 1}.wz',
                                                      style: const TextStyle(
                                                        color:
                                                            appBackgroundblue,
                                                        fontSize: 20,
                                                      ),
                                                    )
                                                  ],
                                                ),
                                              ),
                                            ),
                                          ),
                                        ),
                                      ),
                                    ),
                                ],
                              ),
                            ),
                            const SizedBox(
                              height: 20,
                            ),
                            Container(
                              decoration: BoxDecoration(
                                  borderRadius: const BorderRadius.all(
                                      Radius.circular(10)),
                                  color: Colors.red[100]),
                              child: Padding(
                                padding: const EdgeInsets.all(10),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.center,
                                  mainAxisAlignment: MainAxisAlignment.center,
                                  children: [
                                    Text(
                                      AppLocalizations.of(context)
                                          .doNotShareYourShemir,
                                      style: const TextStyle(
                                          color: Colors.red,
                                          fontWeight: FontWeight.bold),
                                    ),
                                    Text(
                                        AppLocalizations.of(context)
                                            .enterShemirSecretWithSpace,
                                        textAlign: TextAlign.center,
                                        style: const TextStyle(
                                          color: Colors.red,
                                        )),
                                  ],
                                ),
                              ),
                            ),
                            const SizedBox(
                              height: 20,
                            ),
                            Obx(() {
                              return SizedBox(
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
                                  onPressed: () async {
                                    FocusManager.instance.primaryFocus
                                        ?.unfocus();
                                    Get.closeAllSnackbars();

                                    try {
                                      SSS sss = SSS();

                                      final String mnemonics = sss.combine(
                                        fileContent,
                                        isShemirBase64,
                                      );
                                      if (mnemonics == '') {
                                        return;
                                      }
                                      if (isLoading.value) return;
                                      final pref = Hive.box(secureStorageKey);

                                      isLoading.value = true;
                                      final mnemonicsList =
                                          pref.get(mnemonicListKey);
                                      List decodedmnemonic = [];

                                      if (mnemonicsList != null) {
                                        decodedmnemonic =
                                            jsonDecode(mnemonicsList) as List;

                                        for (Map phrases in decodedmnemonic) {
                                          if (phrases['phrase'] == mnemonics) {
                                            Get.snackbar(
                                              '',
                                              AppLocalizations.of(context)
                                                  .mnemonicAlreadyImported,
                                              backgroundColor: Colors.red,
                                              colorText: Colors.white,
                                            );

                                            isLoading.value = false;

                                            return;
                                          }
                                        }
                                      }

                                      final mnemonicValid = await compute(
                                        bip39.validateMnemonic,
                                        mnemonics,
                                      );
                                      if (!mnemonicValid) {
                                        Get.snackbar(
                                          '',
                                          AppLocalizations.of(context)
                                              .invalidmnemonic,
                                          backgroundColor: Colors.red,
                                          colorText: Colors.white,
                                        );

                                        isLoading.value = false;

                                        return;
                                      }

                                      String walletName =
                                          walletNameController.text.trim();

                                      if (walletName.isEmpty) walletName = null;

                                      await initializeAllPrivateKeys(
                                        mnemonics,
                                      );

                                      decodedmnemonic.add({
                                        'phrase': mnemonics,
                                        'name': walletName,
                                      });

                                      await pref.put(
                                        mnemonicListKey,
                                        jsonEncode(decodedmnemonic),
                                      );
                                      await pref.put(
                                          currentMmenomicKey, mnemonics);

                                      await pref.put(
                                        currentUserWalletNameKey,
                                        walletName,
                                      );

                                      RestartWidget.restartApp(context);

                                      isLoading.value = false;
                                    } catch (e) {
                                      if (kDebugMode) {
                                        print(e);
                                      }
                                      Get.snackbar(
                                        '',
                                        AppLocalizations.of(context)
                                            .errorTryAgain,
                                        backgroundColor: Colors.red,
                                        colorText: Colors.white,
                                      );

                                      isLoading.value = false;
                                    }
                                  },
                                  child: Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: isLoading.value
                                        ? const Loader(
                                            color: Colors.white,
                                          )
                                        : Text(
                                            AppLocalizations.of(context)
                                                .confirm,
                                            style: const TextStyle(
                                              color: Colors.white,
                                              fontSize: 16,
                                              fontWeight: FontWeight.bold,
                                            ),
                                          ),
                                  ),
                                ),
                              );
                            })
                          ],
                        ),
                      ),
                    ),
                  ),
                );
        }));
  }
}
