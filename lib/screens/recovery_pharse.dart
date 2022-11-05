import 'package:cryptowallet/screens/confirm_seed_phrase.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
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
  bool invisiblemnemonic = false;
  bool securitydialogOpen = false;
  final scaffoldKey = GlobalKey<ScaffoldState>();

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
  void dispose() {
    WidgetsBinding.instance?.removeObserver(this);
    enableScreenShot();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    List mmemonic = widget.data.split(' ');
    int currentIndex = 0;

    return Scaffold(
      appBar: AppBar(
        title: Text(AppLocalizations.of(context).yourSecretPhrase),
      ),
      key: scaffoldKey,
      body: securitydialogOpen
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
                                      style:
                                          const TextStyle(color: Colors.grey),
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
                                      style:
                                          const TextStyle(color: Colors.grey),
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
                                      style:
                                          const TextStyle(color: Colors.grey),
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
                      widget.verify != null
                          ? Container()
                          : SizedBox(
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
                                  Navigator.push(
                                    context,
                                    MaterialPageRoute(
                                      builder: (ctx) => Confirmmnemonic(
                                        mmenomic: widget.data.split(' '),
                                      ),
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
                    ],
                  ),
                ),
              ),
            ),
    );
  }
}
