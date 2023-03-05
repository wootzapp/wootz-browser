import 'dart:async';
import 'dart:convert';

import 'package:cryptowallet/components/user_balance.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:provider/provider.dart';
import '../models/provider.dart';
import '../utils/app_config.dart';

class Portfolio extends StatefulWidget {
  const Portfolio({Key? key}) : super(key: key);

  @override
  State<Portfolio> createState() => _PortfolioState();
}

class _PortfolioState extends State<Portfolio> {
  var userBalance = ValueNotifier<Map<dynamic, dynamic>>({});
  Timer? timer;
  final bool skipNetworkRequest = true;

  @override
  void initState() {
    super.initState();
    getUserBalance();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await getUserBalance(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  Future getUserBalance() async {
    try {
      final allCryptoPrice = jsonDecode(
        await getCryptoPrice(
          skipNetworkRequest: skipNetworkRequest,
        ),
      ) as Map;

      final pref = Hive.box(secureStorageKey);

      final mnemonic = pref.get(currentMmenomicKey);

      final currencyWithSymbol =
          jsonDecode(await rootBundle.loadString('json/currency_symbol.json'));

      final defaultCurrency = pref.get('defaultCurrency') ?? "USD";

      final symbol = currencyWithSymbol[defaultCurrency]['symbol'];

      double balance = await totalCryptoBalance(
        mnemonic: mnemonic,
        defaultCurrency: defaultCurrency,
        allCryptoPrice: allCryptoPrice,
        skipNetworkRequest: skipNetworkRequest,
      );

      userBalance.value = {
        'balance': balance,
        'symbol': symbol,
      };
      print(userBalance.value);
      // final userBalanceModel =
      //     Provider.of<UserBalanceNotifier>(context, listen: false);
      // userBalanceModel.updateUserBalance(userBalance.value);
    } catch (_) {}
  }

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.fromLTRB(20, 0, 20, 0),
      child: Stack(
        clipBehavior: Clip.none,
        alignment: Alignment.centerLeft,
        children: [
          Card(
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(20),
            ),
            child: Container(
              decoration: const BoxDecoration(
                color: appBackgroundblue,
                borderRadius: BorderRadius.all(Radius.circular(20)),
              ),
              width: double.infinity,
              height: 150,
              child: Padding(
                padding: const EdgeInsets.only(left: 20, right: 20),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const SizedBox(
                      height: 20,
                    ),
                    Text(
                      AppLocalizations.of(context)!.portfolio,
                      style: const TextStyle(
                        fontSize: 20,
                        color: Color.fromRGBO(255, 255, 255, .6),
                        letterSpacing: 3,
                      ),
                    ),
                    const SizedBox(
                      height: 20,
                    ),
                    // Consumer<UserBalanceNotifier>(
                    //     builder: (context, notifier, child) {
                    //   final userBalanceModel = Provider.of<UserBalanceNotifier>(
                    //       context,
                    //       listen: false);
                    ValueListenableBuilder(
                        valueListenable: userBalance,
                        builder: (context, value, child) {
                          if (userBalance.value != null &&
                              userBalance.value.isNotEmpty) {
                            return GestureDetector(
                              onTap: () async {
                                final pref = Hive.box(secureStorageKey);
                                final userPreviousHidingBalance = pref
                                    .get(hideBalanceKey, defaultValue: false);
                                await pref.put(
                                    hideBalanceKey, !userPreviousHidingBalance);
                              },
                              child: SizedBox(
                                height: 35,
                                child: UserBalance(
                                  symbol: userBalance.value['symbol'],
                                  balance: userBalance.value['balance'],
                                  reversed: true,
                                  iconSize: 29,
                                  iconDivider: const SizedBox(
                                    width: 5,
                                  ),
                                  iconSuffix: const Icon(
                                    FontAwesomeIcons.eyeSlash,
                                    color: Colors.white,
                                    size: 29,
                                  ),
                                  iconColor: Colors.white,
                                  textStyle: const TextStyle(
                                      color: Colors.white,
                                      fontSize: 30,
                                      fontWeight: FontWeight.bold),
                                ),
                              ),
                            );
                          } else {
                            return Container();
                          }
                        }),
                    const SizedBox(
                      height: 40,
                    ),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}
