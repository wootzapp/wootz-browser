import 'package:bitcoin_flutter/bitcoin_flutter.dart';

NetworkType litecoin = NetworkType(
  messagePrefix: '\x19Litecoin Signed Message:\n',
  bech32: 'ltc',
  bip32: Bip32Type(public: 0x019da462, private: 0x019d9cfe),
  pubKeyHash: 0x30,
  scriptHash: 0x32,
  wif: 0xb0,
);

NetworkType zcash = NetworkType(
  messagePrefix: '\x18ZCash Signed Message:\n',
  bip32: Bip32Type(public: 0x0488B21E, private: 0x0488ADE4),
  pubKeyHash: 0x1CB8,
  scriptHash: 0x1CBD,
  bech32: 't1',
  wif: 0x80,
);
final namecoin = NetworkType(
  messagePrefix: '\x18Namecoin Signed Message:\n',
  bech32: 'nc',
  bip32: Bip32Type(public: 0x0488b21e, private: 0x0488ade4),
  pubKeyHash: 0x34, //From 52
  scriptHash: 0x0d, //13
  wif: 0xb4,
); //fr

final firoNetwork = NetworkType(
  messagePrefix: '\x18Zcoin Signed Message:\n',
  bech32: 'bc',
  bip32: Bip32Type(public: 0x0488b21e, private: 0x0488ade4),
  pubKeyHash: 0x52,
  scriptHash: 0x07,
  wif: 0xd2,
);

final firoTestNetwork = NetworkType(
  messagePrefix: '\x18Zcoin Signed Message:\n',
  bech32: 'bc',
  bip32: Bip32Type(public: 0x043587cf, private: 0x04358394),
  pubKeyHash: 0x41,
  scriptHash: 0xb2,
  wif: 0xb9,
);

NetworkType dash = NetworkType(
  messagePrefix: '\x19Dash Signed Message:\n',
  bip32: Bip32Type(public: 0x02fe52cc, private: 0x02fe52f8),
  pubKeyHash: 0x4c,
  scriptHash: 0x10,
  wif: 0xcc,
);

NetworkType dogecoin = NetworkType(
  messagePrefix: '\x19Dogecoin Signed Message:\n',
  bip32: Bip32Type(public: 0x02facafd, private: 0x02fac398),
  pubKeyHash: 0x1e,
  scriptHash: 0x16,
  wif: 0x9e,
);

final dogecointestnet = NetworkType(
  messagePrefix: '\x18Dogecoin Signed Message:\n',
  bech32: 'tb',
  bip32: Bip32Type(public: 0x043587cf, private: 0x04358394),
  pubKeyHash: 0x71,
  scriptHash: 0xc4,
  wif: 0xf1,
);

final bitcoincash = NetworkType(
  messagePrefix: '\x18Bitcoin Signed Message:\n',
  bech32: 'bc',
  bip32: Bip32Type(public: 0x0488b21e, private: 0x0488ade4),
  pubKeyHash: 0x00,
  scriptHash: 0x05,
  wif: 0x80,
);

final bitcoincashtestnet = NetworkType(
  messagePrefix: '\x18Bitcoin Signed Message:\n',
  bech32: 'tb',
  bip32: Bip32Type(public: 0x043587cf, private: 0x04358394),
  pubKeyHash: 0x6f,
  scriptHash: 0xc4,
  wif: 0xef,
);
