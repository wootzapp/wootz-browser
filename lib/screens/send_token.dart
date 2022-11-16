import 'dart:math';

import 'package:bech32/bech32.dart';
import 'package:bitcoin_flutter/bitcoin_flutter.dart';
import 'package:cryptowallet/components/loader.dart';
import 'package:cryptowallet/screens/transfer_token.dart';
import 'package:cryptowallet/utils/alt_ens.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/bitcoin_util.dart';
import 'package:cryptowallet/utils/filecoin_util.dart';
import 'package:cryptowallet/utils/qr_scan_view.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:decimal/decimal.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:web3dart/crypto.dart';
import 'package:stellar_flutter_sdk/stellar_flutter_sdk.dart' as stellar
    hide Row;
import 'package:web3dart/web3dart.dart' as web3;
import 'package:bs58check/bs58check.dart' as bs58check;

import 'package:cardano_wallet_sdk/cardano_wallet_sdk.dart' as cardano;
import 'package:solana/solana.dart' as solana;
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class SendToken extends StatefulWidget {
  final Map data;

  const SendToken({this.data, Key key}) : super(key: key);

  @override
  _SendTokenState createState() => _SendTokenState();
}

class _SendTokenState extends State<SendToken> {
  final recipientAddressController = TextEditingController();
  final amount = TextEditingController();
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  RxBool isLoading = false.obs;

