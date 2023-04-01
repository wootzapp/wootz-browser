import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/material.dart';
import 'package:hive/hive.dart';

import '../main.dart';

class DarkModeToggler extends StatefulWidget {
  const DarkModeToggler({Key key}) : super(key: key);

  @override
  State<DarkModeToggler> createState() => _DarkModeTogglerState();
}

class _DarkModeTogglerState extends State<DarkModeToggler> {
  @override
  Widget build(BuildContext context) {
    return IconButton(
      onPressed: () async {
        MyApp.themeNotifier.value =
            Theme.of(context).brightness == Brightness.dark
                ? ThemeMode.light
                : ThemeMode.dark;
        final pref = Hive.box(secureStorageKey);
        await pref.put(
          darkModekey,
          !(Theme.of(context).brightness == Brightness.dark),
        );
      },
      icon: Icon(
        Theme.of(context).brightness == Brightness.dark
            ? Icons.light_mode
            : Icons.dark_mode,
        size: 25,
        color: Colors.white,
      ),
    );
  }
}
