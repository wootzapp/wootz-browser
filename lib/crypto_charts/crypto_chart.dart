import 'dart:convert';
import 'dart:math';

import 'package:cryptowallet/components/loader.dart';
import 'package:cryptowallet/crypto_charts/chart_price.dart';
import 'package:cryptowallet/utils/format_money.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:charts_flutter/flutter.dart' as charts;
import 'package:flutter/services.dart';
import 'package:get/state_manager.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart' as http;
import 'package:charts_flutter/src/text_element.dart' as TextElement;
import 'package:charts_flutter/src/text_style.dart' as style;
import 'package:intl/intl.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import '../utils/app_config.dart';

class CryptoChart extends StatefulWidget {
  final String name;
  final String symbol;
  const CryptoChart({Key key, this.name, this.symbol}) : super(key: key);

  @override
  State<CryptoChart> createState() => _CryptoChartState();
}

String getMarketData({int days, String coinGeckoId, String defaultCurrency}) {
  return "https://api.coingecko.com/api/v3/coins/$coinGeckoId/market_chart?vs_currency=$defaultCurrency&days=$days";
}

class _CryptoChartState extends State<CryptoChart> {
  List<charts.Series<List, num>> series;
  List<List<dynamic>> chartData = [];

  @override
  void initState() {
    super.initState();
  }

  RxInt days = 1.obs;

