import 'dart:async';
import 'dart:convert';
import 'dart:math';

import 'package:cryptowallet/components/user_balance.dart';
import 'package:cryptowallet/crypto_charts/crypto_chart.dart';
import 'package:cryptowallet/screens/dapp.dart';
import 'package:cryptowallet/screens/receive_token.dart';
import 'package:cryptowallet/screens/send_token.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/bitcoin_util.dart';
import 'package:cryptowallet/utils/format_money.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_svg/svg.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:intl/intl.dart';
import 'package:page_transition/page_transition.dart';

class Token extends StatefulWidget {
  final Map data;
  const Token({this.data, Key key}) : super(key: key);

  @override
  _TokenState createState() => _TokenState();
}

class _TokenState extends State<Token> {
  RxMap tokenTransaction = {}.obs;
  RxDouble cryptoBalance = 0.0.obs;
  RxMap blockchainPrice = {}.obs;
  bool skipNetworkRequest = true;
  Timer timer;
  RxBool trxOpen = true.obs;
  final List<String> months = [
    'Jan',
    'Feb',
    'Mar',
    'Apr',
    'May',
    'Jun',
    'Jul',
    'Aug',
    'Sep',
    'Oct',
    'Nov',
    'Dec'
  ];

  @override
  void initState() {
    super.initState();
    callTokenApi();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await callTokenApi(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  Future callTokenApi() async {
    await getTokenTransactions();
    await getBlockchainPrice();
    await getBlockchainBalance();
    if (skipNetworkRequest) skipNetworkRequest = false;
  }

  Future getBlockchainPrice() async {
    try {
      final bool priceNotavailble =
          widget.data['noPrice'] != null && widget.data['noPrice'] == true;
      if (priceNotavailble) return;

      final currencyWithSymbol =
          jsonDecode(await rootBundle.loadString('json/currency_symbol.json'))
              as Map;
      final defaultCurrency =
          Hive.box(secureStorageKey).get('defaultCurrency') ?? "USD";

      final symbol = (currencyWithSymbol[defaultCurrency]['symbol']);

      Map allCryptoPrice = jsonDecode(
        await getCryptoPrice(
          skipNetworkRequest: skipNetworkRequest,
        ),
      ) as Map;

      final Map cryptoMarket =
          allCryptoPrice[coinGeckCryptoSymbolToID[widget.data['symbol']]];

      final double price =
          (cryptoMarket[defaultCurrency.toLowerCase()] as num).toDouble();

      final change =
          (cryptoMarket[defaultCurrency.toLowerCase() + '_24h_change'] as num)
              .toDouble();

      blockchainPrice.value = {
        'price': price,
        'change': change,
        'symbol': symbol
      };
    } catch (_) {
      print(_);
    }
  }

  Future getBlockchainBalance() async {
    try {
      final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);
      if (widget.data['contractAddress'] != null) {
        cryptoBalance.value = await getERC20TokenBalance(
          widget.data,
          skipNetworkRequest: skipNetworkRequest,
        );
      } else if (widget.data['POSNetwork'] != null) {
        final getBitcoinDetails = await getBitcoinFromMemnomic(
          mnemonic,
          widget.data['POSNetwork'],
        );
        cryptoBalance.value = await getBitcoinAddressBalance(
          getBitcoinDetails['address'],
          widget.data['POSNetwork'],
          skipNetworkRequest: skipNetworkRequest,
        );
      } else if (widget.data['default'] == 'SOL') {
        final getSolanaDetails = await getSolanaFromMemnomic(mnemonic);
        cryptoBalance.value = await getSolanaAddressBalance(
          getSolanaDetails['address'],
          widget.data['solanaCluster'],
          skipNetworkRequest: skipNetworkRequest,
        );
      } else if (widget.data['default'] == 'ADA') {
        final getCardanoDetails = await getCardanoFromMemnomic(
          mnemonic,
          widget.data['cardano_network'],
        );
        cryptoBalance.value = await getCardanoAddressBalance(
          getCardanoDetails['address'],
          widget.data['cardano_network'],
          widget.data['blockFrostKey'],
          skipNetworkRequest: skipNetworkRequest,
        );
      } else if (widget.data['default'] == 'FIL') {
        final getFileCoinDetails = await getFileCoinFromMemnomic(
          mnemonic,
          widget.data['prefix'],
        );
        cryptoBalance.value = await getFileCoinAddressBalance(
          getFileCoinDetails['address'],
          baseUrl: widget.data['baseUrl'],
          skipNetworkRequest: skipNetworkRequest,
        );
      } else if (widget.data['default'] == 'XLM') {
        final getStellarDetails = await getStellarFromMemnomic(
          mnemonic,
        );

        cryptoBalance.value = await getStellarAddressBalance(
          getStellarDetails['address'],
          widget.data['sdk'],
          widget.data['cluster'],
          skipNetworkRequest: skipNetworkRequest,
        );
      } else {
        final getEthereumDetails = await getEthereumFromMemnomic(
          mnemonic,
          widget.data['coinType'],
        );
        final ethBalance = await getEthereumAddressBalance(
          getEthereumDetails['eth_wallet_address'],
          widget.data['rpc'],
          coinType: widget.data['coinType'],
          skipNetworkRequest: skipNetworkRequest,
        );
        cryptoBalance.value = ethBalance;
      }
    } catch (_) {}
  }

  Future getTokenTransactions() async {
    try {
      final pref = Hive.box(secureStorageKey);

      String mnemonic = pref.get(currentMmenomicKey);
      String currentAddress;

      if (widget.data['POSNetwork'] != null) {
        currentAddress = (await getBitcoinFromMemnomic(
          mnemonic,
          widget.data['POSNetwork'],
        ))['address'];
      } else if (widget.data['default'] == 'SOL') {
        currentAddress = (await getSolanaFromMemnomic(mnemonic))['address']
            .toString()
            .toLowerCase();
      } else if (widget.data['default'] == 'ADA') {
        currentAddress = (await getCardanoFromMemnomic(
          mnemonic,
          widget.data['cardano_network'],
        ))['address']
            .toString()
            .toLowerCase();
      } else if (widget.data['default'] == 'FIL') {
        currentAddress = (await getFileCoinFromMemnomic(
          mnemonic,
          widget.data['prefix'],
        ))['address']
            .toString()
            .toLowerCase();
      } else if (widget.data['default'] == 'XLM') {
        currentAddress = (await getStellarFromMemnomic(mnemonic))['address']
            .toString()
            .toLowerCase();
      } else {
        final response = await getEthereumFromMemnomic(
          mnemonic,
          widget.data['coinType'],
        );
        currentAddress =
            response['eth_wallet_address'].toString().toLowerCase();
      }
      String contractAddrLookUpkey;
      String evmAddrLookUpkey;

      if (widget.data['rpc'] != null) {
        contractAddrLookUpkey =
            '${widget.data['contractAddress']}${widget.data['rpc']} Details';
        evmAddrLookUpkey =
            '${widget.data['default']}${widget.data['rpc']} Details';
      }

      final isContractAddress = widget.data['contractAddress'] != null;
      final isEvmAddress = widget.data['rpc'] != null;

      if (isContractAddress && pref.get(contractAddrLookUpkey) != null) {
        tokenTransaction.value = {
          'trx': jsonDecode(pref.get(contractAddrLookUpkey)),
          'currentUser': currentAddress
        };
      } else if (widget.data['default'] != null &&
          isEvmAddress &&
          pref.get(evmAddrLookUpkey) != null) {
        tokenTransaction.value = {
          'trx': jsonDecode(pref.get(evmAddrLookUpkey)),
          'currentUser': currentAddress
        };
      } else if (widget.data['default'] != null &&
          !isEvmAddress &&
          pref.get('${widget.data['default']} Details') != null) {
        tokenTransaction.value = {
          'trx': jsonDecode(pref.get('${widget.data['default']} Details')),
          'currentUser': currentAddress
        };
      } else {
        tokenTransaction.value = {
          'trx': [],
          'currentUser': currentAddress,
        };
      }
    } catch (_) {}
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
          widget.data['contractAddress'] != null
              ? ellipsify(str: widget.data['name'])
              : widget.data['name'],
        ),
      ),
      body: SizedBox(
        height: double.infinity,
        child: SafeArea(
          child: RefreshIndicator(
            onRefresh: () async {},
            child: SingleChildScrollView(
              physics: const AlwaysScrollableScrollPhysics(),
              child: Column(
                children: [
                  Padding(
                    padding: const EdgeInsets.all(25),
                    child: Column(
                      children: [
                        SizedBox(
                          width: double.infinity,
                          height: 300,
                          child: Card(
                            shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(10)),
                            child: Padding(
                              padding: const EdgeInsets.only(
                                  left: 10, right: 10, top: 20),
                              child: Column(
                                children: [
                                  Row(
                                    mainAxisAlignment:
                                        MainAxisAlignment.spaceBetween,
                                    children: [
                                      Text(
                                        widget.data['contractAddress'] != null
                                            ? widget.data['network']
                                            : AppLocalizations.of(context).coin,
                                        style: const TextStyle(
                                            fontSize: 16, color: Colors.grey),
                                      ),
                                      widget.data['noPrice'] != null
                                          ? Text(
                                              '\$0',
                                              style: TextStyle(
                                                  fontSize: 16,
                                                  color: widget.data[
                                                              'contractAddress'] !=
                                                          null
                                                      ? const Color(0x00ffffff)
                                                      : null),
                                            )
                                          : Container(),
                                      Obx(() {
                                        return blockchainPrice != null &&
                                                blockchainPrice.isNotEmpty
                                            ? Row(
                                                children: [
                                                  Text(
                                                    '${widget.data['contractAddress'] != null ? ellipsify(str: blockchainPrice['symbol']) : (blockchainPrice)['symbol']}${formatMoney((blockchainPrice)['price'])}',
                                                    style: const TextStyle(
                                                        fontSize: 16),
                                                  ),
                                                  const SizedBox(
                                                    width: 5,
                                                  ),
                                                  Text(
                                                    ((blockchainPrice)[
                                                                    'change'] >
                                                                0
                                                            ? '+'
                                                            : '') +
                                                        formatMoney(
                                                            (blockchainPrice)[
                                                                'change']) +
                                                        '%',
                                                    style: TextStyle(
                                                      fontSize: 14,
                                                      color: ((blockchainPrice)[
                                                                  'change'] <
                                                              0)
                                                          ? red
                                                          : green,
                                                    ),
                                                  )
                                                ],
                                              )
                                            : const Text(
                                                '',
                                                style: TextStyle(fontSize: 18),
                                              );
                                      }),
                                    ],
                                  ),
                                  const SizedBox(
                                    height: 20,
                                  ),
                                  widget.data['image'] != null
                                      ? CircleAvatar(
                                          radius: 30,
                                          backgroundImage: AssetImage(
                                            widget.data['image'],
                                          ),
                                          backgroundColor:
                                              Theme.of(context).backgroundColor,
                                        )
                                      : CircleAvatar(
                                          radius: 30,
                                          child: Text(
                                            ellipsify(
                                                str: widget.data['symbol'],
                                                maxLength: 3),
                                            textAlign: TextAlign.center,
                                            style: const TextStyle(
                                              color: Colors.white,
                                            ),
                                          ),
                                        ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Obx(() {
                                    return cryptoBalance != null
                                        ? UserBalance(
                                            iconSize: 20,
                                            textStyle: const TextStyle(
                                                fontSize: 20,
                                                fontWeight: FontWeight.bold),
                                            balance: cryptoBalance.value,
                                            symbol: widget.data[
                                                        'contractAddress'] !=
                                                    null
                                                ? ellipsify(
                                                    str: widget.data['symbol'])
                                                : widget.data['symbol'],
                                          )
                                        : const Text(
                                            '',
                                            style: TextStyle(
                                              fontSize: 20,
                                              fontWeight: FontWeight.bold,
                                            ),
                                          );
                                  }),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  const Divider(),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Row(
                                      mainAxisAlignment:
                                          MainAxisAlignment.center,
                                      children: [
                                        Column(
                                          children: [
                                            GestureDetector(
                                              onTap: () async {
                                                Get.to(
                                                  SendToken(
                                                    data: widget.data,
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
                                                child: const Icon(
                                                    Icons.arrow_upward,
                                                    color: Colors.white),
                                              ),
                                            ),
                                            const SizedBox(
                                              height: 5,
                                            ),
                                            Text(AppLocalizations.of(context)
                                                .send),
                                          ],
                                        ),
                                        const SizedBox(
                                          width: 40,
                                        ),
                                        Column(
                                          children: [
                                            GestureDetector(
                                              onTap: () async {
                                                final mnemonic =
                                                    Hive.box(secureStorageKey)
                                                        .get('mmemonic');

                                                Get.to(
                                                  ReceiveToken(
                                                    data: widget.data,
                                                    mnemonic: mnemonic,
                                                  ),
                                                );
                                              },
                                              child: Container(
                                                width: 40,
                                                height: 40,
                                                decoration: const BoxDecoration(
                                                  shape: BoxShape.circle,
                                                  color: Color(0xff171840),
                                                ),
                                                child: const Icon(
                                                    Icons.arrow_downward,
                                                    color: Colors.white),
                                              ),
                                            ),
                                            const SizedBox(
                                              height: 5,
                                            ),
                                            Text(AppLocalizations.of(context)
                                                .receive),
                                          ],
                                        ),
                                      ]),
                                  const SizedBox(
                                    height: 20,
                                  ),
                                ],
                              ),
                            ),
                          ),
                        ),
                        const SizedBox(
                          height: 20,
                        ),
                        Obx(
                          () => GestureDetector(
                            onTap: () {
                              trxOpen.value = !trxOpen.value;
                            },
                            child: Card(
                              shape: RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(30)),
                              child: Padding(
                                padding: const EdgeInsets.all(15.0),
                                child: Row(
                                  mainAxisAlignment: MainAxisAlignment.center,
                                  children: [
                                    const Text(
                                      "Transactions",
                                      style: TextStyle(
                                        fontSize: 18,
                                      ),
                                    ),
                                    const SizedBox(
                                      width: 5,
                                    ),
                                    Transform.rotate(
                                      child: const Icon(
                                        Icons.arrow_back_ios_new,
                                        size: 15,
                                      ),
                                      angle: trxOpen.value
                                          ? 90 * pi / 180
                                          : 270 * pi / 180,
                                    )
                                  ],
                                ),
                              ),
                            ),
                          ),
                        ),
                        Obx(() {
                          final listTransactions = <Widget>[];
                          if (tokenTransaction != null &&
                              tokenTransaction.isNotEmpty) {
                            List data = tokenTransaction['trx'] as List;

                            int count = 1;

                            for (final datum in data) {
                              if (datum == null) continue;
                              if (count > maximumTransactionToSave) break;
                              if (datum['from'].toString().toLowerCase() !=
                                  tokenTransaction['currentUser']
                                      .toString()
                                      .toLowerCase()) continue;
                              final tokenSent =
                                  datum['value'] / pow(10, datum['decimal']);
                              DateTime trnDate =
                                  DateFormat("yyyy-MM-dd hh:mm:ss")
                                      .parse(datum['time']);

                              listTransactions.addAll([
                                GestureDetector(
                                  onTap: () async {
                                    final pref = Hive.box(secureStorageKey);
                                    bool hasWallet =
                                        pref.get(currentMmenomicKey) != null;
                                    Widget nextWidget;
                                    if (hasWallet) {
                                      nextWidget = await dappWidget(
                                        context,
                                        widget.data['blockExplorer']
                                            .toString()
                                            .replaceFirst(
                                              transactionhashTemplateKey,
                                              datum['transactionHash'],
                                            ),
                                      );
                                    } else {
                                      nextWidget = Dapp(
                                        provider: '',
                                        init: '',
                                        data: widget.data['blockExplorer']
                                            .toString()
                                            .replaceFirst(
                                              transactionhashTemplateKey,
                                              datum['transactionHash'],
                                            ),
                                      );
                                    }
                                    await Get.to(
                                      nextWidget,
                                      transition: Transition.leftToRight,
                                    );
                                  },
                                  child: Container(
                                    color: Colors.transparent,
                                    child: Column(
                                      crossAxisAlignment:
                                          CrossAxisAlignment.start,
                                      children: [
                                        const SizedBox(
                                          height: 10,
                                        ),
                                        Row(
                                          mainAxisAlignment:
                                              MainAxisAlignment.spaceBetween,
                                          children: [
                                            Flexible(
                                              child: Row(
                                                children: [
                                                  SvgPicture.asset(
                                                      'assets/sent-trans.svg'),
                                                  const SizedBox(
                                                    width: 10,
                                                  ),
                                                  Expanded(
                                                    child: Column(
                                                      crossAxisAlignment:
                                                          CrossAxisAlignment
                                                              .start,
                                                      children: [
                                                        UserBalance(
                                                          balance: tokenSent,
                                                          symbol: '-',
                                                          reversed: true,
                                                          textStyle:
                                                              const TextStyle(
                                                            fontSize: 18,
                                                          ),
                                                        ),
                                                        const SizedBox(
                                                            height: 10),
                                                        Text(
                                                          '${trnDate.day} ${months[trnDate.month - 1]} ${trnDate.year}',
                                                          style:
                                                              const TextStyle(
                                                                  color: Colors
                                                                      .grey),
                                                        )
                                                      ],
                                                    ),
                                                  ),
                                                  Expanded(
                                                    child: Column(
                                                      crossAxisAlignment:
                                                          CrossAxisAlignment
                                                              .end,
                                                      children: [
                                                        const Text('Sent'),
                                                        const SizedBox(
                                                            height: 10),
                                                        Text(
                                                          ellipsify(
                                                            str: datum['to'],
                                                          ),
                                                          overflow:
                                                              TextOverflow.fade,
                                                          style:
                                                              const TextStyle(
                                                            color: Colors.grey,
                                                          ),
                                                        ),
                                                      ],
                                                    ),
                                                  ),
                                                ],
                                              ),
                                            ),
                                          ],
                                        )
                                      ],
                                    ),
                                  ),
                                ),
                                const Divider()
                              ]);
                              count++;
                            }
                          }
                          return (listTransactions.isNotEmpty && trxOpen.value)
                              ? Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: listTransactions,
                                )
                              : Container();
                        })
                      ],
                    ),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
