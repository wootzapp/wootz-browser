import 'package:flutter/material.dart';

class Loader extends StatelessWidget {
  final Color color;
  final double size;
  const Loader({Key key, this.color, this.size}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: size ?? 25,
      height: size ?? 25,
      child: CircularProgressIndicator(
        color: color,
        strokeWidth: 2,
      ),
    );
  }
}
