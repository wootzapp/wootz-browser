import 'dart:convert';

import 'package:bitcoin_flutter/bitcoin_flutter.dart';
import 'package:cryptowallet/config/colors.dart';
import 'package:cryptowallet/utils/qr_scan_view.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:decimal/decimal.dart';
import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';

import '../components/loader.dart';
import '../screens/main_screen.dart';
import '../screens/security.dart';
import '../screens/send_token.dart';
import '../screens/transfer_token.dart';
import '../screens/view_profiles.dart';
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
  final btcUSD = TextEditingController();
  num bitcoinPrice;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: () {
        FocusScopeNode currentFocus = FocusScope.of(context);

        if (!currentFocus.hasPrimaryFocus) {
          currentFocus.unfocus();
        }
      },
      child: Scaffold(
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
                                const Text('Profiles'),
                                GestureDetector(
                                  onTap: () async {
                                    final pref = Hive.box(secureStorageKey);
                                    final mnemonics = pref.get(mnemonicListKey);

                                    final currentPhrase =
                                        pref.get(currentMmenomicKey);

                                    if (currentPhrase != null) {
                                      await Navigator.of(context)
                                          .push(MaterialPageRoute(
                                        builder: (_) => ViewProfiles(
                                          data: (jsonDecode(mnemonics) as List),
                                          currentPhrase: currentPhrase,
                                        ),
                                      ));
                                    } else {
                                      ScaffoldMessenger.of(context)
                                          .showSnackBar(SnackBar(
                                        content:
                                            const Text('No Profiles Created'),
                                        backgroundColor: Colors.red,
                                        action: SnackBarAction(
                                          label: 'OK',
                                          onPressed: () {},
                                          textColor: Colors.white,
                                        ),
                                      ));
                                    }
                                  },
                                  child: Row(
                                    children: const [
                                      Text('See all Profiles'),
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
                                // Get.back();
                                // Navigator.of(context).pop();

                                if (hasPasscode) {
                                  dappWidget = const MainScreen();
                                } else {
                                  dappWidget = const Security();
                                }
                                // await Get.to(dappWidget);
                                Navigator.of(context).push(MaterialPageRoute(
                                    builder: (_) => dappWidget));
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
                      const SizedBox(
                        height: 100,
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
                                    onChanged: (value) async {
                                      Decimal amount = Decimal.tryParse(value);
                                      if (amount == null) return;
                                      if (bitcoinPrice != null) {
                                        double conversion =
                                            amount.toDouble() * bitcoinPrice;
                                        btcUSD.text = conversion.toString();
                                      }
                                      const dataUrl =
                                          'https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd&include_24hr_change=true';

                                      final response =
                                          await get(Uri.parse(dataUrl))
                                              .timeout(networkTimeOutDuration);
                                      final responseBody = response.body;

                                      if (response.statusCode ~/ 100 == 4 ||
                                          response.statusCode ~/ 100 == 5) {
                                        throw Exception(responseBody);
                                      }

                                      final bitcoinResponse =
                                          jsonDecode(responseBody);
                                      bitcoinPrice = bitcoinResponse['bitcoin']
                                              ['usd']
                                          .toDouble();
                                      double conversion =
                                          amount.toDouble() * bitcoinPrice;
                                      btcUSD.text = conversion.toString();
                                    },
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
                                    controller: btcUSD,
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
                                    // Get.snackbar(
                                    //   '',
                                    //   AppLocalizations.of(context).invalidAddress,
                                    //   colorText: Colors.white,
                                    //   backgroundColor: Colors.red,
                                    // );
                                    ScaffoldMessenger.of(context).showSnackBar(
                                      SnackBar(
                                          content: Text(
                                              AppLocalizations.of(context)
                                                  .invalidAddress),
                                          backgroundColor: Colors.red,
                                          action: SnackBarAction(
                                            textColor: Colors.white,
                                            label: 'OK',
                                            onPressed: () {},
                                          )),
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

                                  // Get.closeAllSnackbars();
//                                 for (var i = 0; i < context.visitor.widgetCount; i++) {
//     Scaffold.of(context.visitor[i].context).hideCurrentSnackBar();
// }
                                  ScaffoldMessenger.of(context)
                                      .removeCurrentSnackBar();

                                  // await Get.to(TransferToken(
                                  //   data: data,
                                  // ));
                                  Navigator.of(context).push(
                                    MaterialPageRoute(
                                      builder: (_) => TransferToken(
                                        data: data,
                                      ),
                                    ),
                                  );
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
                              String data = await Navigator.of(context)
                                  .push(MaterialPageRoute(
                                builder: (_) => const QRScanView(),
                              ));
                              if (data == null) {
                                // Get.snackbar(
                                //   '',
                                //   eIP681ProcessingErrorMsg,
                                //   colorText: Colors.white,
                                //   backgroundColor: Colors.red,
                                // );
                                ScaffoldMessenger.of(context).showSnackBar(
                                  SnackBar(
                                    content:
                                        const Text(eIP681ProcessingErrorMsg),
                                    backgroundColor: Colors.red,
                                    action: SnackBarAction(
                                      textColor: Colors.white,
                                      label: 'OK',
                                      onPressed: (() {}),
                                    ),
                                  ),
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
                                Navigator.of(context).pop();
                              }

                              if (scannedData['success']) {
                                await Navigator.of(context)
                                    .push(MaterialPageRoute(
                                  builder: (_) => SendToken(
                                    data: scannedData['msg'],
                                  ),
                                ));
                                return;
                              }
                              // Get.snackbar(
                              //   '',
                              //   scannedData['msg'],
                              //   colorText: Colors.white,
                              //   backgroundColor: Colors.red,
                              // );
                              ScaffoldMessenger.of(context).showSnackBar(
                                SnackBar(
                                  content: Text(scannedData['msg']),
                                  backgroundColor: Colors.red,
                                  action: SnackBarAction(
                                    label: 'OK',
                                    textColor: Colors.white,
                                    onPressed: () {},
                                  ),
                                ),
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
                              bool hasWallet =
                                  pref.get(currentMmenomicKey) != null;

                              bool hasPasscode =
                                  pref.get(userUnlockPasscodeKey) != null;
                              Widget dappWidget;
                              // Get.back();
                              // Navigator.of(context).pop();

                              if (hasWallet) {
                                dappWidget = const WalletMainBody();
                              } else if (hasPasscode) {
                                dappWidget = const MainScreen();
                              } else {
                                dappWidget = const Security();
                              }
                              await Navigator.of(context).push(
                                  MaterialPageRoute(
                                      builder: (_) => dappWidget));
                              // await Get.to(dappWidget);
                            },
                            child: SizedBox(
                              width: 100,
                              child: Column(
                                children: const [
                                  Icon(FontAwesomeIcons.wallet),
                                  Text(
                                    'My wallet',
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
      ),
    );
  }
}
