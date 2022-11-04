import 'dart:async';
import 'dart:math';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/format_money.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:intl/intl.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:web3dart/web3dart.dart';

import '../components/loader.dart';

class Staking extends StatefulWidget {
  const Staking({Key key}) : super(key: key);

  @override
  State<Staking> createState() => _StakingState();
}

class _StakingState extends State<Staking> {
  Map stakingDetails;

  Timer timer;

  @override
  void initState() {
    super.initState();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await getStakingDetails(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  Future getStakingDetails() async {
    final client = web3.Web3Client(
      getEVMBlockchains()[tokenStakingContractNetwork]['rpc'],
      Client(),
    );
    final pref = Hive.box(secureStorageKey);
    String mnemonic = pref.get(currentMmenomicKey);
    Map response = await getEthereumFromMemnomic(
      mnemonic,
      getEVMBlockchains()[tokenStakingContractNetwork]['coinType'],
    );
    final contract = web3.DeployedContract(
      web3.ContractAbi.fromJson(tokenStakingAbi, ''),
      web3.EthereumAddress.fromHex(
        tokenStakingContractAddress,
      ),
    );

    Map tokenDetails = await getERC20TokenNameSymbolDecimal(
      contractAddress: tokenContractAddress,
      rpc: getEVMBlockchains()[tokenContractNetwork]['rpc'],
    );

    double tokenBalance = await getERC20TokenBalance({
      'contractAddress': tokenContractAddress,
      'rpc': getEVMBlockchains()[tokenContractNetwork]['rpc'],
      'chainId': getEVMBlockchains()[tokenContractNetwork]['chainId'],
      'coinType': getEVMBlockchains()[tokenContractNetwork]['coinType'],
    });

    final getStakerDetails = contract.function('getStakerDetails');

    final getStakerDetailsResponse = await client.call(
        contract: contract,
        function: getStakerDetails,
        params: [EthereumAddress.fromHex(response['eth_wallet_address'])]);

    stakingDetails = {
      'totalStaked': getStakerDetailsResponse[0] /
          BigInt.from(pow(10, int.parse(tokenDetails['decimals']))),
      'stakedDuration': getStakerDetailsResponse[1],
      'availableReward': getStakerDetailsResponse[2] /
          BigInt.from(pow(10, int.parse(tokenDetails['decimals']))),
      'symbol': tokenDetails['symbol'],
      'tokenBalance': tokenBalance
    };
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Stake your $walletAbbr Tokens')),
      body: SafeArea(
        child: RefreshIndicator(
          onRefresh: () async {
            await Future.delayed(const Duration(seconds: 2));
            setState(() {});
          },
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Padding(
              padding: const EdgeInsets.all(25),
              child: Column(
                children: [
                  if (stakingDetails != null)
                    Column(
                      mainAxisAlignment: MainAxisAlignment.start,
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        SizedBox(
                          width: double.infinity,
                          child: Card(
                            shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(35)),
                            color: Colors.white,
                            child: Padding(
                              padding: const EdgeInsets.fromLTRB(30, 15, 0, 15),
                              child: Column(
                                mainAxisAlignment: MainAxisAlignment.start,
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  const Text(
                                    'Total Staked',
                                    style: TextStyle(
                                        fontWeight: FontWeight.bold,
                                        fontSize: 18,
                                        letterSpacing: 2),
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Text(
                                    '${formatMoney(stakingDetails['totalStaked'])} ${stakingDetails['symbol']}',
                                    style: const TextStyle(
                                        color: Colors.grey,
                                        fontSize: 16,
                                        letterSpacing: 2),
                                  ),
                                ],
                              ),
                            ),
                          ),
                        ),
                        SizedBox(
                          width: double.infinity,
                          child: Card(
                            shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(35)),
                            color: Colors.white,
                            child: Padding(
                              padding: const EdgeInsets.fromLTRB(30, 15, 0, 15),
                              child: Column(
                                mainAxisAlignment: MainAxisAlignment.start,
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  const Text(
                                    'Available Rewards',
                                    style: TextStyle(
                                        fontWeight: FontWeight.bold,
                                        fontSize: 18,
                                        letterSpacing: 2),
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Text(
                                    '${formatMoney(stakingDetails['availableReward'])} ${stakingDetails['symbol']}',
                                    style: const TextStyle(
                                        color: Colors.grey,
                                        fontSize: 16,
                                        letterSpacing: 2),
                                  ),
                                ],
                              ),
                            ),
                          ),
                        ),
                        SizedBox(
                          width: double.infinity,
                          child: Card(
                            shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(35)),
                            color: Colors.white,
                            child: Padding(
                              padding: const EdgeInsets.fromLTRB(30, 15, 0, 15),
                              child: Column(
                                mainAxisAlignment: MainAxisAlignment.start,
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  const Text(
                                    'Staked Duration',
                                    style: TextStyle(
                                        fontWeight: FontWeight.bold,
                                        fontSize: 18,
                                        letterSpacing: 2),
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Text(
                                    DateFormat('MM/dd/yyyy, hh:mm a').format(
                                        DateTime.fromMillisecondsSinceEpoch(
                                            (stakingDetails['stakedDuration']
                                                        as BigInt)
                                                    .toInt() *
                                                1000)),
                                    style: const TextStyle(
                                        color: Colors.grey,
                                        fontSize: 16,
                                        letterSpacing: 2),
                                  ),
                                ],
                              ),
                            ),
                          ),
                        ),
                        SizedBox(
                          width: double.infinity,
                          child: Card(
                            shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(35)),
                            color: Colors.white,
                            child: Padding(
                              padding: const EdgeInsets.fromLTRB(30, 15, 0, 15),
                              child: Column(
                                mainAxisAlignment: MainAxisAlignment.start,
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  const Text(
                                    'Token Balance',
                                    style: TextStyle(
                                        fontWeight: FontWeight.bold,
                                        fontSize: 18,
                                        letterSpacing: 2),
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Text(
                                    '${formatMoney(stakingDetails['tokenBalance'])} ${stakingDetails['symbol']}',
                                    style: const TextStyle(
                                        color: Colors.grey,
                                        fontSize: 16,
                                        letterSpacing: 2),
                                  ),
                                ],
                              ),
                            ),
                          ),
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
                            onPressed: () {},
                            child: const Padding(
                              padding: EdgeInsets.all(15),
                              child: Text(
                                'Stake',
                                style: TextStyle(
                                    letterSpacing: 1,
                                    color: Colors.white,
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16),
                              ),
                            ),
                          ),
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
                            onPressed: () {},
                            child: const Padding(
                              padding: EdgeInsets.all(15),
                              child: Text(
                                'Withdraw',
                                style: TextStyle(
                                    letterSpacing: 1,
                                    color: Colors.white,
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16),
                              ),
                            ),
                          ),
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
                            onPressed: () {},
                            child: const Padding(
                              padding: EdgeInsets.all(15),
                              child: Text(
                                'Claim',
                                style: TextStyle(
                                    letterSpacing: 1,
                                    color: Colors.white,
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16),
                              ),
                            ),
                          ),
                        )
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
}
