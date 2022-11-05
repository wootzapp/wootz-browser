import 'dart:async';
import 'dart:math';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/format_money.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/svg.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:web3dart/web3dart.dart';
import '../../config/colors.dart';
import '../../config/styles.dart';
import '../components/loader.dart';
import '../utils/slide_up_panel.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class ClaimAirdrop extends StatefulWidget {
  const ClaimAirdrop({Key key}) : super(key: key);

  @override
  _ClaimAirdropState createState() => _ClaimAirdropState();
}

class _ClaimAirdropState extends State<ClaimAirdrop> {
  bool isLoading = false;
  bool isClaiming = false;
  final scaffoldKey = GlobalKey<ScaffoldState>();
  Map airdropDetails;
  final client = web3.Web3Client(
    getEVMBlockchains()[tokenSaleContractNetwork]['rpc'],
    Client(),
  );
  Timer timer;

  @override
  void initState() {
    super.initState();
    getAirDropdetails();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await getAirDropdetails(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    client?.dispose();
    super.dispose();
  }

  Future getAirDropdetails() async {
    try {
      final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);

      final response = await getEthereumFromMemnomic(
        mnemonic,
        getEVMBlockchains()[tokenSaleContractNetwork]['coinType'],
      );
      final sendingAddress =
          web3.EthereumAddress.fromHex(response['eth_wallet_address']);

      Map tokenDetails = await getERC20TokenNameSymbolDecimal(
        contractAddress: tokenContractAddress,
        rpc: getEVMBlockchains()[tokenContractNetwork]['rpc'],
      );

      final tokenSaleContract = web3.DeployedContract(
          web3.ContractAbi.fromJson(tokenSaleAbi, walletName),
          web3.EthereumAddress.fromHex(tokenSaleContractAddress));

      final viewAirdrop = tokenSaleContract.function(
        'viewAirdrop',
      );
      final viewAirdropTrans = await client
          .call(contract: tokenSaleContract, function: viewAirdrop, params: []);

      final getAirdrop = tokenSaleContract.function(
        'getAirdrop',
      );

      double transactionFee = await getEtherTransactionFee(
        getEVMBlockchains()[tokenSaleContractNetwork]['rpc'],
        getAirdrop.encodeCall([EthereumAddress.fromHex(zeroAddress)]),
        sendingAddress,
        EthereumAddress.fromHex(tokenSaleContractAddress),
      );

      airdropDetails = {
        'success': true,
        'symbol': tokenDetails['symbol'],
        'airdropAmount': double.parse(viewAirdropTrans[2].toString()) /
            pow(10, int.parse(tokenDetails['decimals'])),
        'transactionFee': transactionFee / pow(10, etherDecimals)
      };
      if (mounted) {
        setState(() {});
      }
    } catch (_) {}
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: scaffoldKey,
      appBar: AppBar(title: Text(AppLocalizations.of(context).airDrop)),
      body: SafeArea(
        child: RefreshIndicator(
          onRefresh: () async {
            await Future.delayed(const Duration(seconds: 2));
            setState(() {});
          },
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Padding(
              padding: const EdgeInsets.all(15.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  if (airdropDetails != null)
                    Column(
                      mainAxisAlignment: MainAxisAlignment.start,
                      crossAxisAlignment: CrossAxisAlignment.center,
                      children: [
                        const SizedBox(
                          height: 30,
                        ),
                        SvgPicture.asset('assets/airdrop_big.svg'),
                        const SizedBox(
                          height: 50,
                        ),
                        Text(
                          AppLocalizations.of(context).youWouldEarn,
                          style: s_agRegular_gray12,
                          textAlign: TextAlign.center,
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        Text(
                          '${formatMoney(airdropDetails['airdropAmount'])} ${airdropDetails['symbol']}',
                          style: const TextStyle(
                            fontWeight: FontWeight.bold,
                            fontSize: 25,
                          ),
                        ),
                        const SizedBox(
                          height: 50,
                        ),
                        Text(
                          '${AppLocalizations.of(context).transactionFee}: ${formatMoney(airdropDetails['transactionFee'])} ${getEVMBlockchains()[tokenSaleContractNetwork]['symbol']}',
                          style: const TextStyle(fontSize: 17),
                        ),
                        const SizedBox(
                          height: 20,
                        ),
                        Container(
                          color: Colors.transparent,
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
                              ScaffoldMessenger.of(context)
                                  .hideCurrentSnackBar();
                              setState(() {
                                isClaiming = true;
                              });
                              bool isClaimed = false;
                              String bscAirdropUrl;
                              String transactionHash;

                              try {
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

                                final contract = web3.DeployedContract(
                                    web3.ContractAbi.fromJson(
                                        tokenSaleAbi, walletName),
                                    web3.EthereumAddress.fromHex(
                                        tokenSaleContractAddress));

                                final getAirdrop = contract.function(
                                  'getAirdrop',
                                );

                                final trans = await client.signTransaction(
                                    credentials,
                                    Transaction.callContract(
                                      contract: contract,
                                      function: getAirdrop,
                                      parameters: [
                                        EthereumAddress.fromHex(zeroAddress)
                                      ],
                                    ),
                                    chainId: getEVMBlockchains()[
                                        tokenSaleContractNetwork]['chainId']);

                                transactionHash =
                                    await client.sendRawTransaction(trans);

                                isClaimed = true;
                              } catch (e) {
                                if (kDebugMode) {
                                  print(e);
                                }
                                setState(() {
                                  isClaiming = false;
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

                              bscAirdropUrl =
                                  getEVMBlockchains()[tokenSaleContractNetwork]
                                          ['blockExplorer']
                                      .toString()
                                      .replaceFirst(
                                        transactionhashTemplateKey,
                                        transactionHash,
                                      );
                              await slideUpPanel(context,
                                  StatefulBuilder(builder: (ctx, setState) {
                                return Padding(
                                  padding: const EdgeInsets.all(20),
                                  child: Column(
                                    mainAxisSize: MainAxisSize.min,
                                    crossAxisAlignment:
                                        CrossAxisAlignment.center,
                                    children: [
                                      isClaimed
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
                                          isClaimed
                                              ? AppLocalizations.of(context)
                                                  .airdropSuccess
                                              : AppLocalizations.of(context)
                                                  .airdropFailed,
                                          style: title1,
                                          textAlign: TextAlign.center,
                                        ),
                                      ),
                                      !isClaimed
                                          ? Container()
                                          : Padding(
                                              padding: const EdgeInsets.all(20),
                                              child: Text(
                                                AppLocalizations.of(context)
                                                    .viewOnBscScan,
                                                style: s_agRegular_gray12,
                                                textAlign: TextAlign.center,
                                              ),
                                            ),
                                      !isClaimed
                                          ? Container()
                                          : Padding(
                                              padding: const EdgeInsets.all(20),
                                              child: GestureDetector(
                                                  child: Text(
                                                    bscAirdropUrl,
                                                    style:
                                                        s_agRegularLinkBlue5Underline,
                                                    textAlign: TextAlign.center,
                                                  ),
                                                  onTap: () async {
                                                    setState(() {
                                                      isLoading = true;
                                                    });
                                                    try {
                                                      await dappWidget(context,
                                                          bscAirdropUrl);
                                                    } catch (e) {
                                                      //
                                                    }
                                                    setState(() {
                                                      isLoading = false;
                                                    });
                                                  }),
                                            ),
                                    ],
                                  ),
                                );
                              }));

                              setState(() {
                                isClaiming = false;
                              });
                            },
                            child: isClaiming
                                ? const Padding(
                                    padding: EdgeInsets.all(8.0),
                                    child: Loader(color: white),
                                  )
                                : Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: Text(
                                      AppLocalizations.of(context).claim,
                                      style: const TextStyle(
                                          fontWeight: FontWeight.bold,
                                          color: Colors.white),
                                    ),
                                  ),
                          ),
                        ),
                        const SizedBox(
                          height: 20,
                        ),
                        Text(
                          AppLocalizations.of(context).airdropClaimedOnceOnly,
                          style: s_agRegular_gray12,
                          textAlign: TextAlign.center,
                        ),
                      ],
                    )
                  else
                    SizedBox(
                      height: MediaQuery.of(context).size.height * .8,
                      child: const Center(
                        child: Loader(),
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
