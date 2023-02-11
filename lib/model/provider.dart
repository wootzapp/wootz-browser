import 'package:flutter/material.dart';
import 'package:hive/hive.dart';

import '../utils/app_config.dart';

final pref = Hive.box(secureStorageKey);
// dynamic mnemonicList = pref.get(mnemonicListKey);

class ProviderClass extends ChangeNotifier {
  bool _checkAnyProfilePresent = false;

  int _currentWindowId = 0;

  int get currentWindowId => _currentWindowId;

  bool get checkAnyProfilePresent => _checkAnyProfilePresent;

  Map<int, dynamic> _tabUserCred = {};

  Map<int, dynamic> get tabUserCred => _tabUserCred;

  String _defaultProfile;

  String get defaultProfile => _defaultProfile;

  int _defaultChain;

  int get defaultChain => _defaultChain;

  String _currentProfile;

  String get currentProfile => _currentProfile;

  int _currentChain;

  int get currentChain => _currentChain;

  void setCurrentWindowId(int windowId) {
    _currentWindowId = windowId;
  }

  void setCheckAnyProfilePresent() {
    // if (mnemonicList.length > 0) {
    //   _checkAnyProfilePresent = true;
    // }
    dynamic mnemonicList = pref.get(mnemonicListKey);
    _checkAnyProfilePresent = mnemonicList != null ?? false;
    notifyListeners();
  }

  void setDefaultProfile() {
    _defaultProfile = pref.get(currentMmenomicKey);
    print('default profile $_defaultProfile');
    notifyListeners();
    setDefaultChain();
  }

  void setDefaultChain() {
    // _defaultChain = pref.get(currentUserWalletNameKey);
    _defaultChain = 1;
    notifyListeners();
  }

  void updateMnemonicList() {}

  void addTabUserCred(int windowId) {
    _tabUserCred[windowId] = {
      'profile': _defaultProfile,
      'chain': _defaultChain
    };
    notifyListeners();
  }

  void changeTabUserCred(int windowId, String profile, int chain) {
    print('changeTabUser called');
    print('windowId $windowId');
    if (profile == '') {
      _tabUserCred[windowId]['chain'] = chain;
      print('chain changed inside provider');
    } else if (chain == null) {
      _tabUserCred[windowId]['profile'] = profile;
      print('profile changed inside provider $profile');
    } else {
      _tabUserCred[windowId] = {
        'profile': profile,
        'chain': chain,
      };
      print('profile and chain both changed');
    }
    print('from provider ${_tabUserCred[windowId]['chain']}');

    _currentProfile = _tabUserCred[windowId]['profile'];
    _currentChain = _tabUserCred[windowId]['chain'];

    notifyListeners();
  }

  void deleteTabUserCred(int windowId) {
    _tabUserCred.remove(windowId);
    notifyListeners();
  }
}
