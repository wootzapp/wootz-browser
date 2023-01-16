// ignore_for_file: prefer__ructors, prefer_const_constructors

import 'dart:convert';

import 'package:cryptowallet/screens/webview_tab.dart';
import 'package:cryptowallet/utils/alt_ens.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:multi_value_listenable_builder/multi_value_listenable_builder.dart';
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
  // RxBool finished = false.obs;
  final finished = ValueNotifier<bool>(false);
  // RxBool firstStep = true.obs;
  final firstStep = ValueNotifier<bool>(true);

  // RxBool secondStep = false.obs;
  final secondStep = ValueNotifier<bool>(false);

  // RxBool thirdStep = false.obs;
  final thirdStep = ValueNotifier<bool>(false);

  // RxBool fourthStep = false.obs;
  final fourthStep = ValueNotifier<bool>(false);

  List numbers;

  int currentCorrectItem = 0;
  // RxBool firstTime = true.obs;
  final firstTime = ValueNotifier<bool>(true);

  List<String> mmenomicArray = [];
  // RxList<dynamic> mmenomicShuffled = [].obs;
  final mmenomicShuffled = ValueNotifier<List<dynamic>>([]);
  // RxBool isLoading = false.obs;
  final isLoading = ValueNotifier<bool>(false);

  // RxList<dynamic> boxIndexGotten = [].obs;
  final boxIndexGotten = ValueNotifier<List<dynamic>>([]);
  ScreenshotCallback screenshotCallback = ScreenshotCallback();
  @override
  void initState() {
    super.initState();
    numbers = List<int>.generate(
      widget.mmenomic.length,
      (int index) => index + 1,
    );
    screenshotCallback.addListener(() {
      showDialogWithMessage(
        context: context,
        message: AppLocalizations.of(context).youCantScreenshot,
      );
    });
    disEnableScreenShot();
  }

  void verifySelection(int selectionIndex, List firstThree) {
    if (mmenomicShuffled.value[selectionIndex] ==
        mmenomicArray[firstThree[currentCorrectItem] - 1]) {
      final currentList = boxIndexGotten.value;
      currentList.add(selectionIndex);
      boxIndexGotten.value = currentList;
      if (currentCorrectItem == 2) {
        if (firstStep.value == true) {
          firstStep.value = false;
          secondStep.value = true;
        } else if (firstStep.value == false && secondStep.value == true) {
          secondStep.value = false;
          thirdStep.value = true;
        } else if (secondStep.value == false && thirdStep.value == true) {
          thirdStep.value = false;
          fourthStep.value = true;
        } else if (thirdStep.value == false && fourthStep.value == true) {
          finished.value = true;
          finishConfirm();
        }
        setState(() {
          currentCorrectItem = 0;
        });
      } else {
        setState(() {
          currentCorrectItem++;
        });
      }
    } else {
      invalidSeedOrder();
    }
  }

  Future finishConfirm() async {
    // Get.closeAllSnackbars();
    ScaffoldMessenger.of(context).removeCurrentSnackBar();
    if (isLoading.value) return;

    isLoading.value = true;

    final pref = Hive.box(secureStorageKey);
    String mnemonics = widget.mmenomic.join(' ');
    try {
      final mnemonicsList = pref.get(mnemonicListKey);
      List decodedmnemonic = [];

      if (mnemonicsList != null) {
        decodedmnemonic = jsonDecode(mnemonicsList) as List;

        for (Map phrases in decodedmnemonic) {
          if (phrases['phrase'] == mnemonics) {
            // Get.snackbar(
            //   '',
            //   AppLocalizations.of(context).mnemonicAlreadyImported,
            //   backgroundColor: Colors.red,
            //   colorText: Colors.white,
            // );
            ScaffoldMessenger.of(context).showSnackBar(
              SnackBar(
                content: Text(
                  AppLocalizations.of(context).mnemonicAlreadyImported,
                ),
                backgroundColor: Colors.red,
                action: SnackBarAction(
                  label: 'OK',
                  onPressed: () {},
                  textColor: Colors.white,
                ),
              ),
            );
            isLoading.value = false;
            return;
          }
        }
      }

      await initializeAllPrivateKeys(mnemonics);

      decodedmnemonic.add({
        'phrase': mnemonics,
      });
      await pref.put(
        mnemonicListKey,
        jsonEncode(decodedmnemonic),
      );

      await pref.put(currentMmenomicKey, mnemonics);

      await pref.put(
        currentUserWalletNameKey,
        null,
      );
      isLoading.value = false;

      RestartWidget.restartApp(context);
      final redirectUrl = pref.get('redirectUrl');

      if (redirectUrl != null) {
        // Navigator.of(context).pushReplacement(
        //     MaterialPageRoute(builder: (_) => WebViewTab(url: redirectUrl)));
        // Widget nextWidget = await dappWidget(context, redirectUrl);
        Widget nextWidget = await dappWidget(context, redirectUrl);

        Navigator.of(context)
            .pushReplacement(MaterialPageRoute(builder: (_) => nextWidget));
      } else {
        for (int i = 0; i < 2; i++) {
          Navigator.pop(context);
        }
      }

      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(
            AppLocalizations.of(context).walletCreated,
          ),
        ),
      );

      // Get.snackbar(
      //   '',
      //   AppLocalizations.of(context).walletCreated,
      // );

    } catch (e) {
      if (kDebugMode) {
        print(e);
      }
      // Get.snackbar(
      //   '',
      //   AppLocalizations.of(context).errorTryAgain,
      //   backgroundColor: Colors.red,
      //   colorText: Colors.white,
      // );
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(
            AppLocalizations.of(context).errorTryAgain,
          ),
          backgroundColor: Colors.red,
          action: SnackBarAction(
            label: 'OK',
            onPressed: () {},
            textColor: Colors.white,
          ),
        ),
      );

      isLoading.value = false;
    }
  }

  void invalidSeedOrder() {
    // Get.closeAllSnackbars();
    ScaffoldMessenger.of(context).removeCurrentSnackBar();
    // Get.snackbar(
    //   '',
    //   AppLocalizations.of(context).invalidmnemonic,
    //   backgroundColor: Colors.red,
    //   colorText: Colors.white,
    // );
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(
          AppLocalizations.of(context).invalidmnemonic,
        ),
        backgroundColor: Colors.red,
        action: SnackBarAction(
          label: 'OK',
          onPressed: () {},
          textColor: Colors.white,
        ),
      ),
    );

    finished.value = false;
    firstStep.value = true;
    secondStep.value = false;
    thirdStep.value = false;
    fourthStep.value = false;
    currentCorrectItem = 0;
    firstTime.value = true;
    mmenomicShuffled.value = [];
    isLoading.value = false;
    boxIndexGotten.value = [];
  }

  final scaffoldKey = GlobalKey<ScaffoldState>();

  @override
  Widget build(BuildContext context) {
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
            child: MultiValueListenableBuilder(
              valueListenables: [
                finished,
                firstStep,
                secondStep,
                thirdStep,
                fourthStep,
                firstTime,
                mmenomicShuffled,
                boxIndexGotten,
                isLoading,
              ],
              builder: (context, value, child) {
                if (firstTime.value) {
                  mmenomicArray = widget.mmenomic;
                  mmenomicShuffled.value = [...mmenomicArray]..shuffle();
                  firstTime.value = false;
                }
                List firstThree = [];
                if (firstStep.value) {
                  firstThree = [numbers[0], numbers[1], numbers[2]];
                } else if (secondStep.value) {
                  firstThree = [numbers[3], numbers[4], numbers[5]];
                } else if (thirdStep.value) {
                  firstThree = [numbers[6], numbers[7], numbers[8]];
                } else if (fourthStep.value) {
                  firstThree = [numbers[9], numbers[10], numbers[11]];
                }

                List<Widget> seedPhraseWidget = [];
                final row = mmenomicShuffled.value.length ~/ 3;
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
                                onTap: boxIndexGotten.value
                                        .contains(index * column + inner)
                                    ? null
                                    : () {
                                        verifySelection(
                                          index * column + inner,
                                          firstThree,
                                        );
                                      },
                                child: Card(
                                  color: boxIndexGotten.value.contains(
                                    index * column + inner,
                                  )
                                      ? grey1
                                      : null,
                                  child: Padding(
                                    padding: const EdgeInsets.all(10),
                                    child: Text(
                                      mmenomicShuffled
                                          .value[index * column + inner],
                                      style: TextStyle(
                                        color: boxIndexGotten.value.contains(
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
                return Column(
                  mainAxisAlignment: MainAxisAlignment.start,
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    SizedBox(
                      height: 20,
                    ),
                    if (!finished.value)
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
                                      color: currentCorrectItem >= 1
                                          ? green1
                                          : null,
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
                                      color: currentCorrectItem == 2
                                          ? green1
                                          : null,
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
                    if (finished.value)
                      ValueListenableBuilder(
                          valueListenable: isLoading,
                          builder: (context, value, child) {
                            return SizedBox(
                              width: double.infinity,
                              child: ElevatedButton(
                                style: ButtonStyle(
                                  backgroundColor:
                                      MaterialStateProperty.resolveWith(
                                    (states) => appBackgroundblue,
                                  ),
                                  shape: MaterialStateProperty.resolveWith(
                                    (states) => RoundedRectangleBorder(
                                      borderRadius: BorderRadius.circular(10),
                                    ),
                                  ),
                                ),
                                onPressed: null,
                                child: Padding(
                                  padding: const EdgeInsets.all(15),
                                  child: isLoading.value
                                      ? Loader(color: white)
                                      : Text(
                                          AppLocalizations.of(context)
                                              .continue_,
                                          style: TextStyle(
                                            fontWeight: FontWeight.bold,
                                            color: Colors.white,
                                          ),
                                        ),
                                ),
                              ),
                            );
                          }),
                  ],
                );
              },
            ),
          ),
        ),
      ),
    );
  }
}
