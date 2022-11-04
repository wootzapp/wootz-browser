import 'package:cryptowallet/main.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/material.dart';
import 'package:flutter_countdown_timer/countdown_timer_controller.dart';
import 'package:flutter_countdown_timer/flutter_countdown_timer.dart';
import 'package:flutter_svg/svg.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:hive/hive.dart';

class OpenAppPinFailed extends StatefulWidget {
  final int remainSec;
  const OpenAppPinFailed({Key key, this.remainSec}) : super(key: key);

  @override
  State<OpenAppPinFailed> createState() => _OpenAppPinFailedState();
}

class _OpenAppPinFailedState extends State<OpenAppPinFailed> {
  CountdownTimerController controller;
  int endTime;
  int minutesForUnlock = 1;

  @override
  void initState() {
    super.initState();
    endTime = DateTime.now().millisecondsSinceEpoch +
        1000 * (widget.remainSec ?? 60) * minutesForUnlock;
    controller = CountdownTimerController(endTime: endTime, onEnd: onEnd);
    controller.addListener(() async {
      final pref = Hive.box(secureStorageKey);
      final remaininTime = controller.currentRemainingTime;
      if (remaininTime == null) return;
      await pref.put(appUnlockTime, remaininTime.sec);
    });
  }

  bool hideHeader = false;
  void onEnd() async {
    setState(() {
      hideHeader = true;
    });
    await Navigator.pushReplacement(
      context,
      MaterialPageRoute(builder: (ctx) => const MyHomePage()),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              SvgPicture.asset(
                'assets/security_new.svg',
                width: 100,
              ),
              const SizedBox(
                height: 30,
              ),
              if (!hideHeader)
                Text(
                  AppLocalizations.of(context).lockedAppMessage,
                  style: const TextStyle(fontSize: 18),
                ),
              const SizedBox(
                height: 10,
              ),
              CountdownTimer(
                endWidget: Text(
                  AppLocalizations.of(context).expiredTimeMessage,
                  style: const TextStyle(fontSize: 18),
                ),
                controller: controller,
                onEnd: onEnd,
                textStyle:
                    const TextStyle(fontWeight: FontWeight.bold, fontSize: 18),
                endTime: endTime,
              ),
            ],
          ),
        ),
      ),
    );
  }
}
