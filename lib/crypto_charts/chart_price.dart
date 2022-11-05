import 'package:flutter/material.dart';

class ChartPrice extends StatelessWidget {
  final Map chartPriceData;
  const ChartPrice({Key key, this.chartPriceData}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Text(
          chartPriceData['price'] ?? '',
          style: const TextStyle(
            fontWeight: FontWeight.bold,
            fontSize: 20,
          ),
        ),
      ],
    );
  }
}
