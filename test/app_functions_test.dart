// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility that Flutter provides. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'dart:convert';

import 'package:cryptowallet/eip/eip681.dart';
import 'package:cryptowallet/utils/alt_ens.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/ethereum_blockies.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:cardano_wallet_sdk/cardano_wallet_sdk.dart' as cardano;
import 'package:flutter_test/flutter_test.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  final blockInstance = EthereumBlockies();
  final blockInstanceTwo = EthereumBlockies();
  const busdContractAddress = '0xe9e7cea3dedca5984780bafc599bd69add087d56';
  const address = '0x6Acf5505DF3Eada0BF0547FAb88a85b1A2e03F15';
  const addressTwo = '0x3064c83F8b28193d9B6E7c0717754163DDF3C70b';
  const ensAddress = 'vitalik.eth';
  const eip681String =
      'ethereum:ethereum-$busdContractAddress@56/transfer?address=$address&uint256=1000000000000000000';
  const unstoppableAddress = 'brad.crypto';

  test('can decode known abis', () {
    expect(solidityFunctionSig('withdraw(uint256)'), '0x2e1a7d4d');
    expect(solidityFunctionSig('ownerOf(uint256)'), '0x6352211e');
    expect(solidityFunctionSig('balanceOf(address)'), '0x70a08231');
    expect(solidityFunctionSig('transfer(address,uint256)'), '0xa9059cbb');
    expect(solidityFunctionSig('approve(address,uint256)'), '0x095ea7b3');
    expect(solidityFunctionSig('solversk()'), '0xffb5eff0');
  });

  test('get ethereum address blockie image data and colors', () {
    blockInstance.seedrand(address.toLowerCase());
    HSL color = blockInstance.createColor();
    HSL bgColor = blockInstance.createColor();
    HSL spotColor = blockInstance.createColor();
    List imageData = blockInstance.createImageData();
    expect(sha3(json.encode(blockInstance.randseed)),
        '89b8a19e375159267d7d16447f53766cbd210d6b0328779cc897a03a9922b914');
    expect(
        color.toString(), 'H: 25.0 S: 62.20454423791009 L: 50.21168109970711');
    expect(bgColor.toString(),
        'H: 108.0 S: 57.542195253792315 L: 43.62102017906542');
    expect(spotColor.toString(),
        'H: 31.0 S: 48.8115822751129 L: 50.77201500570961');
    expect(sha3(json.encode(imageData)),
        'd935e1c2fa18d0a7b7f92604e3ea282ab4572124852411306d70e302fb5447a4');

    /// Account two
    blockInstanceTwo.seedrand(addressTwo.toLowerCase());
    HSL colorTwo = blockInstanceTwo.createColor();
    HSL bgColorTwo = blockInstanceTwo.createColor();
    HSL spotColorTwo = blockInstanceTwo.createColor();
    List imageDataTwo = blockInstanceTwo.createImageData();
    expect(sha3(json.encode(blockInstanceTwo.randseed)),
        '491a7d9b769c9e62f67019b5ea33b5b100e8a38e55b1efc0680ac4edaaa18f79');
    expect(colorTwo.toString(),
        'H: 240.0 S: 77.89883877052871 L: 42.880431070402466');
    expect(bgColorTwo.toString(),
        'H: 302.0 S: 52.13426684594446 L: 15.5695927401863');
    expect(spotColorTwo.toString(),
        'H: 252.0 S: 74.00470713805626 L: 64.89102061134344');
    expect(sha3(json.encode(imageDataTwo)),
        '0da3e2aa1ee73f4caae2c09cd4febd40ebdf3a0128b2e6c4686ec93055f221d7');
  });

  test('javalongToInt accuracy convert java long numbers to int', () {
    expect(blockInstance.javaLongToInt(-32839282839282), 37105934);
  });
  test('eip681 conversion', () {
    expect(
        EIP681.build(
          prefix: 'ethereum',
          targetAddress: busdContractAddress,
          chainId: '56',
          functionName: 'transfer',
          parameters: {
            'uint256': (1e18).toString(),
            'address': address,
          },
        ),
        eip681String);

    expect(
      sha3(json.encode(EIP681.parse(eip681String))),
      '5a9e3c6f895795edc845d1bcc17a8e23fe4e176b887f9fb86e952e8a0a3e2908',
    );
  });

  test('name hash working correctly', () async {
    expect(
      nameHash(unstoppableAddress),
      '0x756e4e998dbffd803c21d23b06cd855cdc7a4b57706c95964a37e24b47c10fc9',
    );

    expect(
      nameHash(ensAddress),
      '0xee6c4522aab0003e8d14cd40a6af439055fd2577951148c14b6cea9a53475835',
    );
  });

  test('ens resolves correctly to address and content hash', () async {
    Map ensToAddressMap = await ensToAddress(
      cryptoDomainName: ensAddress,
    );

    Map ensToContentHash = await ensToContentHashAndIPFS(
      cryptoDomainName: ensAddress,
    );
    if (ensToAddressMap['success']) {
      expect(
        ensToAddressMap['msg'],
        '0xd8dA6BF26964aF9D7eEd9e03E53415D37aA96045',
      );
    } else {
      throw Exception(ensToAddressMap['msg']);
    }

    if (ensToContentHash['success']) {
      expect(
        ensToContentHash['msg'],
        'https://ipfs.io/ipfs/QmUWvM2pXSMk8kFMDqDnskMnURePtayXUECMYfwUJYgbks',
      );
    } else {
      throw Exception(ensToContentHash['msg']);
    }
  });

  test('unstoppable domain resolves correctly to address', () async {
    const domainAddress = unstoppableAddress;
    Map domainResult = await unstoppableDomainENS(
      cryptoDomainName: domainAddress,
      currency: 'BTC',
    );
    if (domainResult['success']) {
      expect(domainResult['msg'], 'bc1q359khn0phg58xgezyqsuuaha28zkwx047c0c3y');
    } else {
      throw Exception(domainResult['msg']);
    }
  });

  test('test solidity sha3(keccak256) returns correct data', () {
    expect(sha3('hello world'),
        '47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad');
  });

  test('bitcoin-kind of blockchain pos network,hd path, p2wpkh not null', () {
    for (String i in getBitCoinPOSBlockchains().keys) {
      expect(getBitCoinPOSBlockchains()[i]['POSNetwork'], isNotNull);
      expect(getBitCoinPOSBlockchains()[i]['P2WPKH'], isNotNull);
      expect(getBitCoinPOSBlockchains()[i]['derivationPath'], isNotNull);
      expect(getBitCoinPOSBlockchains()[i]['symbol'], isNotNull);
      expect(getBitCoinPOSBlockchains()[i]['default'], isNotNull);
      expect(getBitCoinPOSBlockchains()[i]['blockExplorer'], isNotNull);
      expect(getBitCoinPOSBlockchains()[i]['image'], isNotNull);
    }
  });
  test('ethereum-kind block have all valid inputs', () {
    for (String i in getEVMBlockchains().keys) {
      expect(getEVMBlockchains()[i]['rpc'], isNotNull);
      expect(getEVMBlockchains()[i]['chainId'], isNotNull);
      expect(getEVMBlockchains()[i]['blockExplorer'], isNotNull);
      expect(getEVMBlockchains()[i]['symbol'], isNotNull);
      expect(getEVMBlockchains()[i]['default'], isNotNull);
      expect(getEVMBlockchains()[i]['image'], isNotNull);
      expect(getEVMBlockchains()[i]['coinType'], isNotNull);
    }
  });
  test('stellar-kind block have all valid inputs', () {
    for (String i in getStellarBlockChains().keys) {
      expect(getStellarBlockChains()[i]['symbol'], isNotNull);
      expect(getStellarBlockChains()[i]['default'], isNotNull);
      expect(getStellarBlockChains()[i]['blockExplorer'], isNotNull);
      expect(getStellarBlockChains()[i]['image'], isNotNull);
      expect(getStellarBlockChains()[i]['sdk'], isNotNull);
      expect(getStellarBlockChains()[i]['cluster'], isNotNull);
    }
  });
  test('filecoin-kind block have all valid inputs', () {
    for (String i in getFilecoinBlockChains().keys) {
      expect(getFilecoinBlockChains()[i]['symbol'], isNotNull);
      expect(getFilecoinBlockChains()[i]['default'], isNotNull);
      expect(getFilecoinBlockChains()[i]['blockExplorer'], isNotNull);
      expect(getFilecoinBlockChains()[i]['image'], isNotNull);
      expect(getFilecoinBlockChains()[i]['baseUrl'], isNotNull);
      expect(getFilecoinBlockChains()[i]['prefix'], isNotNull);
    }
  });
  test('cardano-kind block have all valid inputs', () {
    for (String i in getCardanoBlockChains().keys) {
      expect(getCardanoBlockChains()[i]['symbol'], isNotNull);
      expect(getCardanoBlockChains()[i]['default'], isNotNull);
      expect(getCardanoBlockChains()[i]['blockExplorer'], isNotNull);
      expect(getCardanoBlockChains()[i]['blockFrostKey'], isNotNull);
      expect(getCardanoBlockChains()[i]['cardano_network'], isNotNull);
      expect(getCardanoBlockChains()[i]['image'], isNotNull);
    }
  });
  test('solana-kind block have all valid inputs', () {
    for (String i in getSolanaBlockChains().keys) {
      expect(getSolanaBlockChains()[i]['symbol'], isNotNull);
      expect(getSolanaBlockChains()[i]['default'], isNotNull);
      expect(getSolanaBlockChains()[i]['blockExplorer'], isNotNull);
      expect(getSolanaBlockChains()[i]['image'], isNotNull);
      expect(getSolanaBlockChains()[i]['solanaCluster'], isNotNull);
    }
  });
  test('check if seed phrase generates the correct crypto address', () async {
    // WARNING: These accounts, and their private keys, are publicly known.
    // Any funds sent to them on Mainnet or any other live network WILL BE LOST.
    const mnemonic =
        'express crane road good warm suggest genre organ cradle tuition strike manual';

    final bitcoinKeyLive = await compute(
      calculateBitCoinKey,
      Map.from(getBitCoinPOSBlockchains()['Bitcoin'])
        ..addAll({mnemonicKey: mnemonic}),
    );

    final litecoinKey = await compute(
      calculateBitCoinKey,
      Map.from(getBitCoinPOSBlockchains()['Litecoin'])
        ..addAll({mnemonicKey: mnemonic}),
    );

    final dogecoinKey = await compute(
      calculateBitCoinKey,
      Map.from(getBitCoinPOSBlockchains()['Dogecoin'])
        ..addAll({mnemonicKey: mnemonic}),
    );

    final zcashKey = await compute(
      calculateBitCoinKey,
      Map.from(getBitCoinPOSBlockchains()['ZCash'])
        ..addAll({mnemonicKey: mnemonic}),
    );
    final dashKey = await compute(
      calculateBitCoinKey,
      Map.from(getBitCoinPOSBlockchains()['Dash'])
        ..addAll({mnemonicKey: mnemonic}),
    );

    final ethereumKey = await compute(
      calculateEthereumKey,
      {
        mnemonicKey: mnemonic,
        'coinType': getEVMBlockchains()['Ethereum']['coinType'],
      },
    );
    final ethereumClassicKey = await compute(
      calculateEthereumKey,
      {
        mnemonicKey: mnemonic,
        'coinType': getEVMBlockchains()['Ethereum Classic']['coinType'],
      },
    );
    final cardanoLiveKey = await compute(
      calculateCardanoKey,
      {mnemonicKey: mnemonic, 'network': cardano.NetworkId.mainnet},
    );
    final cardanoTestNetKey = await compute(
      calculateCardanoKey,
      {mnemonicKey: mnemonic, 'network': cardano.NetworkId.testnet},
    );
    final stellarKey = await compute(
      calculateStellarKey,
      {mnemonicKey: mnemonic},
    );

    final filecoinKey = await compute(calculateFileCoinKey, mnemonic);
    final solanaKey = await compute(
      calculateSolanaKey,
      {
        mnemonicKey: mnemonic,
      },
    );

    if (enableTestNet) {
      final bitcoinKeyTest = await compute(
        calculateBitCoinKey,
        Map.from(getBitCoinPOSBlockchains()['Bitcoin(Test)'])
          ..addAll({mnemonicKey: mnemonic}),
      );
      expect(bitcoinKeyTest['address'], 'n4fpz8NjzHwBkyzHBhSYoAegc7LjWZ175E');
    }
    expect(bitcoinKeyLive['address'],
        'bc1qzd9a563p9hfd93e3e2k3986m3ve0nmy4dtruaf');

    expect(
        litecoinKey['address'], 'ltc1qsru3fe2ttd3zgjfhn3r5eqz6tpe5cfzqszg8s7');
    expect(dashKey['address'], 'Xy1VVEXaiJstcmA9Jr1k38rcr3sGn3kQti');
    expect(dogecoinKey['address'], 'DF6pp77Q4ms37ABLberK4EuBtREiB1BGJz');
    expect(zcashKey['address'], 't1UNRtPu3WJUVTwwpFQHUWcu2LAhCrwDWuU');
    expect(stellarKey['address'],
        'GA5MO26YHJK7VMDCTODG7DYO5YATNMRYQVTXNMNKKRFYXZOINJYQEXYT');
    expect(
      await etherPrivateKeyToAddress(ethereumKey),
      '0x4AA3f03885Ad09df3d0CD08CD1Fe9cC52Fc43dBF',
    );
    expect(
      await etherPrivateKeyToAddress(ethereumClassicKey),
      '0x5C4b9839FDD8D5156549bE3eD5a00c933AaA3544',
    );
    expect(filecoinKey['ck'], 'eyUZ+e0KK6R2++xl+FWskE0Q97e30yfXRT74Ne6RCYE=');
    expect(
      solanaKey['address'],
      '5rxJLW9p2NQPMRjKM1P3B7CQ7v2RASpz45T7QP39bX5W',
    );
    expect(
      cardanoLiveKey['address'],
      'addr1q9r4l5l6xzsvum2g5s7u99wt630p8qd9xpepf73reyyrmxpqde5sugs7jg27gp04fcq7a9z90gz3ac8mq7p7k5vwedsq34lpxc',
    );
    expect(
      cardanoTestNetKey['address'],
      'addr_test1qpr4l5l6xzsvum2g5s7u99wt630p8qd9xpepf73reyyrmxpqde5sugs7jg27gp04fcq7a9z90gz3ac8mq7p7k5vwedsqjrzp28',
    );
  });

  test('user pin length and pin trials is secured and correct.', () async {
    expect(pinLength, greaterThanOrEqualTo(4));
    expect(userPinTrials, greaterThanOrEqualTo(1));
    expect(maximumTransactionToSave, greaterThanOrEqualTo(10));
  });

  test('dapp browser signing key are correct.', () {
    expect(personalSignKey, 'Personal');
    expect(normalSignKey, 'Normal Sign');
    expect(typedMessageSignKey, "Typed Message");
  });

  test('can import token from blockchain', () async {
    Map bep20TokenDetails = await getERC20TokenDetails(
      contractAddress: busdContractAddress,
      rpc: getEVMBlockchains()['Smart Chain']['rpc'],
    );

    expect(bep20TokenDetails['name'], 'BUSD Token');
    expect(bep20TokenDetails['symbol'], 'BUSD');
    expect(bep20TokenDetails['decimals'], '18');
  });
}
