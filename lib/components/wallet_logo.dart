import 'package:flutter/material.dart';

class WalletLogo extends StatelessWidget {
  const WalletLogo({Key key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Image.asset(
      'assets/logo.png',
      width: 100,
    );
  }
}
