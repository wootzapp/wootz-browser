import 'package:cryptowallet/utils/blockie_painter.dart';
import 'package:flutter/material.dart';

class BlockieWidget extends StatefulWidget {
  final double size;
  final String data;
  const BlockieWidget({Key key, this.size, this.data}) : super(key: key);

  @override
  State<BlockieWidget> createState() => _BlockieWidgetState();
}

class _BlockieWidgetState extends State<BlockieWidget> {
  @override
  Widget build(BuildContext context) {
    return SafeArea(
      child: Center(
        child: CustomPaint(
          painter: BlockiePainter(widget.data),
          size: Size(widget.size, widget.size),
        ),
      ),
    );
  }
}
