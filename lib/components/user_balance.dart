import 'package:flutter/material.dart';
import 'package:hive_flutter/hive_flutter.dart';
import '../utils/app_config.dart';
import '../utils/format_money.dart';
import 'hide_balance_widget.dart';

class UserBalance extends StatefulWidget {
  final double balance;
  final double iconSize;
  final String symbol;
  final TextStyle textStyle;
  final Color iconColor;
  final Widget iconDivider;
  final bool reversed;
  final Widget iconSuffix;
  const UserBalance({
    Key key,
    this.symbol,
    this.balance,
    this.textStyle,
    this.iconSize,
    this.iconColor,
    this.iconSuffix,
    this.iconDivider,
    this.reversed,
  }) : super(key: key);

  @override
  State<UserBalance> createState() => _UserBalanceState();
}

class _UserBalanceState extends State<UserBalance> {
  @override
  Widget build(BuildContext context) {
    return ValueListenableBuilder(
      valueListenable:
          Hive.box(secureStorageKey).listenable(keys: [hideBalanceKey]),
      builder: (context, box, _) {
        if (box.get(hideBalanceKey, defaultValue: false)) {
          return HideBalanceWidget(
            iconSize: widget.iconSize,
            iconColor: widget.iconColor,
            iconDivider: widget.iconDivider,
            iconSuffix: widget.iconSuffix,
          );
        }

        String toDisplay = '${formatMoney(widget.balance)} ${widget.symbol}';
        if (widget.reversed != null && widget.reversed == true) {
          toDisplay = toDisplay.split(' ').reversed.toList().join(' ');
        }

        return Text(
          toDisplay,
          style: widget.textStyle ??
              const TextStyle(
                fontWeight: FontWeight.w500,
                fontSize: 15,
                overflow: TextOverflow.fade,
              ),
        );
      },
    );
  }
}
