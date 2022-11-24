import 'dart:io';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:image_picker/image_picker.dart';
import 'package:scan/scan.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:get/get.dart';

class QRScanView extends StatefulWidget {
  const QRScanView({Key key}) : super(key: key);

  @override
  _QRScanViewState createState() => _QRScanViewState();
}

class _QRScanViewState extends State<QRScanView> with WidgetsBindingObserver {
  final ScanController controller = ScanController();
  RxBool cameraOn = false.obs;
  File image;

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance?.addObserver(this);
  }

  @override
  void dispose() {
    WidgetsBinding.instance?.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    super.didChangeAppLifecycleState(state);
    switch (state) {
      case AppLifecycleState.resumed:
        controller.resume();
        break;

      case AppLifecycleState.paused:
        controller.pause();
        break;
      default:
        break;
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: Center(
          child: Stack(
            children: [
              ScanView(
                controller: controller,
                scanAreaScale: 1,
                scanLineColor: appBackgroundblue,
                onCapture: (data) {
                  Get.back(result: data);
                },
              ),
              Positioned(
                right: 0,
                child: IconButton(
                  onPressed: () {
                    controller.toggleTorchMode();

                    cameraOn.value = !cameraOn.value;
                  },
                  icon: Obx(
                    () => Icon(
                      FontAwesomeIcons.bolt,
                      color: cameraOn.value ? Colors.grey : Colors.white,
                      size: 35,
                    ),
                  ),
                ),
              ),
              Positioned(
                right: 0,
                bottom: 0,
                child: IconButton(
                  onPressed: () {
                    selectImage(
                        context: context,
                        onSelect: (XFile file) async {
                          final data = await Scan.parse(file.path);
                          if (data != null) {
                            Get.back(result: data);
                          } else {
                            showDialogWithMessage(
                              context: context,
                              message:
                                  AppLocalizations.of(context).errorTryAgain,
                            );
                          }
                        });
                  },
                  icon: const Icon(
                    Icons.image,
                    size: 35,
                    color: Colors.white,
                  ),
                ),
              )
            ],
          ),
        ),
      ),
    );
  }
}
