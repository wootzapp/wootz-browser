import 'dart:async';
import 'dart:math';

import 'package:cached_network_image/cached_network_image.dart';
import 'package:cryptowallet/components/user_balance.dart';
import 'package:cryptowallet/utils/format_money.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import '../components/loader.dart';
import '../config/colors.dart';
import '../config/styles.dart';
import '../utils/app_config.dart';
import '../utils/slide_up_panel.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class ExchangeToken extends StatefulWidget {
  const ExchangeToken({Key key}) : super(key: key);
  @override
  _ExchangeTokenState createState() => _ExchangeTokenState();
}

class _ExchangeTokenState extends State<ExchangeToken>
    with AutomaticKeepAliveClientMixin {
  bool isLoading = false;
  bool tokenPayLoading = false;
  bool tokenGetLoading = false;
  String error = '';
  final amountPay = TextEditingController()..text = '1';
  List networks = getEVMBlockchains().keys.toList();
  String network;
  String currentSelectedTokenPay = 'ETH';
  String currentSelectedTokenGet = 'ETH';
  String currentTokenPayContractAddress = nativeTokenAddress;
  String currentTokenGetContractAddress = nativeTokenAddress;
  Map tokenList;
  Map selectedItemPay = {};
  Map selectedItemGet = {};
  String networkImage;
  Map networkBalanceInfo;
  double tokenToGet_;
  final nativeTokenLCase = nativeTokenAddress.toLowerCase();
  Map selecteditemPayBalanceAndSymbol;
  Map selecteditemGetBalanceAndSymbol;

  bool dataLoading = false;
  Timer timer;

  @override
  void dispose() {
    timer?.cancel();
    amountPay.dispose();
    super.dispose();
  }

  Future getAllExchangeToken() async {
    if (mounted) {
      setState(() {
        dataLoading = true;
      });
    }

    await selectedToPayDetails();
    await selectedToGetDetails();
    await getNetworkBalance();
    await tokenToGetFromNetwork();
    if (mounted) {
      setState(() {
        dataLoading = false;
      });
    }
  }

  @override
  void initState() {
    super.initState();
    network = networks[0];
    getAllExchangeToken();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await getAllExchangeToken(),
    );
    networkImage = getEVMBlockchains().values.toList()[0]['image'];
  }

  Future selectedToGetDetails() async {
    try {
      double cryptoBalance = 0;

      if (selectedItemGet.values.isEmpty) {
        tokenList = await get1InchUrlList(1);
        selectedItemGet = tokenList[nativeTokenLCase];
      }

      if (selectedItemGet['address'].toString().toLowerCase() ==
          nativeTokenLCase) {
        final mnemonic = (Hive.box(secureStorageKey)).get(currentMmenomicKey);

        final getEthereumDetails = await getEthereumFromMemnomic(
          mnemonic,
          getEVMBlockchains()[network]['coinType'],
        );
        cryptoBalance = await getEthereumAddressBalance(
          getEthereumDetails['eth_wallet_address'],
          getEVMBlockchains()[network]['rpc'],
          coinType: getEVMBlockchains()[network]['coinType'],
        );
      } else {
        cryptoBalance = await getERC20TokenBalance({
          'contractAddress': selectedItemGet['address'],
          'rpc': getEVMBlockchains()[network]['rpc'],
          'chainId': getEVMBlockchains()[network]['chainId'],
          'coinType': getEVMBlockchains()[network]['coinType'],
        });
      }

      selecteditemGetBalanceAndSymbol = {
        'balance': cryptoBalance,
        'symbol': selectedItemGet['symbol']
      };
      setState(() {});
    } catch (_) {}
  }

  Future selectedToPayDetails() async {
    try {
      double cryptoBalance = 0;
      final Map networkDetails = getEVMBlockchains()[network];
      if (selectedItemPay.values.isEmpty) {
        tokenList = await get1InchUrlList(1);
        selectedItemPay = tokenList[nativeTokenLCase];
      }

      if (selectedItemPay['address'].toString().toLowerCase() ==
          nativeTokenLCase) {
        final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);

        final getEthereumDetails = await getEthereumFromMemnomic(
          mnemonic,
          networkDetails['coinType'],
        );
        cryptoBalance = await getEthereumAddressBalance(
          getEthereumDetails['eth_wallet_address'],
          networkDetails['rpc'],
          coinType: networkDetails['coinType'],
        );
      } else {
        cryptoBalance = await getERC20TokenBalance({
          'contractAddress': selectedItemPay['address'],
          'rpc': networkDetails['rpc'],
          'chainId': networkDetails['chainId'],
          'coinType': networkDetails['coinType']
        });
      }
      selecteditemPayBalanceAndSymbol = {
        'balance': cryptoBalance,
        'symbol': selectedItemPay['symbol']
      };
      setState(() {});
    } catch (_) {}
  }

  Future swapToken(
    double transactionFee,
    web3.Web3Client client,
    Map response,
    Map approveTransactionOneInch,
    bool isNotApproved,
    Map swapTransactionOneInch,
    web3.Credentials credentials,
  ) async {
    try {
      if (!await authencate(context)) {
        throw Exception(
          AppLocalizations.of(context).authFailed,
        );
      }
      final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);

      final getEthereumDetails = await getEthereumFromMemnomic(
        mnemonic,
        getEVMBlockchains()[network]['coinType'],
      );
      final inSufficientNetworkFees = transactionFee >
          await getEthereumAddressBalance(
            getEthereumDetails['eth_wallet_address'],
            getEVMBlockchains()[network]['rpc'],
            coinType: getEVMBlockchains()[network]['coinType'],
          );

      if (inSufficientNetworkFees) {
        throw Exception(
          AppLocalizations.of(context).insufficientBalance,
        );
      }
      int nonce = await client.getTransactionCount(
        web3.EthereumAddress.fromHex(
          response['eth_wallet_address'],
        ),
      );
      if (selectedItemPay['address'].toString().toLowerCase() !=
              nativeTokenAddress.toLowerCase() &&
          isNotApproved) {
        final approveTransaction = await client.signTransaction(
            credentials,
            web3.Transaction(
              nonce: nonce,
              from:
                  web3.EthereumAddress.fromHex(response['eth_wallet_address']),
              to: web3.EthereumAddress.fromHex(approveTransactionOneInch['to']),
              value: web3.EtherAmount.inWei(BigInt.from(
                  double.parse(approveTransactionOneInch['value']))),
              gasPrice: web3.EtherAmount.inWei(
                BigInt.from(
                  double.parse(approveTransactionOneInch['gasPrice']),
                ),
              ),
              data: txDataToUintList(
                approveTransactionOneInch['data'],
              ),
            ),
            chainId: getEVMBlockchains()[network]['chainId']);

        await client.sendRawTransaction(approveTransaction);

        nonce++;
      }

      final swapTransaction = await client.signTransaction(
        credentials,
        web3.Transaction(
          nonce: nonce,
          from: web3.EthereumAddress.fromHex(
              swapTransactionOneInch['tx']['from']),
          to: web3.EthereumAddress.fromHex(swapTransactionOneInch['tx']['to']),
          value: web3.EtherAmount.inWei(
            BigInt.from(
              double.parse(
                swapTransactionOneInch['tx']['value'],
              ),
            ),
          ),
          gasPrice: web3.EtherAmount.inWei(
            BigInt.from(
              double.parse(
                swapTransactionOneInch['tx']['gasPrice'],
              ),
            ),
          ),
          data: txDataToUintList(swapTransactionOneInch['tx']['data']),
        ),
        chainId: getEVMBlockchains()[network]['chainId'],
      );

      final swapTransactionhash =
          await client.sendRawTransaction(swapTransaction);

      if (Navigator.canPop(context)) {
        Navigator.pop(context);
      }

      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          backgroundColor: Colors.green,
          content: Text(
            '${AppLocalizations.of(context).tokenSwappedSuccessfully} $swapTransactionhash',
            style: const TextStyle(color: Colors.white),
          ),
        ),
      );
    } catch (e) {
      if (Navigator.canPop(context)) {
        Navigator.pop(context);
      }
      if (kDebugMode) {
        print(e);
      }
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          backgroundColor: Colors.red,
          content: Text(
            e.toString(),
            style: const TextStyle(color: Colors.white),
          ),
        ),
      );
    }
  }

  Future tokenToGetFromNetwork() async {
    final Map networkDetails = getEVMBlockchains()[network];
    try {
      if (selectedItemPay.values.isEmpty) {
        tokenList = await get1InchUrlList(1);
        selectedItemPay = selectedItemGet = tokenList[nativeTokenLCase];
      }
      if (selectedItemGet.values.isEmpty) {
        tokenList = await get1InchUrlList(1);
        selectedItemGet = selectedItemGet = tokenList[nativeTokenLCase];
      }

      final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);
      final response = await getEthereumFromMemnomic(
        mnemonic,
        networkDetails['coinType'],
      );

      double amountPaying = double.tryParse(amountPay.text);

      if (amountPaying != null) {
        amountPaying *= pow(10, selectedItemPay['decimals']);

        Map transactionData = await oneInchSwapUrlResponse(
          fromTokenAddress: selectedItemPay['address'],
          toTokenAddress: selectedItemGet['address'],
          amountInWei: amountPaying,
          fromAddress: response['eth_wallet_address'],
          slippage: 0.1,
          chainId: getEVMBlockchains()[network]['chainId'],
        );

        if (transactionData != null) {
          final tokenToGet = double.parse(transactionData['toTokenAmount']) /
              pow(10, transactionData['toToken']['decimals']);

          tokenToGet_ = tokenToGet;
        } else {
          tokenToGet_ = 0;
        }
      }
      setState(() {});
    } catch (e) {
      if (kDebugMode) {
        print(e);
      }
    }
  }

  Future getNetworkBalance() async {
    try {
      final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);

      final getEthereumDetails = await getEthereumFromMemnomic(
        mnemonic,
        getEVMBlockchains()[network]['coinType'],
      );
      final cryptoBalance = await getEthereumAddressBalance(
        getEthereumDetails['eth_wallet_address'],
        getEVMBlockchains()[network]['rpc'],
        coinType: getEVMBlockchains()[network]['coinType'],
      );

      networkBalanceInfo = {
        'balance': cryptoBalance,
        'symbol': getEVMBlockchains()[network]['symbol']
      };
      setState(() {});
    } catch (_) {}
  }

  // scaffold keuy
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  @override
  Widget build(BuildContext context) {
    super.build(context);

    return Scaffold(
      key: _scaffoldKey,
      body: SafeArea(
          child: RefreshIndicator(
        onRefresh: () async {
          await Future.delayed(const Duration(seconds: 2));
          setState(() {});
        },
        child: SingleChildScrollView(
          child: Padding(
            padding: const EdgeInsets.all(20.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const SizedBox(
                  height: 20,
                ),
                Text(
                  AppLocalizations.of(context).swap,
                  style: const TextStyle(
                      fontWeight: FontWeight.bold, fontSize: 24),
                ),
                const SizedBox(
                  height: 20,
                ),
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: [
                    Text(
                      AppLocalizations.of(context).network,
                      style: const TextStyle(
                        fontSize: 20,
                      ),
                    ),
                    GestureDetector(
                      onTap: () {
                        showBlockChainDialog(
                          context: context,
                          onTap: (blockChainData) async {
                            Navigator.pop(context);
                            ScaffoldMessenger.of(context).hideCurrentSnackBar();

                            try {
                              tokenList = await get1InchUrlList(
                                blockChainData['chainId'],
                              );

                              network = blockChainData['name'];
                              networkImage = blockChainData['image'];
                              selectedItemPay =
                                  selectedItemGet = tokenList[nativeTokenLCase];

                              currentSelectedTokenPay =
                                  currentSelectedTokenGet =
                                      getEVMBlockchains()[network]['symbol'];
                              amountPay.text = '0';

                              try {
                                await getAllExchangeToken();
                              } catch (_) {}
                              setState(() {});
                            } catch (e) {
                              ScaffoldMessenger.of(context).showSnackBar(
                                SnackBar(
                                  backgroundColor: Colors.red,
                                  content: Text(
                                    AppLocalizations.of(context)
                                        .networkNotSupported,
                                    style: const TextStyle(color: Colors.white),
                                  ),
                                ),
                              );
                            }
                          },
                          selectedChainId: getEVMBlockchains()[network]
                              ['chainId'],
                        );
                      },
                      child: CircleAvatar(
                        radius: 20,
                        backgroundImage: AssetImage(
                          networkImage ?? '',
                        ),
                      ),
                    ),
                  ],
                ),
                const SizedBox(
                  height: 20,
                ),
                Column(
                  children: [
                    SizedBox(
                      height: 300,
                      child: Stack(
                        clipBehavior: Clip.none,
                        children: <Widget>[
                          Positioned(
                            top: 0,
                            left: 0,
                            child: SizedBox(
                              height: 150,
                              width: MediaQuery.of(context).size.width * .9,
                              child: Card(
                                  child: Center(
                                child: Padding(
                                  padding: const EdgeInsets.only(
                                      top: 15, bottom: 20),
                                  child: Column(
                                    children: [
                                      const SizedBox(
                                        height: 10,
                                      ),
                                      Row(
                                        mainAxisAlignment:
                                            MainAxisAlignment.spaceBetween,
                                        children: [
                                          Padding(
                                            padding:
                                                const EdgeInsets.only(left: 10),
                                            child: Text(
                                                AppLocalizations.of(context)
                                                    .from,
                                                style: s12_18_agSemiboldGrey),
                                          ),
                                        ],
                                      ),
                                      const SizedBox(
                                        height: 10,
                                      ),
                                      Row(
                                          mainAxisAlignment:
                                              MainAxisAlignment.spaceBetween,
                                          children: [
                                            Flexible(
                                              child: Padding(
                                                  padding:
                                                      const EdgeInsets.only(
                                                          left: 10),
                                                  child: SizedBox(
                                                    width: double.infinity,
                                                    child: TextFormField(
                                                      onChanged: (value) async {
                                                        await getAllExchangeToken();
                                                        setState(() {});
                                                      },
                                                      keyboardType:
                                                          const TextInputType
                                                              .numberWithOptions(
                                                        decimal: true,
                                                      ),
                                                      style: h5,
                                                      decoration:
                                                          const InputDecoration(
                                                        isDense: true,
                                                        isCollapsed: true,
                                                        border:
                                                            InputBorder.none,
                                                      ),
                                                      controller: amountPay,
                                                    ),
                                                  )),
                                            ),
                                            GestureDetector(
                                              onTap: () async {
                                                FocusManager
                                                    .instance.primaryFocus
                                                    ?.unfocus();

                                                if (tokenPayLoading) return;
                                                tokenPayLoading = true;
                                                try {
                                                  tokenList ??=
                                                      await get1InchUrlList(1);

                                                  buildSwapUi(
                                                    tokenList: tokenList.values
                                                        .toList(),
                                                    context: context,
                                                    onSelect: (value) async {
                                                      setState(() {
                                                        selectedItemPay = value;
                                                      });
                                                      await getAllExchangeToken();
                                                    },
                                                  );
                                                } catch (_) {}
                                                tokenPayLoading = false;
                                              },
                                              child: Container(
                                                color: Colors.transparent,
                                                child: Row(
                                                  children: [
                                                    CachedNetworkImage(
                                                      imageUrl: ipfsTohttp(
                                                        selectedItemPay[
                                                                'logoURI'] ??
                                                            'https://tokens.1inch.io/$nativeTokenLCase.png',
                                                      ),
                                                      width: 30,
                                                      height: 30,
                                                      placeholder:
                                                          (context, url) =>
                                                              Column(
                                                        mainAxisAlignment:
                                                            MainAxisAlignment
                                                                .center,
                                                        mainAxisSize:
                                                            MainAxisSize.min,
                                                        children: const [
                                                          SizedBox(
                                                            width: 20,
                                                            height: 20,
                                                            child: Loader(
                                                              color:
                                                                  appPrimaryColor,
                                                            ),
                                                          )
                                                        ],
                                                      ),
                                                      errorWidget: (context,
                                                              url, error) =>
                                                          const Icon(
                                                        Icons.error,
                                                        color: Colors.red,
                                                      ),
                                                    ),
                                                    const SizedBox(
                                                      width: 5,
                                                    ),
                                                    Text(
                                                      ellipsify(
                                                        str: selectedItemPay[
                                                                'symbol'] ??
                                                            'ETH',
                                                      ),
                                                      style: m_agRegular,
                                                    ),
                                                    const SizedBox(
                                                      width: 5,
                                                    ),
                                                    const Icon(
                                                      Icons.arrow_forward_ios,
                                                    ),
                                                  ],
                                                ),
                                              ),
                                            )
                                          ]),
                                      Row(
                                        children: [
                                          Padding(
                                            padding:
                                                const EdgeInsets.only(left: 10),
                                            child: Row(
                                              children: [
                                                Text(
                                                  '${AppLocalizations.of(context).balance}: ',
                                                  style: m_agRegular_grey,
                                                ),
                                                if (selecteditemPayBalanceAndSymbol !=
                                                    null)
                                                  UserBalance(
                                                    symbol:
                                                        selecteditemPayBalanceAndSymbol[
                                                            'symbol'],
                                                    balance:
                                                        selecteditemPayBalanceAndSymbol[
                                                            'balance'],
                                                    textStyle: m_agRegular_grey,
                                                  )
                                                else
                                                  Container()
                                              ],
                                            ),
                                          ),
                                        ],
                                      )
                                    ],
                                  ),
                                ),
                              )),
                            ),
                          ),
                          Positioned(
                            bottom: 0,
                            left: 0,
                            child: SizedBox(
                              height: 150,
                              width: MediaQuery.of(context).size.width * .9,
                              child: Card(
                                child: Center(
                                  child: Padding(
                                    padding: const EdgeInsets.only(
                                        top: 20, bottom: 20),
                                    child: Column(
                                      children: [
                                        const SizedBox(
                                          height: 10,
                                        ),
                                        Row(
                                          mainAxisAlignment:
                                              MainAxisAlignment.spaceBetween,
                                          children: [
                                            Padding(
                                              padding: const EdgeInsets.only(
                                                  left: 10),
                                              child: Text(
                                                  AppLocalizations.of(context)
                                                      .to,
                                                  style: s12_18_agSemiboldGrey),
                                            ),
                                            const Padding(
                                              padding:
                                                  EdgeInsets.only(right: 10),
                                              child: Text('', style: s_normal),
                                            ),
                                          ],
                                        ),
                                        const SizedBox(
                                          height: 10,
                                        ),
                                        Row(
                                            mainAxisAlignment:
                                                MainAxisAlignment.spaceBetween,
                                            children: [
                                              Flexible(
                                                child: Padding(
                                                  padding:
                                                      const EdgeInsets.only(
                                                          left: 10),
                                                  child: tokenToGet_ != null
                                                      ? Text(
                                                          '${formatMoney(tokenToGet_)}',
                                                          style: h5)
                                                      : const SizedBox(
                                                          width: 25,
                                                          height: 25,
                                                          child: Loader(
                                                            color:
                                                                appPrimaryColor,
                                                          ),
                                                        ),
                                                ),
                                              ),
                                              GestureDetector(
                                                onTap: () async {
                                                  FocusManager
                                                      .instance.primaryFocus
                                                      ?.unfocus();
                                                  if (tokenGetLoading) return;
                                                  tokenGetLoading = true;
                                                  try {
                                                    tokenList ??=
                                                        await get1InchUrlList(
                                                            1);

                                                    buildSwapUi(
                                                      tokenList: tokenList
                                                          .values
                                                          .toList(),
                                                      context: context,
                                                      onSelect: (value) async {
                                                        setState(() {
                                                          selectedItemGet =
                                                              value;
                                                        });
                                                        await getAllExchangeToken();
                                                      },
                                                    );
                                                  } catch (_) {}
                                                  tokenGetLoading = false;
                                                },
                                                child: Container(
                                                  color: Colors.transparent,
                                                  child: Row(
                                                    children: [
                                                      CachedNetworkImage(
                                                        imageUrl: ipfsTohttp(
                                                          selectedItemGet[
                                                                  'logoURI'] ??
                                                              'https://tokens.1inch.io/$nativeTokenLCase.png',
                                                        ),
                                                        placeholder:
                                                            (context, url) =>
                                                                Column(
                                                          mainAxisAlignment:
                                                              MainAxisAlignment
                                                                  .center,
                                                          mainAxisSize:
                                                              MainAxisSize.min,
                                                          children: const [
                                                            SizedBox(
                                                              width: 20,
                                                              height: 20,
                                                              child: Loader(
                                                                color:
                                                                    appPrimaryColor,
                                                              ),
                                                            )
                                                          ],
                                                        ),
                                                        errorWidget: (context,
                                                                url, error) =>
                                                            const Icon(
                                                          Icons.error,
                                                          color: Colors.red,
                                                        ),
                                                        width: 30,
                                                        height: 30,
                                                      ),
                                                      const SizedBox(
                                                        width: 5,
                                                      ),
                                                      Text(
                                                        ellipsify(
                                                          str: selectedItemGet[
                                                                  'symbol'] ??
                                                              'ETH',
                                                        ),
                                                        style: m_agRegular,
                                                      ),
                                                      const SizedBox(
                                                        width: 5,
                                                      ),
                                                      const Icon(
                                                        Icons.arrow_forward_ios,
                                                      ),
                                                    ],
                                                  ),
                                                ),
                                              )
                                            ]),
                                        Row(
                                          children: [
                                            Padding(
                                              padding: const EdgeInsets.only(
                                                  left: 10),
                                              child: Row(
                                                children: [
                                                  Text(
                                                    '${AppLocalizations.of(context).balance}: ',
                                                    style: m_agRegular_grey,
                                                  ),
                                                  if (selecteditemGetBalanceAndSymbol !=
                                                      null)
                                                    UserBalance(
                                                      symbol:
                                                          selecteditemGetBalanceAndSymbol[
                                                              'symbol'],
                                                      balance:
                                                          selecteditemGetBalanceAndSymbol[
                                                              'balance'],
                                                      textStyle:
                                                          m_agRegular_grey,
                                                    )
                                                  else
                                                    Container()
                                                ],
                                              ),
                                            ),
                                          ],
                                        )
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                            ),
                          ),
                          Positioned(
                            bottom: 0,
                            top: 0,
                            right: 30,
                            child: SizedBox(
                              width: 35,
                              height: 35,
                              child: GestureDetector(
                                onTap: () async {
                                  if (selectedItemGet.isNotEmpty &&
                                      selectedItemPay.isNotEmpty) {
                                    setState(() {
                                      final selectedToGet_ = selectedItemGet;
                                      selectedItemGet = selectedItemPay;
                                      selectedItemPay = selectedToGet_;
                                    });
                                  }
                                  await getAllExchangeToken();
                                },
                                child: Container(
                                  decoration: const BoxDecoration(
                                    color: appPrimaryColor,
                                    shape: BoxShape.circle,
                                  ),
                                  child: Transform.rotate(
                                    angle: 90 / (180 / pi),
                                    child: const Icon(
                                      FontAwesomeIcons.exchangeAlt,
                                      color: Colors.white,
                                      size: 15,
                                    ),
                                  ),
                                ),
                              ),
                            ),
                          ),
                        ],
                      ),
                    ),
                    const SizedBox(
                      height: 20,
                    ),
                    if (networkBalanceInfo != null)
                      Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Loader(
                            size: 15,
                            color: dataLoading ? null : Colors.transparent,
                          ),
                          const SizedBox(
                            width: 10,
                          ),
                          Text(
                            '${AppLocalizations.of(context).networkBalance}: ',
                            style: const TextStyle(color: Colors.grey),
                          ),
                          UserBalance(
                            balance: networkBalanceInfo['balance'],
                            symbol: networkBalanceInfo['symbol'],
                            textStyle: const TextStyle(color: Colors.grey),
                          ),
                          const Loader(
                            size: 15,
                            color: Colors.transparent,
                          ),
                          const SizedBox(
                            width: 10,
                          ),
                        ],
                      )
                    else
                      Container(),
                    const SizedBox(
                      height: 20,
                    ),
                    SizedBox(
                      width: double.infinity,
                      child: ElevatedButton(
                        style: ButtonStyle(
                          backgroundColor: MaterialStateProperty.resolveWith(
                              (states) => appBackgroundblue),
                          shape: MaterialStateProperty.resolveWith(
                            (states) => RoundedRectangleBorder(
                              borderRadius: BorderRadius.circular(10),
                            ),
                          ),
                        ),
                        onPressed: () async {
                          FocusManager.instance.primaryFocus?.unfocus();
                          bool isSwaping = false;

                          try {
                            setState(() {
                              isLoading = true;
                            });

                            ScaffoldMessenger.of(context).hideCurrentSnackBar();
                            final mnemonic = Hive.box(secureStorageKey)
                                .get(currentMmenomicKey);
                            final response = await getEthereumFromMemnomic(
                              mnemonic,
                              getEVMBlockchains()[network]['coinType'],
                            );
                            final client = web3.Web3Client(
                              getEVMBlockchains()[network]['rpc'],
                              Client(),
                            );

                            final credentials = web3.EthPrivateKey.fromHex(
                              response['eth_wallet_privateKey'],
                            );

                            if (selectedItemPay.values.isEmpty) {
                              tokenList = await get1InchUrlList(1);
                              selectedItemPay =
                                  selectedItemGet = tokenList[nativeTokenLCase];
                            }
                            if (selectedItemGet.values.isEmpty) {
                              tokenList = await get1InchUrlList(1);
                              selectedItemGet =
                                  selectedItemGet = tokenList[nativeTokenLCase];
                            }

                            if (double.tryParse(amountPay.text) == null ||
                                double.tryParse(amountPay.text) == 0) {
                              ScaffoldMessenger.of(context).showSnackBar(
                                SnackBar(
                                  backgroundColor: Colors.red,
                                  content: Text(
                                    AppLocalizations.of(context)
                                        .pleaseEnterAmount,
                                    style: const TextStyle(
                                      color: Colors.white,
                                    ),
                                  ),
                                ),
                              );
                              setState(() {
                                isLoading = false;
                              });
                              return;
                            }

                            if (selectedItemPay['address'] ==
                                selectedItemGet['address']) {
                              ScaffoldMessenger.of(context).showSnackBar(
                                SnackBar(
                                  backgroundColor: Colors.red,
                                  content: Text(
                                    AppLocalizations.of(context)
                                        .pleaseSelectDifferentTokens,
                                    style: const TextStyle(
                                      color: Colors.white,
                                    ),
                                  ),
                                ),
                              );
                              setState(() {
                                isLoading = false;
                              });
                              return;
                            }

                            double transactionFee = 0.0;
                            Map swapTransactionOneInch =
                                await oneInchSwapUrlResponse(
                              fromTokenAddress: selectedItemPay['address'],
                              toTokenAddress: selectedItemGet['address'],
                              amountInWei: (double.tryParse(amountPay.text) *
                                  pow(10, selectedItemPay['decimals'])),
                              fromAddress: response['eth_wallet_address'],
                              slippage: 0.1,
                              chainId: getEVMBlockchains()[network]['chainId'],
                            );

                            if (swapTransactionOneInch == null) {
                              throw Exception('could not get liquidity');
                            }

                            final fromTokenAmount = double.parse(
                                    swapTransactionOneInch['fromTokenAmount']) /
                                pow(
                                  10,
                                  swapTransactionOneInch['fromToken']
                                      ['decimals'],
                                );
                            final toTokenAmount = double.parse(
                                    swapTransactionOneInch['toTokenAmount']) /
                                pow(
                                    10,
                                    swapTransactionOneInch['toToken']
                                        ['decimals']);

                            transactionFee += await getEtherTransactionFee(
                              getEVMBlockchains()[network]['rpc'],
                              txDataToUintList(
                                  swapTransactionOneInch['tx']['data']),
                              web3.EthereumAddress.fromHex(
                                  swapTransactionOneInch['tx']['from']),
                              web3.EthereumAddress.fromHex(
                                  swapTransactionOneInch['tx']['to']),
                              value: double.parse(
                                  swapTransactionOneInch['tx']['value']),
                              gasPrice: web3.EtherAmount.inWei(
                                BigInt.from(
                                  double.parse(
                                      swapTransactionOneInch['tx']['gasPrice']),
                                ),
                              ),
                            );

                            Map approveTransactionOneInch;
                            bool isNotApproved = true;

                            if (selectedItemPay['address']
                                    .toString()
                                    .toLowerCase() !=
                                nativeTokenLCase) {
                              final userTokenBalance =
                                  await getERC20TokenBalance({
                                'contractAddress': selectedItemPay['address'],
                                'rpc': getEVMBlockchains()[network]['rpc'],
                                'chainId': getEVMBlockchains()[network]
                                    ['chainId'],
                                'coinType': getEVMBlockchains()[network]
                                    ['coinType'],
                              });

                              if (userTokenBalance <
                                  (double.tryParse(amountPay.text) ?? 0)) {
                                throw Exception('Insufficient funds');
                              }

                              final allowance = await getErc20Allowance(
                                owner: response['eth_wallet_address'],
                                rpc: getEVMBlockchains()[network]['rpc'],
                                contractAddress: selectedItemPay['address'],
                                spender: swapTransactionOneInch['tx']['to'],
                              );

                              if (allowance <
                                  BigInt.from(((double.tryParse(
                                              amountPay.text) ??
                                          0) *
                                      pow(10, selectedItemPay['decimals'])))) {
                                approveTransactionOneInch =
                                    await approveTokenFor1inch(
                                  getEVMBlockchains()[network]['chainId'],
                                  (double.tryParse(amountPay.text) *
                                      pow(10, selectedItemPay['decimals'])),
                                  selectedItemPay['address'],
                                );

                                transactionFee += await getEtherTransactionFee(
                                  getEVMBlockchains()[network]['rpc'],
                                  txDataToUintList(
                                      approveTransactionOneInch['data']),
                                  web3.EthereumAddress.fromHex(
                                      response['eth_wallet_address']),
                                  web3.EthereumAddress.fromHex(
                                      approveTransactionOneInch['to']),
                                );
                              } else {
                                isNotApproved = false;
                              }
                            }

                            transactionFee /= pow(10, etherDecimals);

                            setState(() {
                              isLoading = false;
                            });

                            await slideUpPanel(
                              context,
                              StatefulBuilder(builder: (ctx, setState) {
                                return Padding(
                                  padding: const EdgeInsets.all(25),
                                  child: Column(
                                    mainAxisSize: MainAxisSize.min,
                                    children: [
                                      Text(
                                        AppLocalizations.of(context).swap,
                                        style: const TextStyle(
                                          fontSize: 20,
                                          fontWeight: FontWeight.bold,
                                        ),
                                      ),
                                      const SizedBox(
                                        height: 20,
                                      ),
                                      Row(
                                        children: [
                                          CachedNetworkImage(
                                            imageUrl: ipfsTohttp(
                                                selectedItemPay['logoURI']),
                                            width: 30,
                                            height: 30,
                                            placeholder: (context, url) =>
                                                Column(
                                              mainAxisAlignment:
                                                  MainAxisAlignment.center,
                                              mainAxisSize: MainAxisSize.min,
                                              children: const [
                                                SizedBox(
                                                  width: 20,
                                                  height: 20,
                                                  child: Loader(
                                                    color: appPrimaryColor,
                                                  ),
                                                )
                                              ],
                                            ),
                                            errorWidget:
                                                (context, url, error) =>
                                                    const Icon(
                                              Icons.error,
                                              color: Colors.red,
                                            ),
                                          ),
                                          const SizedBox(
                                            width: 10,
                                          ),
                                          Text(
                                            '${formatMoney(
                                              fromTokenAmount,
                                            )} ${ellipsify(str: selectedItemPay['symbol'])}',
                                            overflow: TextOverflow.fade,
                                            style: const TextStyle(
                                                fontWeight: FontWeight.bold,
                                                fontSize: 18),
                                          ),
                                        ],
                                      ),

                                      const SizedBox(
                                        height: 20,
                                      ),
                                      Align(
                                        alignment: Alignment.centerLeft,
                                        child: Text(
                                          AppLocalizations.of(context).for_,
                                          style: const TextStyle(
                                            fontSize: 20,
                                            color: Colors.grey,
                                            fontWeight: FontWeight.bold,
                                          ),
                                        ),
                                      ),
                                      const SizedBox(
                                        height: 20,
                                      ),
                                      Row(
                                        children: [
                                          CachedNetworkImage(
                                            imageUrl: ipfsTohttp(
                                                selectedItemGet['logoURI']),
                                            width: 30,
                                            height: 30,
                                            placeholder: (context, url) =>
                                                Column(
                                              mainAxisAlignment:
                                                  MainAxisAlignment.center,
                                              mainAxisSize: MainAxisSize.min,
                                              children: const [
                                                SizedBox(
                                                  width: 20,
                                                  height: 20,
                                                  child: Loader(
                                                    color: appPrimaryColor,
                                                  ),
                                                )
                                              ],
                                            ),
                                            errorWidget:
                                                (context, url, error) =>
                                                    const Icon(
                                              Icons.error,
                                              color: Colors.red,
                                            ),
                                          ),
                                          const SizedBox(
                                            width: 10,
                                          ),
                                          Text(
                                            '${formatMoney(
                                              toTokenAmount,
                                            )} ${ellipsify(str: selectedItemGet['symbol'])}',
                                            overflow: TextOverflow.fade,
                                            style: const TextStyle(
                                                fontWeight: FontWeight.bold,
                                                fontSize: 18),
                                          ),
                                        ],
                                      ),
                                      const SizedBox(
                                        height: 20,
                                      ),
                                      // transaction fee
                                      Flexible(
                                        child: Row(
                                          children: [
                                            Flexible(
                                              child: Text(
                                                '${AppLocalizations.of(context).transactionFee}: ',
                                                style: const TextStyle(
                                                  fontWeight: FontWeight.bold,
                                                  color: Colors.grey,
                                                  fontSize: 16,
                                                ),
                                              ),
                                            ),
                                            const SizedBox(
                                              width: 5,
                                            ),
                                            Flexible(
                                              child: Text(
                                                formatMoney(transactionFee
                                                        .toString()) +
                                                    ' ' +
                                                    getEVMBlockchains()[network]
                                                        ['symbol'],
                                                // bold
                                                style: const TextStyle(
                                                    fontWeight: FontWeight.bold,
                                                    color: Colors.grey,
                                                    fontSize: 16),
                                              ),
                                            ),
                                          ],
                                        ),
                                      ),
                                      const SizedBox(
                                        height: 20,
                                      ),

                                      Container(
                                          color: Colors.transparent,
                                          width: double.infinity,
                                          child: ElevatedButton(
                                            onPressed: () async {
                                              setState(() {
                                                isSwaping = true;
                                              });
                                              await swapToken(
                                                transactionFee,
                                                client,
                                                response,
                                                approveTransactionOneInch,
                                                isNotApproved,
                                                swapTransactionOneInch,
                                                credentials,
                                              );
                                              setState(() {
                                                isSwaping = false;
                                              });
                                            },
                                            child: isSwaping
                                                ? const Loader(
                                                    color: Colors.white,
                                                  )
                                                : Text(
                                                    AppLocalizations.of(context)
                                                        .confirm,
                                                    style: const TextStyle(
                                                        color: Colors.white,
                                                        fontWeight:
                                                            FontWeight.bold),
                                                  ),
                                            style: ElevatedButton.styleFrom(
                                              backgroundColor:
                                                  appBackgroundblue,
                                              padding: const EdgeInsets.all(15),
                                              shape: RoundedRectangleBorder(
                                                borderRadius:
                                                    BorderRadius.circular(
                                                        10), // <-- Radius
                                              ),
                                            ),
                                          )),
                                    ],
                                  ),
                                );
                              }),
                            );
                          } catch (e) {
                            if (kDebugMode) {
                              print(e);
                            }
                            ScaffoldMessenger.of(context).showSnackBar(
                              SnackBar(
                                backgroundColor: Colors.red,
                                content: Text(
                                  e.toString(),
                                  style: const TextStyle(color: Colors.white),
                                ),
                              ),
                            );

                            setState(() {
                              isLoading = false;
                              isSwaping = false;
                            });
                          }
                        },
                        child: isLoading
                            ? const Padding(
                                padding: EdgeInsets.all(8.0),
                                child: Loader(color: white),
                              )
                            : Padding(
                                padding: const EdgeInsets.all(15),
                                child: Text(
                                  AppLocalizations.of(context).swap,
                                  style: const TextStyle(
                                    fontWeight: FontWeight.bold,
                                    color: Colors.white,
                                  ),
                                ),
                              ),
                      ),
                    ),
                  ],
                )
              ],
            ),
          ),
        ),
      )),
    );
  }

  @override
  bool get wantKeepAlive => true;
}
