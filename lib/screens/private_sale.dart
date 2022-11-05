import 'dart:async';
import 'dart:math';

import 'package:cryptowallet/utils/format_money.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:web3dart/web3dart.dart';
import '../components/loader.dart';
import '../components/user_balance.dart';
import '../config/colors.dart';
import '../config/styles.dart';
import '../utils/app_config.dart';
import '../utils/slide_up_panel.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class PrivateSale extends StatefulWidget {
  const PrivateSale({Key key}) : super(key: key);
  @override
  _PrivateSaleState createState() => _PrivateSaleState();
}

class _PrivateSaleState extends State<PrivateSale>
    with AutomaticKeepAliveClientMixin {
  bool isLoading = false;
  String error = '';
  final etherAmountController = TextEditingController()..text = '1';
  final scaffoldKey = GlobalKey<ScaffoldState>();
  Map privateSaleDetails;
  double networkBalance;
  double tokenBalance;

  Timer timer;

  @override
  void initState() {
    super.initState();
    callAllApi();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await callAllApi(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    etherAmountController.dispose();
    super.dispose();
  }

  Future callAllApi() async {
    await getPrivateSaleDetails();
    await getEthereumBalance();
    await getWalletTokenBalance();
  }

  Future getEthereumBalance() async {
    try {
      final mnemonic = (Hive.box(secureStorageKey)).get(currentMmenomicKey);
      final getEthereumDetails = await getEthereumFromMemnomic(
        mnemonic,
        getEVMBlockchains()[tokenSaleContractNetwork]['coinType'],
      );
      final cryptoBalance = await getEthereumAddressBalance(
        getEthereumDetails['eth_wallet_address'],
        getEVMBlockchains()[tokenSaleContractNetwork]['rpc'],
        coinType: getEVMBlockchains()[tokenSaleContractNetwork]['coinType'],
      );
      networkBalance = cryptoBalance;
      if (mounted) {
        setState(() {});
      }
    } catch (_) {}
  }

  Future getWalletTokenBalance() async {
    try {
      final getTokenBalance = await getERC20TokenBalance({
        'contractAddress': tokenContractAddress,
        'rpc': getEVMBlockchains()[tokenContractNetwork]['rpc'],
        'chainId': getEVMBlockchains()[tokenContractNetwork]['chainId'],
        'coinType': getEVMBlockchains()[tokenContractNetwork]['coinType'],
      });
      tokenBalance = getTokenBalance;
      if (mounted) {
        setState(() {});
      }
    } catch (_) {}
  }

  Future getPrivateSaleDetails() async {
    try {
      final client = web3.Web3Client(
        getEVMBlockchains()[tokenSaleContractNetwork]['rpc'],
        Client(),
      );
      final tokenSaleContract = web3.DeployedContract(
        web3.ContractAbi.fromJson(tokenSaleAbi, ''),
        web3.EthereumAddress.fromHex(tokenSaleContractAddress),
      );

      final tokenPriceFunction = tokenSaleContract.function('viewSale');

      final tokenPrice = await client.call(
          contract: tokenSaleContract,
          function: tokenPriceFunction,
          params: []);

      double tokenPriceDouble =
          pow(10, etherDecimals) / double.parse(tokenPrice[3].toString());

      Map tokenDetails = await getERC20TokenNameSymbolDecimal(
        contractAddress: tokenContractAddress,
        rpc: getEVMBlockchains()[tokenContractNetwork]['rpc'],
      );

      privateSaleDetails = {
        'tokenPrice': tokenPriceDouble,
        'appTokenSymbol': tokenDetails['symbol'],
        'success': true
      };
    } catch (e) {
      privateSaleDetails = {
        'success': false,
      };
    }
    if (mounted) {
      setState(() {});
    }
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    if (kDebugMode) {
      print(privateSaleDetails);
    }
    return Scaffold(
      key: scaffoldKey,
      appBar: AppBar(
        title: Text(AppLocalizations.of(context).privateSale),
      ),
      body: SafeArea(
        child: RefreshIndicator(
          onRefresh: () async {
            await Future.delayed(const Duration(seconds: 2));
            setState(() {});
          },
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Padding(
              padding: const EdgeInsets.all(20.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  if (privateSaleDetails != null &&
                      !privateSaleDetails['success'])
                    SizedBox(
                      height: MediaQuery.of(context).size.height * .8,
                      child: Center(
                        child: Text(
                          AppLocalizations.of(context).couldNotFetchData,
                          style: const TextStyle(fontSize: 18),
                        ),
                      ),
                    )
                  else if (privateSaleDetails != null &&
                      privateSaleDetails['success'])
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
                                                padding: const EdgeInsets.only(
                                                    left: 10),
                                                child: Text(
                                                    AppLocalizations.of(context)
                                                        .from,
                                                    style:
                                                        s12_18_agSemiboldGrey),
                                              ),
                                            ],
                                          ),
                                          const SizedBox(
                                            height: 10,
                                          ),
                                          Row(
                                              mainAxisAlignment:
                                                  MainAxisAlignment
                                                      .spaceBetween,
                                              children: [
                                                Flexible(
                                                  child: Padding(
                                                      padding:
                                                          const EdgeInsets.only(
                                                              left: 10),
                                                      child: SizedBox(
                                                        width: double.infinity,
                                                        child: TextFormField(
                                                          onChanged: (value) {
                                                            setState(() {});
                                                          },
                                                          keyboardType:
                                                              const TextInputType
                                                                      .numberWithOptions(
                                                                  decimal:
                                                                      true),
                                                          style: h5,
                                                          decoration:
                                                              const InputDecoration(
                                                                  isDense: true,
                                                                  isCollapsed:
                                                                      true,
                                                                  border:
                                                                      InputBorder
                                                                          .none),
                                                          controller:
                                                              etherAmountController,
                                                        ),
                                                      )),
                                                ),
                                                Row(
                                                  children: [
                                                    const SizedBox(
                                                      width: 10,
                                                    ),
                                                    CircleAvatar(
                                                      backgroundImage: AssetImage(
                                                          getEVMBlockchains()[
                                                                  tokenSaleContractNetwork]
                                                              ['image']),
                                                      radius: 15,
                                                    ),
                                                    const SizedBox(
                                                      width: 10,
                                                    ),
                                                    Text(
                                                        getEVMBlockchains()[
                                                                tokenSaleContractNetwork]
                                                            ['symbol'],
                                                        style: m_agRegular),
                                                    const SizedBox(
                                                      width: 10,
                                                    ),
                                                  ],
                                                )
                                              ]),
                                          Row(
                                            children: [
                                              Flexible(
                                                child: Padding(
                                                  padding:
                                                      const EdgeInsets.only(
                                                          left: 10),
                                                  child: Row(
                                                    children: [
                                                      Text(
                                                        '${AppLocalizations.of(context).balance}: ',
                                                        style: m_agRegular_grey,
                                                      ),
                                                      if (networkBalance !=
                                                          null)
                                                        UserBalance(
                                                          symbol: getEVMBlockchains()[
                                                                  tokenSaleContractNetwork]
                                                              ['symbol'],
                                                          balance:
                                                              networkBalance,
                                                          textStyle:
                                                              m_agRegular_grey,
                                                        )
                                                    ],
                                                  ),
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
                                                  MainAxisAlignment
                                                      .spaceBetween,
                                              children: [
                                                Padding(
                                                  padding:
                                                      const EdgeInsets.only(
                                                          left: 10),
                                                  child: Text(
                                                      AppLocalizations.of(
                                                              context)
                                                          .to,
                                                      style:
                                                          s12_18_agSemiboldGrey),
                                                ),
                                                const Padding(
                                                  padding: EdgeInsets.only(
                                                      right: 10),
                                                  child:
                                                      Text('', style: s_normal),
                                                ),
                                              ],
                                            ),
                                            const SizedBox(
                                              height: 10,
                                            ),
                                            Row(
                                                mainAxisAlignment:
                                                    MainAxisAlignment
                                                        .spaceBetween,
                                                children: [
                                                  Flexible(
                                                    child: Padding(
                                                      padding:
                                                          const EdgeInsets.only(
                                                              left: 10),
                                                      child: Text(
                                                          '${formatMoney(privateSaleDetails['tokenPrice'] * (double.tryParse(etherAmountController.text) ?? 0))}',
                                                          style: h5),
                                                    ),
                                                  ),
                                                  Row(
                                                    children: [
                                                      const CircleAvatar(
                                                        backgroundImage:
                                                            AssetImage(
                                                                'assets/logo.png'),
                                                        radius: 15,
                                                      ),
                                                      const SizedBox(
                                                        width: 10,
                                                      ),
                                                      Text(
                                                          privateSaleDetails[
                                                              'appTokenSymbol'],
                                                          style: m_agRegular),
                                                      const SizedBox(
                                                        width: 10,
                                                      ),
                                                    ],
                                                  )
                                                ]),
                                            Row(
                                              children: [
                                                Padding(
                                                  padding:
                                                      const EdgeInsets.only(
                                                          left: 10),
                                                  child: Row(
                                                    children: [
                                                      Text(
                                                        '${AppLocalizations.of(context).balance}: ',
                                                        style: m_agRegular_grey,
                                                      ),
                                                      if (tokenBalance != null)
                                                        UserBalance(
                                                          symbol: privateSaleDetails[
                                                              'appTokenSymbol'],
                                                          balance: tokenBalance,
                                                          textStyle:
                                                              m_agRegular_grey,
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
                            ],
                          ),
                        ),
                        const SizedBox(
                          height: 20,
                        ),
                        Text(
                          '1 ${getEVMBlockchains()[tokenSaleContractNetwork]['symbol']} = ${privateSaleDetails['tokenPrice']} ${privateSaleDetails['appTokenSymbol']}',
                          style: const TextStyle(color: Colors.grey),
                        ),
                        const SizedBox(
                          height: 20,
                        ),
                        SizedBox(
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
                            onPressed: () async {
                              // remove focus
                              if ((double.tryParse(
                                          etherAmountController.text.trim()) ??
                                      0) <
                                  minimumNetworkTokenForSwap) {
                                ScaffoldMessenger.of(context).showSnackBar(
                                  SnackBar(
                                    backgroundColor: Colors.red,
                                    content: Text(
                                      '${AppLocalizations.of(context).minimumAmountIs} $minimumNetworkTokenForSwap ${getEVMBlockchains()[tokenSaleContractNetwork]['symbol']}',
                                      style: const TextStyle(
                                        color: Colors.white,
                                      ),
                                    ),
                                  ),
                                );
                                return;
                              }

                              setState(() {
                                isLoading = true;
                              });

                              FocusManager.instance.primaryFocus?.unfocus();
                              ScaffoldMessenger.of(context)
                                  .hideCurrentSnackBar();
                              String transactionHash;
                              bool purchasedSuccessfully = true;
                              try {
                                final client = web3.Web3Client(
                                  getEVMBlockchains()[tokenSaleContractNetwork]
                                      ['rpc'],
                                  Client(),
                                );
                                final mnemonic = Hive.box(secureStorageKey)
                                    .get(currentMmenomicKey);
                                final response = await getEthereumFromMemnomic(
                                  mnemonic,
                                  getEVMBlockchains()[tokenSaleContractNetwork]
                                      ['coinType'],
                                );

                                final credentials = EthPrivateKey.fromHex(
                                  response['eth_wallet_privateKey'],
                                );

                                final tokenSaleContract = web3.DeployedContract(
                                    web3.ContractAbi.fromJson(
                                        tokenSaleAbi, walletName),
                                    web3.EthereumAddress.fromHex(
                                        tokenSaleContractAddress));

                                final tokenSale = tokenSaleContract.function(
                                  'tokenSale',
                                );
                                final trans = await client.signTransaction(
                                    credentials,
                                    Transaction.callContract(
                                      contract: tokenSaleContract,
                                      function: tokenSale,
                                      parameters: [
                                        EthereumAddress.fromHex(zeroAddress)
                                      ],
                                      value: web3.EtherAmount.inWei(BigInt.from(
                                          (double.tryParse(etherAmountController
                                                      .text
                                                      .trim()) ??
                                                  0) *
                                              pow(10, etherDecimals))),
                                    ),
                                    chainId: getEVMBlockchains()[
                                        tokenSaleContractNetwork]['chainId']);

                                transactionHash =
                                    await client.sendRawTransaction(trans);
                              } catch (e) {
                                purchasedSuccessfully = false;
                                setState(() {
                                  isLoading = false;
                                });
                                ScaffoldMessenger.of(context).showSnackBar(
                                  SnackBar(
                                    backgroundColor: Colors.red,
                                    content: Text(
                                      e.toString(),
                                      style: const TextStyle(
                                        color: Colors.white,
                                      ),
                                    ),
                                  ),
                                );
                                return;
                              }
                              setState(() {
                                isLoading = false;
                              });

                              await slideUpPanel(context,
                                  StatefulBuilder(builder: (ctx, setState) {
                                String privateSaleBscScan = getEVMBlockchains()[
                                            tokenSaleContractNetwork]
                                        ['blockExplorer']
                                    .toString()
                                    .replaceFirst(
                                      transactionhashTemplateKey,
                                      transactionHash,
                                    );

                                return Padding(
                                  padding: const EdgeInsets.all(20),
                                  child: Column(
                                    mainAxisSize: MainAxisSize.min,
                                    crossAxisAlignment:
                                        CrossAxisAlignment.center,
                                    children: [
                                      purchasedSuccessfully
                                          ? Image.asset(
                                              'assets/images/successIcon.png',
                                              scale: 10,
                                            )
                                          : Image.asset(
                                              'assets/images/failedIcon.png',
                                              scale: 10,
                                            ),
                                      Padding(
                                        padding: const EdgeInsets.all(30),
                                        child: Text(
                                          purchasedSuccessfully
                                              ? AppLocalizations.of(context)
                                                  .privateSaleSuccessful
                                              : AppLocalizations.of(context)
                                                  .privateSaleFailed,
                                          style: title1,
                                          textAlign: TextAlign.center,
                                        ),
                                      ),
                                      Padding(
                                        padding: const EdgeInsets.all(20),
                                        child: Text(
                                          AppLocalizations.of(context)
                                              .viewOnBscScan,
                                          style: s_agRegular_gray12,
                                          textAlign: TextAlign.center,
                                        ),
                                      ),
                                      Padding(
                                        padding: const EdgeInsets.all(20),
                                        child: GestureDetector(
                                            child: Text(
                                              privateSaleBscScan,
                                              style:
                                                  s_agRegularLinkBlue5Underline,
                                              textAlign: TextAlign.center,
                                            ),
                                            onTap: () async {
                                              setState(() {
                                                isLoading = true;
                                              });
                                              try {
                                                await dappWidget(
                                                  context,
                                                  privateSaleBscScan,
                                                );
                                              } catch (_) {}
                                              setState(() {
                                                isLoading = false;
                                              });
                                            }),
                                      ),
                                    ],
                                  ),
                                );
                              }));
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
                                          color: Colors.white),
                                    ),
                                  ),
                          ),
                        ),
                      ],
                    )
                  else
                    SizedBox(
                      height: MediaQuery.of(context).size.height * .8,
                      child: const Center(
                        child: Loader(),
                      ),
                    )
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }

  @override
  bool get wantKeepAlive => true;
}