  @override
  void dispose() {
    amount.dispose();
    recipientAddressController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: _scaffoldKey,
      appBar: AppBar(
        title: Text(
            '${AppLocalizations.of(context).send} ${widget.data['contractAddress'] != null ? ellipsify(str: widget.data['symbol']) : widget.data['symbol']}'),
      ),
      body: SafeArea(
        child: SingleChildScrollView(
          child: Padding(
            padding: const EdgeInsets.all(25),
            child: Column(
              children: [
                TextFormField(
                  autocorrect: false,
                  keyboardType: TextInputType.visiblePassword,
                  validator: (value) {
                    if (value?.trim() == '') {
                      return AppLocalizations.of(context)
                          .receipientAddressIsRequired;
                    } else {
                      return null;
                    }
                  },
                  controller: recipientAddressController
                    ..text = widget.data["recipient"],
                  decoration: InputDecoration(
                    suffixIcon: Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        IconButton(
                          icon: const Icon(
                            Icons.qr_code_scanner,
                          ),
                          onPressed: () async {
                            String recipientAddr = await Get.to(
                              const QRScanView(),
                            );
                            if (recipientAddr == null) return;
                            recipientAddressController.text = recipientAddr;
                          },
                        ),
                        InkWell(
                          onTap: () async {
                            ClipboardData cdata =
                                await Clipboard.getData(Clipboard.kTextPlain);
                            if (cdata == null) return;
                            if (cdata.text == null) return;
                            recipientAddressController.text = cdata.text;
                          },
                          child: Padding(
                            padding: const EdgeInsets.all(8.0),
                            child: Text(
                              AppLocalizations.of(context).paste,
                            ),
                          ),
                        ),
                      ],
                    ),
                    hintText: AppLocalizations.of(context).receipientAddress +
                        (widget.data['rpc'] == null
                            ? ''
                            : ' ${AppLocalizations.of(context).or} ENS'),

                    focusedBorder: const OutlineInputBorder(
                        borderRadius: BorderRadius.all(Radius.circular(10.0)),
                        borderSide: BorderSide.none),
                    border: const OutlineInputBorder(
                        borderRadius: BorderRadius.all(Radius.circular(10.0)),
                        borderSide: BorderSide.none),
                    enabledBorder: const OutlineInputBorder(
                        borderRadius: BorderRadius.all(Radius.circular(10.0)),
                        borderSide: BorderSide.none), // you
                    filled: true,
                  ),
                ),
                const SizedBox(
                  height: 20,
                ),
                widget.data['isNFT'] != null
                    ? TextFormField(
                        enabled: false,
                        keyboardType: const TextInputType.numberWithOptions(
                            decimal: true),
                        validator: (value) {
                          if (value?.trim() == '') {
                            return AppLocalizations.of(context)
                                .amountIsRequired;
                          } else {
                            return null;
                          }
                        },
                        controller: amount
                          ..text = widget.data['tokenId'].toString(),
                        decoration: const InputDecoration(
                          focusedBorder: OutlineInputBorder(
                              borderRadius:
                                  BorderRadius.all(Radius.circular(10.0)),
                              borderSide: BorderSide.none),
                          border: OutlineInputBorder(
                              borderRadius:
                                  BorderRadius.all(Radius.circular(10.0)),
                              borderSide: BorderSide.none),
                          enabledBorder: OutlineInputBorder(
                              borderRadius:
                                  BorderRadius.all(Radius.circular(10.0)),
                              borderSide: BorderSide.none), // you
                          filled: true,
                        ),
                      )
                    : TextFormField(
                        enabled: true,
                        keyboardType: const TextInputType.numberWithOptions(
                            decimal: true),
                        validator: (value) {
                          if (value?.trim() == '') {
                            return AppLocalizations.of(context)
                                .amountIsRequired;
                          } else {
                            return null;
                          }
                        },
                        controller: amount..text = widget.data['amount'],
                        decoration: InputDecoration(
                          suffixIconConstraints:
                              const BoxConstraints(minWidth: 100),
                          suffixIcon: IconButton(
                            alignment: Alignment.centerRight,
                            icon: Text(
                              AppLocalizations.of(context).max,
                              textAlign: TextAlign.end,
                            ),
                            onPressed: () async {
                              String mnemonic = (Hive.box(secureStorageKey))
                                  .get(currentMmenomicKey);
                              if (widget.data['contractAddress'] != null) {
                                final accountDetails =
                                    await getERC20TokenBalance(widget.data);
                                amount.text = accountDetails.toString();
                              } else if (widget.data['POSNetwork'] != null) {
                                final getBitcoinDetails =
                                    await getBitcoinFromMemnomic(
                                  mnemonic,
                                  widget.data['POSNetwork'],
                                );
                                final accountDetails =
                                    await getBitcoinAddressBalance(
                                  getBitcoinDetails['address'],
                                  widget.data['POSNetwork'],
                                );

                                if (accountDetails != 0) {}

                                amount.text = accountDetails.toString();
                              } else if (widget.data['default'] == 'SOL') {
                                final getSolanaDetails =
                                    await getSolanaFromMemnomic(mnemonic);
                                double accountDetails =
                                    await getSolanaAddressBalance(
                                        getSolanaDetails['address'],
                                        widget.data['solanaCluster']);

                                if (accountDetails != 0) {
                                  final fees = await getSolanaClient(
                                          widget.data['solanaCluster'])
                                      .rpcClient
                                      .getFees();
                                  final feesDouble =
                                      fees.feeCalculator.lamportsPerSignature /
                                          pow(10, solanaDecimals);

                                  final maximumPayable =
                                      accountDetails - feesDouble;

                                  accountDetails = maximumPayable > 0
                                      ? maximumPayable
                                      : accountDetails;
                                }

                                amount.text = accountDetails.toString();
                              } else if (widget.data['default'] == 'ADA') {
                                final getCardanoDetails =
                                    await getCardanoFromMemnomic(
                                  mnemonic,
                                  widget.data['cardano_network'],
                                );
                                double accountDetails =
                                    await getCardanoAddressBalance(
                                  getCardanoDetails['address'],
                                  widget.data['cardano_network'],
                                  widget.data['blockFrostKey'],
                                );

                                if (accountDetails != 0) {
                                  final fees = maxFeeGuessForCardano /
                                      pow(10, cardanoDecimals);

                                  final maximumPayable = accountDetails - fees;

                                  accountDetails = maximumPayable > 0
                                      ? maximumPayable
                                      : accountDetails;
                                }

                                amount.text = accountDetails.toString();
                              } else if (widget.data['default'] == 'FIL') {
                                final getFileCoinDetails =
                                    await getFileCoinFromMemnomic(
                                  mnemonic,
                                  widget.data['prefix'],
                                );
                                double accountDetails =
                                    await getFileCoinAddressBalance(
                                  getFileCoinDetails['address'],
                                  baseUrl: widget.data['baseUrl'],
                                );

                                if (accountDetails != 0) {
                                  final fees = await getFileCoinTransactionFee(
                                    widget.data['prefix'],
                                    widget.data['baseUrl'],
                                  );

                                  final maximumPayable = accountDetails - fees;

                                  accountDetails = maximumPayable > 0
                                      ? maximumPayable
                                      : accountDetails;
                                }
                                amount.text = accountDetails.toString();
                              } else {
                                final getEthereumDetails =
                                    await getEthereumFromMemnomic(
                                  mnemonic,
                                  widget.data['coinType'],
                                );
                                double accountDetails =
                                    await getEthereumAddressBalance(
                                  getEthereumDetails['eth_wallet_address'],
                                  widget.data['rpc'],
                                  coinType: widget.data['coinType'],
                                );

                                if (accountDetails != 0) {
                                  final response =
                                      await getEthereumFromMemnomic(
                                    mnemonic,
                                    widget.data['coinType'],
                                  );
                                  final transactionFee =
                                      await getEtherTransactionFee(
                                    widget.data['rpc'],
                                    null,
                                    web3.EthereumAddress.fromHex(
                                      response['eth_wallet_address'],
                                    ),
                                    web3.EthereumAddress.fromHex(zeroAddress),
                                    value: 1,
                                  );

                                  final maximumPayable = accountDetails -
                                      (transactionFee / pow(10, etherDecimals));

                                  accountDetails = maximumPayable > 0
                                      ? maximumPayable
                                      : accountDetails;
                                }
                                amount.text = accountDetails.toString();
                              }
                            },
                          ),
                          hintText: AppLocalizations.of(context).amount,

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
                  height: 30,
                ),
                Obx(
                  () => SizedBox(
                    width: double.infinity,
                    height: 50,
                    child: ElevatedButton(
                      style: ButtonStyle(
                        backgroundColor: MaterialStateProperty.resolveWith(
                            (states) => appBackgroundblue),
                        shape: MaterialStateProperty.resolveWith(
                          (states) => RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(10),
                          ),
                        ),
                        textStyle: MaterialStateProperty.resolveWith(
                          (states) => const TextStyle(color: Colors.white),
                        ),
                      ),
                      child: isLoading.value
                          ? const Loader()
                          : Text(
                              AppLocalizations.of(context).send,
                              style: const TextStyle(
                                fontWeight: FontWeight.bold,
                                color: Colors.white,
                              ),
                            ),
                      onPressed: () async {
                        if (isLoading.value) return;
                        // hide snackbar if it is showing
                        Get.closeAllSnackbars();
                        FocusManager.instance.primaryFocus?.unfocus();
                        // check if recipinet is valid eth address

                        if (double.tryParse(amount.text.trim()) == null) {
                          Get.snackbar(
                            '',
                            AppLocalizations.of(context).pleaseEnterAmount,
                            colorText: Colors.white,
                            backgroundColor: Colors.red,
                          );
                          return;
                        }

                        String recipient =
                            recipientAddressController.text.trim();
                        String cryptoDomain;
                        bool iscryptoDomain = recipient.contains('.');

                        try {
                          isLoading.value = true;

                          if (widget.data['default'] == 'XLM' &&
                              iscryptoDomain) {
                            try {
                              stellar.FederationResponse response =
                                  await stellar.Federation
                                      .resolveStellarAddress(recipient);
                              cryptoDomain = recipient;
                              recipient = response.accountId;
                            } catch (_) {}
                          } else if (iscryptoDomain) {
                            Map ensAddress = await ensToAddress(
                              cryptoDomainName: recipient,
                            );

                            if (ensAddress['success']) {
                              cryptoDomain = recipient;
                              recipient = ensAddress['msg'];
                            } else {
                              Map unstoppableDomainAddr =
                                  await unstoppableDomainENS(
                                cryptoDomainName: recipient,
                                currency: widget.data['rpc'] == null
                                    ? widget.data['default']
                                    : null,
                              );
                              cryptoDomain = unstoppableDomainAddr['success']
                                  ? recipient
                                  : null;
                              recipient = unstoppableDomainAddr['success']
                                  ? unstoppableDomainAddr['msg']
                                  : recipient;
                            }
                          }

                          isLoading.value = false;

                          if (widget.data['POSNetwork'] != null &&
                              !Address.validateAddress(
                                recipient,
                                widget.data['POSNetwork'],
                              )) {
                            final NetworkType nw = widget.data['POSNetwork'];

                            bool canReceivePayment = false;

                            try {
                              final base58DecodeRecipient =
                                  bs58check.decode(recipient);

                              final pubHashString = base58DecodeRecipient[0]
                                      .toRadixString(16) +
                                  base58DecodeRecipient[1].toRadixString(16);

                              canReceivePayment =
                                  hexToInt(pubHashString).toInt() ==
                                      nw.pubKeyHash;
                            } catch (_) {}

                            if (!canReceivePayment) {
                              Bech32 sel = bech32.decode(recipient);
                              canReceivePayment = nw.bech32 == sel.hrp;
                            }

                            if (!canReceivePayment) {
                              throw Exception(
                                  'Invalid ${widget.data['symbol']} address');
                            }
                          } else if (widget.data['default'] == 'SOL') {
                            solana.Ed25519HDPublicKey.fromBase58(recipient);
                          } else if (widget.data['default'] == 'ADA') {
                            cardano.ShelleyAddress.fromBech32(recipient);
                          } else if (widget.data['default'] == 'XLM') {
                            stellar.KeyPair.fromAccountId(recipient);
                          } else if (widget.data['default'] == 'FIL') {
                            //FIXME:
                            // if (!await Flotus.validateAddress(recipient)) {
                            //   throw Exception('not a valid filecoin address');
                            // }
                          } else if (widget.data['default'] == 'ATOM') {
                            Bech32 sel = bech32.decode(recipient);
                            if (sel.hrp != widget.data['bech32Hrp']) {
                              throw Exception('not a valid cosmos address');
                            }
                          } else if (widget.data['rpc'] != null) {
                            web3.EthereumAddress.fromHex(recipient);
                          }
                        } catch (e) {
                          isLoading.value = false;

                          if (kDebugMode) {
                            print(e);
                          }
                          Get.snackbar(
                            '',
                            AppLocalizations.of(context).canNotSend(
                              widget.data['symbol'],
                            ),
                            colorText: Colors.white,
                            backgroundColor: Colors.red,
                          );

                          return;
                        }
                        if (amount.text.trim() == "" || recipient == "") {
                          return;
                        }
                        final data = {
                          ...widget.data,
                          'amount': Decimal.parse(amount.text).toString(),
                          'recipient': recipient
                        };

                        Get.closeAllSnackbars();

                        await Get.to(TransferToken(
                          data: data,
                          cryptoDomain: cryptoDomain,
                        ));
                      },
                    ),
                  ),
                )
              ],
            ),
          ),
        ),
      ),
    );
  }
}
