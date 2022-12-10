import 'package:cryptowallet/utils/qr_scan_view.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:get/get.dart';
import 'package:hive/hive.dart';

import '../components/loader.dart';
import '../screens/main_screen.dart';
import '../screens/security.dart';
import '../screens/send_token.dart';
import '../screens/wallet_main_body.dart';
import 'app_config.dart';

class WalletBlack extends StatefulWidget {
  const WalletBlack({Key key}) : super(key: key);

  @override
  State<WalletBlack> createState() => _WalletBlackState();
}

class _WalletBlackState extends State<WalletBlack> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: SingleChildScrollView(
          child: Column(
            children: [
              Padding(
                padding: const EdgeInsets.all(15),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Card(
                      child: Padding(
                        padding: const EdgeInsets.all(10.0),
                        child: Column(children: [
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              const Text('Wallets'),
                              GestureDetector(
                                onTap: () async {
                                  final pref = Hive.box(secureStorageKey);
                                  bool hasWallet =
                                      pref.get(currentMmenomicKey) != null;

                                  bool hasPasscode =
                                      pref.get(userUnlockPasscodeKey) != null;
                                  Widget dappWidget;
                                  Get.back();

                                  if (hasWallet) {
                                    dappWidget = const WalletMainBody();
                                  } else if (hasPasscode) {
                                    dappWidget = const MainScreen();
                                  } else {
                                    dappWidget = const Security();
                                  }
                                  await Get.to(dappWidget);
                                },
                                child: Row(
                                  children: const [
                                    Text('See all'),
                                    Icon(Icons.arrow_forward),
                                  ],
                                ),
                              ),
                            ],
                          ),
                          const Divider(
                            color: Colors.grey,
                          ),
                          GestureDetector(
                            onTap: () async {
                              final pref = Hive.box(secureStorageKey);

                              bool hasPasscode =
                                  pref.get(userUnlockPasscodeKey) != null;
                              Widget dappWidget;
                              Get.back();

                              if (hasPasscode) {
                                dappWidget = const MainScreen();
                              } else {
                                dappWidget = const Security();
                              }
                              await Get.to(dappWidget);
                            },
                            child: Row(
                              mainAxisAlignment: MainAxisAlignment.center,
                              children: const [
                                Icon(FontAwesomeIcons.wallet),
                                SizedBox(
                                  width: 5,
                                ),
                                Text('Create new wallet'),
                              ],
                            ),
                          )
                        ]),
                      ),
                    ),
                    SizedBox(
                      height: MediaQuery.of(context).size.height * .7,
                    ),
                    Card(
                      child: Padding(
                        padding: const EdgeInsets.all(10.0),
                        child: Column(children: [
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: const [
                              Text('Send'),
                              Icon(
                                Icons.edit_outlined,
                              ),
                            ],
                          ),
                          const Divider(),
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              const Icon(Icons.currency_bitcoin_sharp),
                              SizedBox(
                                width: 200,
                                height: 40,
                                child: TextFormField(
                                  autocorrect: false,
                                  keyboardType: TextInputType.visiblePassword,

                                  // controller: ,
                                  decoration: const InputDecoration(
                                    hintText: '',

                                    focusedBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    border: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    enabledBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none), // you
                                    filled: true,
                                  ),
                                ),
                              ),
                              const Icon(
                                FontAwesomeIcons.link,
                                size: 20,
                              ),
                              const Icon(
                                FontAwesomeIcons.qrcode,
                                size: 20,
                              ),
                            ],
                          ),
                          const SizedBox(
                            height: 15,
                          ),
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              const Icon(
                                FontAwesomeIcons.bitcoin,
                                size: 30,
                              ),
                              SizedBox(
                                width: 100,
                                height: 40,
                                child: TextFormField(
                                  autocorrect: false,
                                  keyboardType: TextInputType.visiblePassword,
                                  decoration: const InputDecoration(
                                    hintText: '',

                                    focusedBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    border: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    enabledBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none), // you
                                    filled: true,
                                  ),
                                ),
                              ),
                              const Icon(
                                FontAwesomeIcons.dollarSign,
                                size: 30,
                              ),
                              SizedBox(
                                width: 100,
                                height: 40,
                                child: TextFormField(
                                  autocorrect: false,
                                  keyboardType: TextInputType.visiblePassword,

                                  // controller: ,
                                  decoration: const InputDecoration(
                                    hintText: '',

                                    focusedBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    border: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    enabledBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none), // you
                                    filled: true,
                                  ),
                                ),
                              ),
                            ],
                          ),
                          const SizedBox(
                            height: 15,
                          ),
                          SizedBox(
                            width: double.infinity,
                            child: ElevatedButton(
                              style: ButtonStyle(
                                backgroundColor:
                                    MaterialStateProperty.resolveWith(
                                        (states) => Colors.grey),
                                shape: MaterialStateProperty.resolveWith(
                                  (states) => RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(10),
                                  ),
                                ),
                                textStyle: MaterialStateProperty.resolveWith(
                                  (states) =>
                                      const TextStyle(color: Colors.white),
                                ),
                              ),
                              onPressed: () {},
                              child: const Text('Send'),
                            ),
                          )
                        ]),
                      ),
                    ),
                    const SizedBox(
                      height: 15,
                    ),
                    const Text('Explore sites'),
                    const SizedBox(
                      height: 15,
                    ),
                    Row(
                      children: const [
                        Expanded(
                          child: Card(
                            child: Padding(
                              padding: EdgeInsets.all(20.0),
                              child: Text('Currence'),
                            ),
                          ),
                        ),
                        Expanded(
                          child: Card(
                            child: Padding(
                              padding: EdgeInsets.all(20.0),
                              child: Text('Exchange rate'),
                            ),
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(
                      height: 10,
                    ),
                  ],
                ),
              ),
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(10),
                  child: Column(children: [
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        const Text('Transactions history'),
                        Row(
                          children: const [
                            Text('See all'),
                            Icon(Icons.arrow_forward),
                          ],
                        )
                      ],
                    )
                  ]),
                ),
              ),
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(10),
                  child: Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        GestureDetector(
                          onTap: () async {
                            String data = await Get.to(
                              const QRScanView(),
                            );
                            if (data == null) {
                              Get.snackbar(
                                '',
                                eIP681ProcessingErrorMsg,
                                colorText: Colors.white,
                                backgroundColor: Colors.red,
                              );

                              return;
                            }
                            showDialog(
                                barrierDismissible: false,
                                context: context,
                                builder: (context) {
                                  return AlertDialog(
                                      backgroundColor: Colors.white,
                                      content: Column(
                                        mainAxisAlignment:
                                            MainAxisAlignment.center,
                                        mainAxisSize: MainAxisSize.min,
                                        children: const [
                                          SizedBox(
                                            width: 35,
                                            height: 35,
                                            child: Loader(),
                                          ),
                                        ],
                                      ));
                                });

                            Map scannedData = await processEIP681(data);

                            if (Navigator.canPop(context)) {
                              Get.back();
                            }

                            if (scannedData['success']) {
                              await Get.to(
                                SendToken(
                                  data: scannedData['msg'],
                                ),
                              );
                              return;
                            }
                            Get.snackbar(
                              '',
                              scannedData['msg'],
                              colorText: Colors.white,
                              backgroundColor: Colors.red,
                            );
                          },
                          child: SizedBox(
                            width: 100,
                            child: Column(
                              children: const [
                                Icon(Icons.qr_code),
                                Text(
                                  'Scan',
                                  style: TextStyle(fontSize: 12),
                                ),
                              ],
                            ),
                          ),
                        ),
                        SizedBox(
                          width: 100,
                          child: Column(
                            children: const [
                              Icon(Icons.search),
                              Text(
                                'Search Engine',
                                style: TextStyle(fontSize: 12),
                              ),
                            ],
                          ),
                        ),
                        GestureDetector(
                          onTap: () async {
                            final pref = Hive.box(secureStorageKey);

                            bool hasPasscode =
                                pref.get(userUnlockPasscodeKey) != null;
                            Widget dappWidget;
                            Get.back();

                            if (hasPasscode) {
                              dappWidget = const MainScreen();
                            } else {
                              dappWidget = const Security();
                            }
                            await Get.to(dappWidget);
                          },
                          child: SizedBox(
                            width: 100,
                            child: Column(
                              children: const [
                                Icon(FontAwesomeIcons.wallet),
                                Text(
                                  'Create new wallet',
                                  style: TextStyle(fontSize: 12),
                                  textAlign: TextAlign.center,
                                ),
                              ],
                            ),
                          ),
                        ),
                      ]),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
