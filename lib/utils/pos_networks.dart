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
