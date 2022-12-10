import 'dart:convert';

import 'package:bitcoin_flutter/bitcoin_flutter.dart';
import 'package:cryptowallet/utils/qr_scan_view.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:decimal/decimal.dart';
import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:get/get.dart';
import 'package:hive/hive.dart';

import '../components/loader.dart';
import '../screens/main_screen.dart';
import '../screens/security.dart';
import '../screens/send_token.dart';
import '../screens/transfer_token.dart';
import '../screens/view_wallets.dart';
import '../screens/wallet_main_body.dart';
import 'app_config.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class WalletBlack extends StatefulWidget {
  const WalletBlack({Key key}) : super(key: key);

  @override
  State<WalletBlack> createState() => _WalletBlackState();
}

class _WalletBlackState extends State<WalletBlack> {
  final recipientAddrContr = TextEditingController();
  final amount = TextEditingController();
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
                                  final mnemonics = pref.get(mnemonicListKey);

                                  final currentPhrase =
                                      pref.get(currentMmenomicKey);

                                  await Get.to(
                                    ViewWallets(
                                      data: (jsonDecode(mnemonics) as List),
                                      currentPhrase: currentPhrase,
                                    ),
                                  );
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
                              const SizedBox(
                                width: 10,
                              ),
                              Flexible(
                                child: TextFormField(
                                  keyboardType: TextInputType.visiblePassword,
                                  autocorrect: false,
                                  controller: recipientAddrContr,
                                  decoration: const InputDecoration(
                                    hintText: 'Bitcoin Address',
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
                              const SizedBox(
                                width: 10,
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
                              const SizedBox(
                                width: 10,
                              ),
                              Flexible(
                                child: TextFormField(
                                  autocorrect: false,
                                  controller: amount,
                                  keyboardType:
                                      const TextInputType.numberWithOptions(
                                    decimal: true,
                                  ),
                                  decoration: const InputDecoration(
                                    hintText: 'Amount',
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
                              const SizedBox(
                                width: 10,
                              ),
                              const Icon(
                                FontAwesomeIcons.dollarSign,
                                size: 30,
                              ),
                              const SizedBox(
                                width: 10,
                              ),
                              Flexible(
                                child: TextFormField(
                                  autocorrect: false,
                                  keyboardType: TextInputType.visiblePassword,
                                  decoration: const InputDecoration(
                                    hintText: 'USD',
                                    enabled: false,
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
                              onPressed: () async {
                                String recipient = recipientAddrContr.text;
                                Map bitcoinDetails =
                                    getBitCoinPOSBlockchains()['Bitcoin'];
                                if (!Address.validateAddress(
                                  recipientAddrContr.text,
                                  bitcoinDetails['POSNetwork'],
                                )) {
                                  Get.snackbar(
                                    '',
                                    AppLocalizations.of(context).invalidAddress,
                                    colorText: Colors.white,
                                    backgroundColor: Colors.red,
                                  );

                                  return;
                                }

                                final data = {
                                  ...bitcoinDetails,
                                  'name': 'Bitcoin',
                                  'amount':
                                      Decimal.parse(amount.text).toString(),
                                  'recipient': recipient
                                };

                                Get.closeAllSnackbars();

                                await Get.to(TransferToken(
                                  data: data,
                                ));
                                print(bitcoinDetails);
                              },
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
