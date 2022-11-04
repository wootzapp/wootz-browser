import 'package:cryptowallet/components/loader.dart';
import 'package:flutter/material.dart';

class EmptyLoadingScreen extends StatelessWidget {
  const EmptyLoadingScreen({Key key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return const Scaffold(
      body: Center(
        child: Loader(),
      ),
    );
  }
}
