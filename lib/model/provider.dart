import 'package:flutter/material.dart';

class ProviderClass extends ChangeNotifier {}

class LocaleNotifier extends ChangeNotifier {
  Locale _locale;

  Locale get locale => _locale;

  void updateLocale(Locale locale) {
    _locale = locale;
    notifyListeners();
  }
}

class UserBalanceNotifier extends ChangeNotifier {
  Map<String, dynamic> _userBalance = {};

  Map<String, dynamic> get userBalance => _userBalance;

  void updateUserBalance(Map<String, dynamic> newBalance) {
    _userBalance = newBalance;
    notifyListeners();
  }
}
