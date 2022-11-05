import 'dart:convert';

import 'package:cryptowallet/components/wallet_logo.dart';
import 'package:cryptowallet/main.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:hive/hive.dart';
import 'package:bip39/bip39.dart' as bip39;
import 'package:screenshot_callback/screenshot_callback.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import '../components/loader.dart';
import '../utils/qr_scan_view.dart';

class EnterPhrase extends StatefulWidget {
  final bool add;
  const EnterPhrase({Key key, this.add}) : super(key: key);
  @override
  State<EnterPhrase> createState() => _EnterPhraseState();
}

class _EnterPhraseState extends State<EnterPhrase> with WidgetsBindingObserver {
  final mnemonicController = TextEditingController();
  final walletNameController = TextEditingController();
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  bool isLoading = false;

  // disallow screenshots
  ScreenshotCallback screenshotCallback = ScreenshotCallback();
  bool invisiblemnemonic = false;
  bool securitydialogOpen = false;

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
            setState(() {
              securitydialogOpen = false;
            });
          } else {
            SystemNavigator.pop();
          }
        }
        break;
      case AppLifecycleState.paused:
        if (!securitydialogOpen) {
          setState(() {
            invisiblemnemonic = true;
            securitydialogOpen = true;
          });
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
    mnemonicController.dispose();
    walletNameController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
          AppLocalizations.of(context).entermnemonic,
        ),
        actions: [
          IconButton(
            onPressed: () async {
              String seedPhrase = await Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (ctx) => const QRScanView(),
                ),
              );
              if (seedPhrase == null) return;
              mnemonicController.text = seedPhrase;
            },
            icon: const Icon(
              Icons.qr_code_scanner,
            ),
          )
        ],
      ),
      key: _scaffoldKey,
      body: securitydialogOpen
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
                        Stack(
                          children: [
                            TextFormField(
                              maxLines: 3,
                              controller: mnemonicController,
                              keyboardType: TextInputType.visiblePassword,
                              decoration: InputDecoration(
                                contentPadding: const EdgeInsets.only(
                                  top: 100,
                                  left: 12,
                                  right: 12,
                                ),
                                hintText:
                                    AppLocalizations.of(context).entermnemonic,
                                focusedBorder: const OutlineInputBorder(
                                  borderRadius:
                                      BorderRadius.all(Radius.circular(10.0)),
                                  borderSide: BorderSide.none,
                                ),
                                border: const OutlineInputBorder(
                                    borderRadius:
                                        BorderRadius.all(Radius.circular(10.0)),
                                    borderSide: BorderSide.none),
                                enabledBorder: const OutlineInputBorder(
                                  borderRadius:
                                      BorderRadius.all(Radius.circular(10.0)),
                                  borderSide: BorderSide.none,
                                ), // you
                                filled: true,
                              ),
                            ),
                            Positioned(
                              right: 10,
                              top: 10,
                              child: InkWell(
                                onTap: () async {
                                  ClipboardData cdata = await Clipboard.getData(
                                      Clipboard.kTextPlain);
                                  if (cdata == null) return;
                                  if (cdata.text == null) return;
                                  mnemonicController.text = cdata.text;
                                },
                                child: Container(
                                  decoration: BoxDecoration(
                                    color: Theme.of(context)
                                        .scaffoldBackgroundColor,
                                    borderRadius: BorderRadius.circular(10),
                                  ),
                                  child: Padding(
                                    padding: const EdgeInsets.all(8.0),
                                    child: Text(
                                      AppLocalizations.of(context).paste,
                                    ),
                                  ),
                                ),
                              ),
                            ),
                          ],
                        ),
                        const SizedBox(
                          height: 20,
                        ),
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
                            onPressed: () async {
                              FocusManager.instance.primaryFocus?.unfocus();
                              ScaffoldMessenger.of(context)
                                  .hideCurrentSnackBar();

                              final String mnemonics =
                                  mnemonicController.text.trim();
                              if (mnemonics == '') {
                                return;
                              }
                              if (isLoading) return;
                              final pref = Hive.box(secureStorageKey);
                              setState(() {
                                isLoading = true;
                              });
                              try {
                                final mnemonicsList = pref.get(mnemonicListKey);
                                List decodedmnemonic = [];

                                if (mnemonicsList != null) {
                                  decodedmnemonic =
                                      jsonDecode(mnemonicsList) as List;

                                  for (Map phrases in decodedmnemonic) {
                                    if (phrases['phrase'] == mnemonics) {
                                      ScaffoldMessenger.of(context)
                                          .showSnackBar(
                                        SnackBar(
                                          backgroundColor: Colors.red,
                                          content: Text(
                                            AppLocalizations.of(context)
                                                .mnemonicAlreadyImported,
                                            style: const TextStyle(
                                                color: Colors.white),
                                          ),
                                        ),
                                      );
                                      setState(() {
                                        isLoading = false;
                                      });
                                      return;
                                    }
                                  }
                                }

                                final mnemonicValid = await compute(
                                  bip39.validateMnemonic,
                                  mnemonics,
                                );
                                if (!mnemonicValid) {
                                  ScaffoldMessenger.of(context).showSnackBar(
                                    SnackBar(
                                      backgroundColor: Colors.red,
                                      content: Text(
                                        AppLocalizations.of(context)
                                            .invalidmnemonic,
                                        style: const TextStyle(
                                            color: Colors.white),
                                      ),
                                    ),
                                  );
                                  setState(() {
                                    isLoading = false;
                                  });
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
                                await pref.put(currentMmenomicKey, mnemonics);

                                await pref.put(
                                  currentUserWalletNameKey,
                                  walletName,
                                );

                                RestartWidget.restartApp(context);
                                setState(() {
                                  isLoading = false;
                                });
                              } catch (e) {
                                if (kDebugMode) {
                                  print(e);
                                }
                                ScaffoldMessenger.of(context).showSnackBar(
                                  SnackBar(
                                    backgroundColor: Colors.red,
                                    content: Text(
                                      AppLocalizations.of(context)
                                          .errorTryAgain,
                                      style:
                                          const TextStyle(color: Colors.white),
                                    ),
                                  ),
                                );

                                setState(() {
                                  isLoading = false;
                                });
                              }
                            },
                            child: Padding(
                              padding: const EdgeInsets.all(15),
                              child: isLoading
                                  ? const Loader(
                                      color: Colors.white,
                                    )
                                  : Text(
                                      AppLocalizations.of(context).confirm,
                                      style: const TextStyle(
                                        color: Colors.white,
                                        fontSize: 16,
                                        fontWeight: FontWeight.bold,
                                      ),
                                    ),
                            ),
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
              ),
            ),
    );
  }
}