  Map savedData = {};
  final maxSecondsToRemakeRequest = 15;
  List<String> months = [
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
  RxString priceNotifier = ''.obs;
  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text('${widget.name} (${widget.symbol})'),
        ),
        body: SizedBox(
          height: double.infinity,
          child: RefreshIndicator(
            onRefresh: () async {
              await Future.delayed(const Duration(seconds: 2));
              setState(() {});
            },
            child: SafeArea(
              child: SingleChildScrollView(
                physics: const AlwaysScrollableScrollPhysics(),
                child: Padding(
                  padding: const EdgeInsets.all(25),
                  child: Column(
                    children: [
                      Card(
                        shape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(20),
                        ),
                        child: Column(
                          children: [
                            Padding(
                              padding: const EdgeInsets.only(top: 10.0),
                              child: Obx(
                                () => ChartPrice(
                                  chartPriceData: {
                                    'price': priceNotifier.value
                                  },
                                ),
                              ),
                            ),
                            SizedBox(
                              width: double.infinity,
                              height: 250,
                              child: FutureBuilder(future: () async {
                                final pref = Hive.box(secureStorageKey);
                                String defaultCurrency =
                                    pref.get('defaultCurrency') ?? "usd";

                                Map viewportY = {
                                  'min': 0,
                                  'max': 0,
                                };
                                Map viewportX = {
                                  'min': 0,
                                  'max': 0,
                                };

                                final currencyWithSymbol = jsonDecode(
                                  await rootBundle
                                      .loadString('json/currency_symbol.json'),
                                );

                                final symbol = currencyWithSymbol[
                                    defaultCurrency.toUpperCase()]['symbol'];

                                if (savedData[days] == null) {
                                  final request = await http
                                      .get(
                                        Uri.parse(
                                          getMarketData(
                                            days: days.value,
                                            coinGeckoId:
                                                coinGeckCryptoSymbolToID[
                                                    widget.symbol],
                                            defaultCurrency: defaultCurrency,
                                          ),
                                        ),
                                      )
                                      .timeout(networkTimeOutDuration);

                                  if (request.statusCode ~/ 100 == 4 ||
                                      request.statusCode ~/ 100 == 5) {
                                    throw Exception('Request failed');
                                  }

                                  savedData[days] = request.body;
                                }

                                int currentIndex = 0;

                                final jsonDecodedPrices =
                                    jsonDecode(savedData[days])['prices'];

                                priceNotifier.value = '$symbol${formatMoney([
                                  jsonDecodedPrices[
                                      (jsonDecodedPrices as List).length - 1]
                                ][0][1])}';

                                chartData =
                                    (jsonDecodedPrices as Iterable).map((e) {
                                  if (currentIndex == 0) {
                                    viewportX['max'] = e[0];
                                    viewportX['min'] = e[0];
                                    viewportY['min'] = e[0];
                                    viewportY['min'] = e[0];
                                  }
                                  if (viewportX['max'] < e[0]) {
                                    viewportX['max'] = e[0];
                                  }
                                  if (viewportX['min'] > e[0]) {
                                    viewportX['min'] = e[0];
                                  }
                                  if (viewportY['max'] < e[1]) {
                                    viewportY['max'] = e[1];
                                  }
                                  if (viewportY['min'] > e[1]) {
                                    viewportY['min'] = e[1];
                                  }
                                  currentIndex++;
                                  return List<dynamic>.from(e);
                                }).toList();

                                series = [
                                  charts.Series(
                                    id: 'crypto chart',
                                    data: chartData,
                                    labelAccessorFn: (List series, _) =>
                                        '${series[0]}',
                                    domainFn: (List series, _) => series[0],
                                    measureFn: (List series, _) => series[1],
                                    colorFn: (List series, _) =>
                                        charts.ColorUtil.fromDartColor(
                                            Colors.blue),
                                  )
                                ];
                                return {
                                  'viewportY': viewportY,
                                  'viewportX': viewportX,
                                  'symbol': symbol
                                };
                              }(), builder: (context, snapshot) {
                                if (snapshot.hasError) {
                                  if (kDebugMode) {
                                    print(snapshot.error);
                                  }
                                  return Center(
                                    child: Text(
                                      AppLocalizations.of(context)
                                          .couldNotFetchData,
                                      style: const TextStyle(fontSize: 18),
                                    ),
                                  );
                                }
                                if (snapshot.hasData) {
                                  return charts.LineChart(
                                    series,
                                    selectionModels: [
                                      charts.SelectionModelConfig(
                                          type: charts.SelectionModelType.info,
                                          changedListener:
                                              (charts.SelectionModel model) {
                                            if (model.hasDatumSelection) {
                                              final millis = model
                                                  .selectedDatum[0].datum[0];

                                              final dt = DateTime
                                                  .fromMillisecondsSinceEpoch(
                                                      millis);

                                              final date = DateFormat('hh:mm a')
                                                  .format(dt);

                                              final price = model
                                                  .selectedSeries[0]
                                                  .measureFn(model
                                                      .selectedDatum[0].index);
                                              CustomCircleSymbolRenderer
                                                      .backgroundColor =
                                                  Theme.of(context)
                                                      .textTheme
                                                      .bodyMedium
                                                      .color;
                                              CustomCircleSymbolRenderer
                                                      .textColor =
                                                  Theme.of(context)
                                                      .scaffoldBackgroundColor;

                                              priceNotifier.value =
                                                  '${snapshot.data['symbol']}${formatMoney(price)}';

                                              CustomCircleSymbolRenderer.value =
                                                  '$date\n${months[dt.month - 1]} ${dt.day}, ${dt.year}';
                                            }
                                          })
                                    ],
                                    behaviors: [
                                      charts.SelectNearest(
                                        eventTrigger:
                                            charts.SelectionTrigger.tapAndDrag,
                                      ),
                                      charts.LinePointHighlighter(
                                        symbolRenderer:
                                            CustomCircleSymbolRenderer(),
                                      ),
                                    ],
                                    domainAxis: charts.NumericAxisSpec(
                                      tickProviderSpec: const charts
                                              .BasicNumericTickProviderSpec(
                                          desiredTickCount: 10,
                                          zeroBound: false),
                                      renderSpec: const charts.NoneRenderSpec(),
                                      viewport: charts.NumericExtents(
                                        snapshot.data['viewportX']['min'],
                                        snapshot.data['viewportX']['max'],
                                      ),
                                    ),
                                    primaryMeasureAxis: charts.NumericAxisSpec(
                                      renderSpec: const charts.NoneRenderSpec(),
                                      viewport: charts.NumericExtents(
                                        snapshot.data['viewportY']['min'],
                                        snapshot.data['viewportY']['max'],
                                      ),
                                    ),
                                    animate: false,
                                  );
                                } else {
                                  return Column(
                                    mainAxisAlignment: MainAxisAlignment.center,
                                    children: const [
                                      Loader(),
                                    ],
                                  );
                                }
                              }),
                            ),
                            Padding(
                              padding: const EdgeInsets.only(
                                left: 20,
                                right: 20,
                              ),
                              child: Row(
                                mainAxisAlignment:
                                    MainAxisAlignment.spaceBetween,
                                children: [
                                  GestureDetector(
                                    onTap: () {
                                      days.value = 1;
                                    },
                                    child: Container(
                                      decoration: BoxDecoration(
                                        border: Border(
                                          bottom: BorderSide(
                                            width: 5,
                                            color: days.value == 1
                                                ? Colors.blue
                                                : Colors.transparent,
                                          ),
                                        ),
                                      ),
                                      width: 50,
                                      height: 30,
                                      child: const Center(
                                        child: Text(
                                          '1D',
                                        ),
                                      ),
                                    ),
                                  ),
                                  GestureDetector(
                                    onTap: () {
                                      days.value = 7;
                                    },
                                    child: Container(
                                      decoration: BoxDecoration(
                                        border: Border(
                                          bottom: BorderSide(
                                            width: 5,
                                            color: days.value == 7
                                                ? Colors.blue
                                                : Colors.transparent,
                                          ),
                                        ),
                                      ),
                                      width: 50,
                                      height: 30,
                                      child: const Center(
                                        child: Text(
                                          '1W',
                                        ),
                                      ),
                                    ),
                                  ),
                                  GestureDetector(
                                    onTap: () {
                                      days.value = 30;
                                    },
                                    child: Container(
                                      decoration: BoxDecoration(
                                        border: Border(
                                          bottom: BorderSide(
                                            width: 5,
                                            color: days.value == 30
                                                ? Colors.blue
                                                : Colors.transparent,
                                          ),
                                        ),
                                      ),
                                      width: 50,
                                      height: 30,
                                      child: const Center(
                                        child: Text(
                                          '1M',
                                        ),
                                      ),
                                    ),
                                  ),
                                  GestureDetector(
                                    onTap: () {
                                      days.value = 365;
                                    },
                                    child: Container(
                                      decoration: BoxDecoration(
                                        border: Border(
                                          bottom: BorderSide(
                                            width: 5,
                                            color: days.value == 365
                                                ? Colors.blue
                                                : Colors.transparent,
                                          ),
                                        ),
                                      ),
                                      width: 50,
                                      height: 30,
                                      child: const Center(
                                        child: Text(
                                          '1Y',
                                        ),
                                      ),
                                    ),
                                  ),
                                ],
                              ),
                            )
                          ],
                        ),
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ));
  }
}

class CustomCircleSymbolRenderer extends charts.CircleSymbolRenderer {
  static String value;
  static Color textColor;
  static Color backgroundColor;
  @override
  void paint(
    charts.ChartCanvas canvas,
    Rectangle<num> bounds, {
    List<int> dashPattern,
    charts.Color fillColor,
    charts.FillPatternType fillPattern,
    charts.Color strokeColor,
    double strokeWidthPx,
  }) {
    super.paint(
      canvas,
      bounds,
      dashPattern: dashPattern,
      fillColor: fillColor,
      strokeColor: strokeColor,
      strokeWidthPx: strokeWidthPx,
    );
    canvas.drawRRect(
      Rectangle(
        bounds.left - 5,
        bounds.top - 31,
        bounds.width + (5 * value.length),
        bounds.height + 30,
      ),
      radius: 5,
      roundTopLeft: true,
      roundTopRight: true,
      roundBottomRight: true,
      roundBottomLeft: true,
      fill: charts.ColorUtil.fromDartColor(backgroundColor),
    );
    final textStyle = style.TextStyle();
    textStyle.color = charts.ColorUtil.fromDartColor(textColor);
    textStyle.fontSize = 15;
    canvas.drawText(
      TextElement.TextElement(value, style: textStyle),
      (bounds.left).round(),
      (bounds.top - 26).round(),
    );
  }
}
