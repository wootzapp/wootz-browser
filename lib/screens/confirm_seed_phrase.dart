// ignore_for_file: prefer__ructors, prefer_const_constructors

import 'dart:convert';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:screenshot_callback/screenshot_callback.dart';
import '../components/loader.dart';
import '../config/colors.dart';
import '../main.dart';
import '../utils/rpc_urls.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class Confirmmnemonic extends StatefulWidget {
  final List mmenomic;
  const Confirmmnemonic({
    Key key,
    this.mmenomic,
  }) : super(key: key);
  @override
  _ConfirmmnemonicState createState() => _ConfirmmnemonicState();
}

class _ConfirmmnemonicState extends State<Confirmmnemonic> {
  bool finished = false;
  bool firstStep = true;
  bool secondStep = false;
  bool thirdStep = false;
  bool fourthStep = false;
  final List numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12];
  int currentCorrectItem = 0;
  bool firstTime = true;
  List<String> mmenomicArray = [];
  List<String> mmenomicShuffled = [];
  bool isLoading = false;
  List<int> boxIndexGotten = [];
  // disallow screenshots
  ScreenshotCallback screenshotCallback = ScreenshotCallback();
  @override
  void initState() {
    super.initState();
    screenshotCallback.addListener(() {
      showDialogWithMessage(
        context: context,
        message: AppLocalizations.of(context).youCantScreenshot,
      );
    });
    disEnableScreenShot();
  }

  void verifySelection(int selectionIndex, List firstThree) {
    if (mmenomicShuffled[selectionIndex] ==
        mmenomicArray[firstThree[currentCorrectItem] - 1]) {
      setState(() {
        boxIndexGotten.add(selectionIndex);
        if (currentCorrectItem == 2) {
          if (firstStep == true) {
            firstStep = false;
            secondStep = true;
          } else if (firstStep == false && secondStep == true) {
            secondStep = false;
            thirdStep = true;
          } else if (secondStep == false && thirdStep == true) {
            thirdStep = false;
            fourthStep = true;
          } else if (thirdStep == false && fourthStep == true) {
            finished = true;
          }
          currentCorrectItem = 0;
        } else {
          currentCorrectItem++;
        }
      });
    } else {
      invalidSeedOrder();
    }
  }

  void invalidSeedOrder() {
    Get.closeAllSnackbars();
    Get.snackbar(
      '',
      AppLocalizations.of(context).invalidmnemonic,
      backgroundColor: Colors.red,
      colorText: Colors.white,
    );

    setState(() {
      finished = false;
      firstStep = true;
      secondStep = false;
      thirdStep = false;
      fourthStep = false;
      currentCorrectItem = 0;
      firstTime = true;
      mmenomicShuffled.length = 0;
      isLoading = false;
      boxIndexGotten.length = 0;
    });
  }

  @override
  Widget build(BuildContext context) {
    if (firstTime) {
      mmenomicArray = widget.mmenomic;
      mmenomicShuffled = [...mmenomicArray]..shuffle();
      firstTime = false;
    }
    List firstThree = [];
    if (firstStep) {
      firstThree = [numbers[0], numbers[1], numbers[2]];
    } else if (secondStep) {
      firstThree = [numbers[3], numbers[4], numbers[5]];
    } else if (thirdStep) {
      firstThree = [numbers[6], numbers[7], numbers[8]];
    } else if (fourthStep) {
      firstThree = [numbers[9], numbers[10], numbers[11]];
    }

    final scaffoldKey = GlobalKey<ScaffoldState>();

    List<Widget> seedPhraseWidget = [];
    final row = mmenomicShuffled.length ~/ 3;
    const column = 3;
    for (int index = 0; index < row; index++) {
      seedPhraseWidget.addAll(
        [
          SizedBox(
            height: 15,
          ),
          Row(
            children: [
              for (int inner = 0; inner < column; inner++)
                Expanded(
                  child: GestureDetector(
                    onTap: boxIndexGotten.contains(index * column + inner)
                        ? null
                        : () {
                            verifySelection(
                              index * column + inner,
                              firstThree,
                            );
                          },
                    child: Card(
                      color: boxIndexGotten.contains(
                        index * column + inner,
                      )
                          ? grey1
                          : null,
                      child: Padding(
                        padding: const EdgeInsets.all(10),
                        child: Text(
                          mmenomicShuffled[index * column + inner],
                          style: TextStyle(
                            color: boxIndexGotten.contains(
                              index * column + inner,
                            )
                                ? grey3
                                : null,
                          ),
                        ),
                      ),
                    ),
                  ),
                ),
            ],
          ),
        ],
      );
    }

    return Scaffold(
      appBar: AppBar(
        title: Text(
          AppLocalizations.of(context).confirmmnemonic,
        ),
      ),
      key: scaffoldKey,
      body: SafeArea(
        child: SingleChildScrollView(
          child: Padding(
            padding: EdgeInsets.fromLTRB(20, 0, 20, 0),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.start,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                SizedBox(
                  height: 20,
                ),
                if (!finished)
                  Card(
                    child: Padding(
                      padding: const EdgeInsets.all(10.0),
                      child: Column(
                        children: [
                          Text(
                            AppLocalizations.of(context)
                                .selectEachWordAsPresented,
                          ),
                          Row(
                            children: [
                              Expanded(
                                child: Card(
                                  color:
                                      currentCorrectItem >= 1 ? green1 : null,
                                  child: Padding(
                                    padding: const EdgeInsets.all(10),
                                    child: Text(
                                      currentCorrectItem >= 1
                                          ? mmenomicArray[firstThree[0] - 1]
                                          : firstThree[0].toString(),
                                      style: TextStyle(
                                        color: currentCorrectItem >= 1
                                            ? green5
                                            : null,
                                      ),
                                    ),
                                  ),
                                ),
                              ),
                              Expanded(
                                child: Card(
                                  color:
                                      currentCorrectItem == 2 ? green1 : null,
                                  child: Padding(
                                    padding: const EdgeInsets.all(10),
                                    child: Text(
                                      currentCorrectItem == 2
                                          ? mmenomicArray[firstThree[1] - 1]
                                              .toString()
                                          : firstThree[1].toString(),
                                      style: TextStyle(
                                        color: currentCorrectItem == 2
                                            ? green5
                                            : null,
                                      ),
                                    ),
                                  ),
                                ),
                              ),
                              Expanded(
                                child: Card(
                                  child: Padding(
                                    padding: const EdgeInsets.all(10),
                                    child: Text(
                                      firstThree[2].toString(),
                                      style: TextStyle(color: null),
                                    ),
                                  ),
                                ),
                              ),
                            ],
                          ),
                        ],
                      ),
                    ),
                  ),
                Container(
                  color: Colors.transparent,
                  width: double.infinity,
                  child: Column(
                    children: seedPhraseWidget,
                  ),
                ),
                const SizedBox(
                  height: 40,
                ),
                SizedBox(
                  width: double.infinity,
                  child: ElevatedButton(
                    style: ButtonStyle(
                      backgroundColor: MaterialStateProperty.resolveWith(
                        (states) => appBackgroundblue,
                      ),
                      shape: MaterialStateProperty.resolveWith(
                        (states) => RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(10),
                        ),
                      ),
                    ),
                    onPressed: finished
                        ? () async {
                            Get.closeAllSnackbars();
                            if (isLoading) return;
                            setState(() {
                              isLoading = true;
                            });
                            final pref = Hive.box(secureStorageKey);
                            String mnemonics = widget.mmenomic.join(' ');
                            try {
                              final mnemonicsList = pref.get(mnemonicListKey);
                              List decodedmnemonic = [];

                              if (mnemonicsList != null) {
                                decodedmnemonic =
                                    jsonDecode(mnemonicsList) as List;

                                for (Map phrases in decodedmnemonic) {
                                  if (phrases['phrase'] == mnemonics) {
                                    return;
                                  }
                                }
                              }
                              await initializeAllPrivateKeys(mnemonics);

                              decodedmnemonic.add({'phrase': mnemonics});
                              await pref.put(
                                mnemonicListKey,
                                jsonEncode(decodedmnemonic),
                              );

                              await pref.put(currentMmenomicKey, mnemonics);

                              await pref.put(
                                currentUserWalletNameKey,
                                null,
                              );

                              RestartWidget.restartApp(context);
                            } catch (e) {
                              if (kDebugMode) {
                                print(e);
                              }
                              Get.snackbar(
                                '',
                                AppLocalizations.of(context).errorTryAgain,
                                backgroundColor: Colors.red,
                                colorText: Colors.white,
                              );

                              setState(() {
                                isLoading = false;
                              });
                            }
                          }
                        : null,
                    child: Padding(
                      padding: const EdgeInsets.all(15),
                      child: isLoading
                          ? Loader(color: white)
                          : Text(
                              AppLocalizations.of(context).continue_,
                              style: TextStyle(
                                fontWeight: FontWeight.bold,
                                color: Colors.white,
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
    );
  }
}
