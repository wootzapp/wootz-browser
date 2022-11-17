import 'dart:async';
import 'dart:convert';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/format_money.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:get/state_manager.dart';
import 'package:hive/hive.dart';

class GetBlockChainWidget extends StatefulWidget {
  final AssetImage image;
  final String name;
  final String priceWithCurrency;
  final double cryptoChange;
  final Widget cryptoAmount;
  final bool hasPrice;
  final String symbol;

  const GetBlockChainWidget({
    Key key,
    AssetImage image_,
    String name_,
    String priceWithCurrency_,
    double cryptoChange_,
    Widget cryptoAmount_,
    String symbol_,
    bool hasPrice_,
  })  : hasPrice = hasPrice_,
        image = image_,
        symbol = symbol_,
        name = name_,
        priceWithCurrency = priceWithCurrency_,
        cryptoChange = cryptoChange_,
        cryptoAmount = cryptoAmount_,
        super(key: key);

  @override
  State<GetBlockChainWidget> createState() => _GetBlockChainWidgetState();
}

class _GetBlockChainWidgetState extends State<GetBlockChainWidget> {
  Timer timer;
  RxMap blockchainPrice = {}.obs;
  bool skipNetworkRequest = true;

  @override
  void initState() {
    super.initState();
    if (widget.hasPrice != null && widget.hasPrice) {
      getBlockchainPrice();
      timer = Timer.periodic(
        httpPollingDelay,
        (Timer t) async {
          try {
            await getBlockchainPrice();
          } catch (_) {}
        },
      );
    }
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  Future getBlockchainPrice() async {
    try {
      Map allCryptoPrice = jsonDecode(
        await getCryptoPrice(
          skipNetworkRequest: skipNetworkRequest,
        ),
      ) as Map;

      if (skipNetworkRequest) skipNetworkRequest = false;

      final currencyWithSymbol = jsonDecode(
        await rootBundle.loadString('json/currency_symbol.json'),
      );

      final defaultCurrency =
          Hive.box(secureStorageKey).get('defaultCurrency') ?? "USD";
      final symbol = currencyWithSymbol[defaultCurrency]['symbol'];

      final Map cryptoMarket =
          allCryptoPrice[coinGeckCryptoSymbolToID[widget.symbol]];

      final double cryptoWidgetPrice =
          (cryptoMarket[defaultCurrency.toLowerCase()] as num).toDouble();

      blockchainPrice.value = {
        'price': symbol + formatMoney(cryptoWidgetPrice),
        'change':
            (cryptoMarket[defaultCurrency.toLowerCase() + '_24h_change'] as num)
                .toDouble()
      };
    } catch (_) {}
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      color: Colors.transparent,
      child: Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
        Expanded(
          child: Row(
            children: [
              widget.image != null
                  ? CircleAvatar(
                      backgroundImage: widget.image,
                      backgroundColor: Theme.of(context).backgroundColor,
                    )
                  : CircleAvatar(
                      child: Text(
                        ellipsify(str: widget.symbol, maxLength: 3),
                        textAlign: TextAlign.center,
                        style: const TextStyle(
                          color: Colors.white,
                        ),
                      ),
                    ),
              const SizedBox(
                width: 10,
              ),
              Flexible(
                child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceBetween,
                        children: [
                          Text(
                            widget.name,
                            style: const TextStyle(
                                fontSize: 15, fontWeight: FontWeight.w500),
                            overflow: TextOverflow.fade,
                          ),
                          widget.cryptoAmount
                        ],
                      ),
                      const SizedBox(
                        height: 10,
                      ),
                      Row(
                        children: [
                          Expanded(
                              child: Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              Obx(() {
                                if (blockchainPrice != null &&
                                    blockchainPrice.isNotEmpty) {
                                  return Row(
                                    children: [
                                      Text(
                                        blockchainPrice['price'],
                                        style: TextStyle(
                                            fontSize: 15,
                                            color: widget.hasPrice
                                                ? null
                                                : const Color(0x00ffffff)),
                                      ),
                                      const SizedBox(
                                        width: 5,
                                      ),
                                      widget.hasPrice
                                          ? Text(
                                              (blockchainPrice['change'] > 0
                                                      ? '+'
                                                      : '') +
                                                  formatMoney(blockchainPrice[
                                                      'change']) +
                                                  '%',
                                              style: widget.hasPrice
                                                  ? TextStyle(
                                                      fontSize: 12,
                                                      color: (blockchainPrice[
                                                                  'change'] <
                                                              0)
                                                          ? red
                                                          : green,
                                                    )
                                                  : const TextStyle(
                                                      fontSize: 12,
                                                      color: Color(0x00ffffff)),
                                            )
                                          : Container()
                                    ],
                                  );
                                } else {
                                  return Container();
                                }
                              })
                            ],
                          )),
                        ],
                      ),
                    ]),
              )
            ],
          ),
        ),
      ]),
    );
  }
}
