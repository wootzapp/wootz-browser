import 'package:flutter/material.dart';

Widget buildRow(
  String imageAsset,
  String name, {
  bool isSelected = false,
}) {
  return Padding(
    padding: const EdgeInsets.symmetric(horizontal: 20.0),
    child: Column(
      children: <Widget>[
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(
              children: <Widget>[
                if (imageAsset != '')
                  CircleAvatar(backgroundImage: AssetImage(imageAsset))
                else
                  const Icon(Icons.account_box_rounded),
                const SizedBox(width: 12),
                Text(name),
              ],
            ),
            if (isSelected)
              Container(
                decoration: const BoxDecoration(
                    shape: BoxShape.circle, color: Colors.blue),
                child: const Padding(
                  padding: EdgeInsets.all(2),
                  child: Icon(
                    Icons.check,
                    size: 20,
                    color: Colors.white,
                  ),
                ),
              ),
          ],
        ),
        const Divider()
      ],
    ),
  );
}
