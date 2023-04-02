import 'hdnode.dart';

/// BIP32 account with simple helper methods
class Account {
  final HDNode accountNode;

  int currentChild = 0;

  Account(this.accountNode, [this.currentChild]);

  /// Returns address at the current position
  String getCurrentAddress([bool legacyFormat = true]) {
    if (legacyFormat) {
      return accountNode.derive(currentChild).toLegacyAddress();
    } else {
      return accountNode.derive(currentChild).toCashAddress();
    }
  }

  /// moves the position forward and returns an address from the new position
  String getNextAddress([bool legacyFormat = true]) {
    if (legacyFormat) {
      return accountNode.derive(++currentChild).toLegacyAddress();
    } else {
      return accountNode.derive(++currentChild).toCashAddress();
    }
  }
}
