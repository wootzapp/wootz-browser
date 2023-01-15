import 'dart:math';

import 'package:awesome_dialog/awesome_dialog.dart';
import 'package:cryptowallet/eip/eip681.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:decimal/decimal.dart';
import 'package:flutter/foundation.dart';
// import 'package:get/get.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:hive/hive.dart';
import 'package:path/path.dart';
import 'package:qr_flutter/qr_flutter.dart';
import 'package:share/share.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

import '../components/loader.dart';

class ReceiveToken extends StatefulWidget {
  final Map data;
  final String mnemonic;
  const ReceiveToken({Key key, this.data, this.mnemonic}) : super(key: key);

  @override
  _ReceiveTokenState createState() => _ReceiveTokenState();
}

class _ReceiveTokenState extends State<ReceiveToken> {
  final scaffoldKey = GlobalKey<ScaffoldState>();
  // RxString userAddress = "".obs;
  final userAddress = ValueNotifier<String>("");

  // RxBool isRequestingPayment = false.obs;
  final isRequestingPayment = ValueNotifier<bool>(false);
  // RxString amountRequested = "".obs;
  final amountRequested = ValueNotifier<String>("");
  TextEditingController amountField = TextEditingController();

  @override
  void dispose() {
    FocusManager.instance.primaryFocus?.unfocus();
    amountField.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
            '${AppLocalizations.of(context).receive} ${widget.data['contractAddress'] != null ? ellipsify(str: widget.data['symbol']) : widget.data['symbol']}'),
      ),
      key: scaffoldKey,
      body: FutureBuilder(
        future: () async {
          final pref = Hive.box(secureStorageKey);
          String mnemonic = pref.get(currentMmenomicKey);
          if (widget.data['POSNetwork'] != null) {
            final getBitCoinDetails = await getBitcoinFromMemnomic(
              mnemonic,
              widget.data['POSNetwork'],
            );

            return {'address': getBitCoinDetails['address']};
          } else if (widget.data['default'] == 'SOL') {
            final solanaDetails = await getSolanaFromMemnomic(mnemonic);
            return {'address': solanaDetails['address']};
          } else if (widget.data['default'] == 'ADA') {
            final cardano = await getCardanoFromMemnomic(
              mnemonic,
              widget.data['cardano_network'],
            );
            return {'address': cardano['address']};
          } else if (widget.data['default'] == 'FIL') {
            final filecoin = await getFileCoinFromMemnomic(
              mnemonic,
              widget.data['prefix'],
            );
            return {'address': filecoin['address']};
          } else if (widget.data['default'] == 'XLM') {
            final stellar = await getStellarFromMemnomic(mnemonic);
            return {'address': stellar['address']};
          }
          final response = await getEthereumFromMemnomic(
            mnemonic,
            widget.data['coinType'],
          );
          return {
            'address': response['eth_wallet_address'],
            'isEthReceivePayment': true
          };
        }(),
        builder: (context, snapshot) {
          if (snapshot.hasError) {
            if (kDebugMode) {
              print(snapshot.error.toString() + 'error here');
            }
          }
          if (snapshot.hasData) {
            if (!isRequestingPayment.value) {
              userAddress.value = snapshot.data['address'];
            }
            return SafeArea(
              child: SingleChildScrollView(
                child: Padding(
                  padding: const EdgeInsets.all(25),
                  child: Column(
                    children: [
                      SizedBox(
                        width: 300,
                        height: 300,
                        child: Card(
                          color: const Color(0xffF1F1F1),
                          shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(10)),
                          child: Align(
                            alignment: Alignment.center,
                            child: Padding(
                              padding:
                                  const EdgeInsets.only(top: 10, bottom: 10),
                              child: ValueListenableBuilder(
                                  valueListenable: userAddress,
                                  builder: (context, value, child) {
                                    return QrImage(
                                      data: userAddress.value,
                                      version: QrVersions.auto,
                                      size: 250,
                                    );
                                  }),
                            ),
                          ),
                        ),
                      ),
                      const SizedBox(
                        height: 40,
                      ),
                      GestureDetector(
                        onTap: () async {
                          // copy to clipboard
                          await Clipboard.setData(ClipboardData(
                            text: (snapshot.data as Map)['address'],
                          ));
                          // Get.snackbar(
                          //   '',
                          //   AppLocalizations.of(context).copiedToClipboard,
                          // );
                          ScaffoldMessenger.of(context).showSnackBar(
                            SnackBar(
                              content: Text(
                                AppLocalizations.of(context).copiedToClipboard,
                              ),
                              backgroundColor: Colors.red,
                              action: SnackBarAction(
                                label: 'OK',
                                textColor: Colors.white,
                                onPressed: () {},
                              ),
                            ),
                          );
                        },
                        child: Card(
                          shape: RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(10)),
                          color: colorForAddress,
                          child: Padding(
                            padding: const EdgeInsets.all(10),
                            child: Column(
                              mainAxisAlignment: MainAxisAlignment.start,
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  (snapshot.data as Map)['address'],
                                  textAlign: TextAlign.center,
                                  style: const TextStyle(
                                    fontSize: 14,
                                    color: Colors.black,
                                  ),
                                ),
                              ],
                            ),
                          ),
                        ),
                      ),
                      ValueListenableBuilder(
                          valueListenable: amountRequested,
                          builder: (context, value, child) {
                            return amountRequested != null
                                ? Text(amountRequested.value)
                                : Container();
                          }),
                      const SizedBox(
                        height: 40,
                      ),
                      Text.rich(
                          TextSpan(children: [
                            TextSpan(
                              text: AppLocalizations.of(context).sendOnly(
                                '${widget.data['contractAddress'] != null ? ellipsify(str: widget.data['name']) : widget.data['name']} (${widget.data['contractAddress'] != null ? ellipsify(str: widget.data['symbol']) : widget.data['symbol']})',
                              ),
                            ),
                          ]),
                          textAlign: TextAlign.center),
                      const SizedBox(
                        height: 40,
                      ),
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceAround,
                        children: [
                          Column(
                            children: [
                              GestureDetector(
                                  onTap: () async {
                                    // copy to clipboard
                                    await Clipboard.setData(ClipboardData(
                                      text: (snapshot.data as Map)['address'],
                                    ));

                                    // Get.snackbar(
                                    //   '',
                                    //   AppLocalizations.of(context)
                                    //       .copiedToClipboard,
                                    // );
                                    ScaffoldMessenger.of(context).showSnackBar(
                                      SnackBar(
                                        content: Text(
                                          AppLocalizations.of(context)
                                              .copiedToClipboard,
                                        ),
                                        backgroundColor: Colors.red,
                                        action: SnackBarAction(
                                          label: 'OK',
                                          textColor: Colors.white,
                                          onPressed: () {},
                                        ),
                                      ),
                                    );
                                  },
                                  child: Container(
                                      width: 40,
                                      height: 40,
                                      decoration: const BoxDecoration(
                                        shape: BoxShape.circle,
                                        color: Color(0xff0C66F1),
                                      ),
                                      child: const Icon(Icons.copy,
                                          color: Colors.white))),
                              const SizedBox(
                                height: 5,
                              ),
                              Text(AppLocalizations.of(context).copy),
                            ],
                          ),
                          Column(
                            children: [
                              GestureDetector(
                                  onTap: () async {
                                    await Share.share(
                                        '${AppLocalizations.of(context).publicAddressToReceive} ${widget.data['symbol']} ${(snapshot.data as Map)['address']}');
                                  },
                                  child: Container(
                                      width: 40,
                                      height: 40,
                                      decoration: const BoxDecoration(
                                        shape: BoxShape.circle,
                                        color: Color(0xff0C66F1),
                                      ),
                                      child: const Icon(Icons.share,
                                          color: Colors.white))),
                              const SizedBox(
                                height: 5,
                              ),
                              Text(AppLocalizations.of(context).share),
                            ],
                          ),
                          if (snapshot.data['isEthReceivePayment'] != null)
                            Column(
                              children: [
                                GestureDetector(
                                    onTap: () {
                                      AwesomeDialog(
                                        showCloseIcon: true,
                                        context: context,
                                        closeIcon: const Icon(
                                          Icons.close,
                                        ),
                                        animType: AnimType.SCALE,
                                        dialogType: DialogType.INFO,
                                        keyboardAware: true,
                                        body: Padding(
                                          padding: const EdgeInsets.all(8.0),
                                          child: Column(
                                            children: <Widget>[
                                              Text(
                                                AppLocalizations.of(context)
                                                    .requestPayment,
                                              ),
                                              const SizedBox(
                                                height: 10,
                                              ),
                                              Material(
                                                elevation: 0,
                                                color: Colors.blueGrey
                                                    .withAlpha(40),
                                                child: TextFormField(
                                                  keyboardType:
                                                      TextInputType.number,
                                                  controller: amountField,
                                                  autofocus: true,
                                                  minLines: 1,
                                                  decoration: InputDecoration(
                                                    border: InputBorder.none,
                                                    labelText:
                                                        AppLocalizations.of(
                                                                context)
                                                            .amount,
                                                    prefixIcon: const Icon(
                                                        Icons.text_fields),
                                                  ),
                                                ),
                                              ),
                                              const SizedBox(
                                                height: 10,
                                              ),
                                              AnimatedButton(
                                                isFixedHeight: false,
                                                text:
                                                    AppLocalizations.of(context)
                                                        .ok,
                                                pressEvent: () {
                                                  if (Navigator.canPop(
                                                      context)) {
                                                    // Get.back();
                                                    Navigator.of(context).pop();
                                                  }

                                                  FocusManager
                                                      .instance.primaryFocus
                                                      ?.unfocus();
                                                  String ethereumRequestURL;

                                                  if (amountField.text !=
                                                          null &&
                                                      double.tryParse(
                                                              amountField.text
                                                                  .trim()) !=
                                                          null) {
                                                    Decimal
                                                        amountUserEnteredInDecimals =
                                                        Decimal.parse(
                                                            amountField.text
                                                                .trim());
                                                    try {
                                                      if (widget.data[
                                                              'default'] !=
                                                          null) {
                                                        ethereumRequestURL = EIP681.build(
                                                            targetAddress:
                                                                (snapshot.data
                                                                        as Map)[
                                                                    'address'],
                                                            chainId: widget
                                                                .data['chainId']
                                                                .toString(),
                                                            parameters: {
                                                              'value': (amountUserEnteredInDecimals *
                                                                      Decimal.parse(pow(
                                                                              10,
                                                                              18)
                                                                          .toString()))
                                                                  .toString()
                                                            });
                                                      } else {
                                                        ethereumRequestURL =
                                                            EIP681.build(
                                                                targetAddress:
                                                                    widget.data[
                                                                        'contractAddress'],
                                                                chainId: widget
                                                                    .data[
                                                                        'chainId']
                                                                    .toString(),
                                                                functionName:
                                                                    'transfer',
                                                                parameters: {
                                                              'uint256': (amountUserEnteredInDecimals *
                                                                      Decimal.parse(pow(
                                                                              10,
                                                                              double.parse(widget.data['decimals']))
                                                                          .toString()))
                                                                  .toString(),
                                                              'address':
                                                                  (snapshot.data
                                                                          as Map)[
                                                                      'address']
                                                            });
                                                      }
                                                    } catch (e) {
                                                      if (kDebugMode) {
                                                        print(e);
                                                      }
                                                    }

                                                    if (kDebugMode) {
                                                      print(ethereumRequestURL);
                                                    }
                                                  }

                                                  isRequestingPayment.value =
                                                      true;
                                                  amountRequested.value =
                                                      ethereumRequestURL != null
                                                          ? "+${amountField.text.trim()} ${widget.data['symbol']}"
                                                          : null;
                                                  amountField.text = '';
                                                  userAddress.value =
                                                      ethereumRequestURL ??
                                                          (snapshot.data
                                                                  as Map)[
                                                              'address'];
                                                },
                                              )
                                            ],
                                          ),
                                        ),
                                      ).show();
                                    },
                                    child: Container(
                                        width: 40,
                                        height: 40,
                                        decoration: const BoxDecoration(
                                          shape: BoxShape.circle,
                                          color: Colors.black,
                                        ),
                                        child: const Icon(Icons.add,
                                            color: Colors.white))),
                                const SizedBox(
                                  height: 5,
                                ),
                                Text(AppLocalizations.of(context).request),
                              ],
                            ),
                        ],
                      ),
                    ],
                  ),
                ),
              ),
            );
          }
          return const Center(child: Loader());
        },
      ),
    );
  }
}
