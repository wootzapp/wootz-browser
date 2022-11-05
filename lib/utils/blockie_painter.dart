import 'dart:math';

import 'package:cryptowallet/utils/ethereum_blockies.dart';
import 'package:flutter/material.dart';

class BlockiePainter extends CustomPainter {
  String address;
  BlockiePainter(this.address);
  @override
  void paint(Canvas canvas, Size size) {
    canvas.scale(size.width);
    final blockInstance = EthereumBlockies();
    blockInstance.seedrand(address.toLowerCase());
    HSL color = blockInstance.createColor();
    HSL bgColor = blockInstance.createColor();
    HSL spotColor = blockInstance.createColor();

    List imageData = blockInstance.createImageData();

    int width =
        (blockInstance.javaLongToInt(sqrt(imageData.length)) as num).toInt();

    int w = width * blockInstance.size;
    int h = width * blockInstance.size;

    Color backGroundColor_ =
        HSLColor.fromAHSL(1, bgColor.h, bgColor.s / 100, bgColor.l / 100)
            .toColor();
    Color spotColor_ =
        HSLColor.fromAHSL(1, spotColor.h, spotColor.s / 100, spotColor.l / 100)
            .toColor();
    Color imageColor_ =
        HSLColor.fromAHSL(1, color.h, color.s / 100, color.l / 100).toColor();

    Paint paint = Paint();
    paint.color = backGroundColor_;
    paint.style = PaintingStyle.fill;

    canvas.clipRRect(
      RRect.fromRectAndRadius(
        Rect.fromLTWH(
            -w.toDouble() / 2, -w.toDouble() / 2, w.toDouble(), w.toDouble()),
        Radius.circular(w.toDouble() / 2),
      ),
    );
    canvas.drawRect(
        Rect.fromLTWH(
            -w.toDouble() / 2, -h.toDouble() / 2, w.toDouble(), h.toDouble()),
        paint);

    for (int i = 0; i < imageData.length; i++) {
      if (imageData[i] != 0) {
        final row = blockInstance.javaLongToInt((i / width).floor());
        final col = i % width;

        paint.color = imageData[i] == 1 ? imageColor_ : spotColor_;
        canvas.drawRect(
          Rect.fromLTWH(
            (col * blockInstance.size).toDouble() - w.toDouble() / 2,
            (row * blockInstance.size).toDouble() - h.toDouble() / 2,
            blockInstance.size.toDouble(),
            blockInstance.size.toDouble(),
          ),
          paint,
        );
      }
    }
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) {
    return false;
  }
}
