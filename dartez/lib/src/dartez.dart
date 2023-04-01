library dartez;

import 'dart:convert';
import 'dart:typed_data';
import 'dart:core';
import 'package:convert/convert.dart';
import 'package:blake2b/blake2b_hash.dart';
import 'package:crypto/crypto.dart';
import 'package:dartez/utils/wallet_utils.dart';
import 'package:ed25519_hd_key/ed25519_hd_key.dart';
import 'package:bip39/bip39.dart' as bip39;
import 'package:bs58check/bs58check.dart' as bs58check;
import 'package:dartez/chain/tezos/tezos_language_util.dart';
import 'package:dartez/chain/tezos/tezos_message_utils.dart';
import 'package:dartez/chain/tezos/tezos_node_reader.dart';
import 'package:dartez/chain/tezos/tezos_node_writer.dart';
import 'package:dartez/helper/constants.dart';
import 'package:dartez/helper/http_helper.dart';
import 'package:dartez/helper/operation_helper.dart';
import 'package:dartez/packages/password_hash/pbkdf2.dart';
import 'package:dartez/reporting/tezos/tezos_conseil_client.dart';
import 'package:dartez/src/soft-signer/soft_signer.dart';
import 'package:dartez/dartez.dart';
import 'package:dartez/utils/sodium/sodium_utils.dart';
import "package:unorm_dart/unorm_dart.dart" as unorm;
import 'package:dartez/helper/generateKeys.dart';

import '../utils/sodium/platform_impl/sodium_utils_base.dart';

class Dartez {
  static SodiumUtils sodiumUtils = SodiumUtils();

  // init dartez
  Future<void> init() async {
    await sodiumUtils.init();
  }

  static String generateMnemonic({int strength = 256}) {
    return bip39.generateMnemonic(strength: strength);
  }

  static List<String> getKeysFromMnemonic({
    required String mnemonic,
    SignerCurve signerCurve = SignerCurve.ED25519,
  }) {
    Uint8List seed = bip39.mnemonicToSeed(mnemonic);
    Uint8List seedLength32 = seed.sublist(0, 32);
    return WalletUtils().getKeysFromMnemonic(seedLength32, signerCurve);
  }

  static Future<List<String>> getKeysFromMnemonicAndPassphrase({
    required String mnemonic,
    required String passphrase,
  }) async {
    return await _unlockKeys(
      passphrase: passphrase,
      mnemonic: mnemonic,
    );
  }

  static Future<List<String>> restoreIdentityFromDerivationPath(
      String derivationPath, String mnemonic,
      {String password = '',
      String? pkh,
      bool validate = true,
      SignerCurve signerCurve = SignerCurve.ED25519}) async {
    if (validate) {
      if (![12, 15, 18, 21, 24].contains(mnemonic.split(' ').length)) {
        throw new Exception("Invalid mnemonic length.");
      }
      if (!bip39.validateMnemonic(mnemonic)) {
        throw new Exception("The given mnemonic could not be validated.");
      }
    }

    Uint8List seed = bip39.mnemonicToSeed(mnemonic);

    if (derivationPath.length > 0) {
      KeyData keysource = await ED25519_HD_KEY.derivePath(derivationPath, seed);
      var combinedKey = Uint8List.fromList(keysource.key + keysource.chainCode);
      if (signerCurve == SignerCurve.ED25519) {
        return WalletUtils().getKeysFromPrivateKey(
            GenerateKeys.readKeysWithHint(
                combinedKey, GenerateKeys.keyPrefixes[PrefixEnum.edsk]!),
            signerCurve);
      } else {
        return WalletUtils().getKeysFromPrivateKey(
            GenerateKeys.readKeysWithHint(combinedKey.sublist(0, 32),
                GenerateKeys.keyPrefixes[PrefixEnum.spsk]!),
            signerCurve);
      }
    } else {
      return await _unlockKeys(mnemonic: mnemonic, passphrase: password);
    }
  }

  static List<String> getKeysFromSecretKey(String skKey) {
    if (skKey.startsWith("edsk")) {
      return WalletUtils().getKeysFromPrivateKey(skKey, SignerCurve.ED25519);
    } else if (skKey.startsWith("spsk")) {
      return WalletUtils().getKeysFromPrivateKey(skKey, SignerCurve.SECP256K1);
    }
    return [];
  }

  static Future<List<String>> unlockFundraiserIdentity({
    required String mnemonic,
    required String email,
    String passphrase = "",
  }) async {
    return await _unlockKeys(
      email: email,
      passphrase: passphrase,
      mnemonic: mnemonic,
    );
  }

  static String signPayload({
    required SoftSigner signer,
    required String payload,
  }) {
    var opSignature =
        signer.signOperation(Uint8List.fromList(hex.decode(payload)));
    var base58signature = TezosMessageUtils.readSignatureWithHint(
        opSignature, signer.getSignerCurve());
    return base58signature;
  }

  static Future<List<String>> signOperationGroup({
    required String privateKey,
    required String forgedOperation,
  }) async {
    String watermarkedForgedOperationBytesHex = '03' + forgedOperation;
    List<int> hexStringToListOfInt =
        hex.decode(watermarkedForgedOperationBytesHex);
    Uint8List hashedWatermarkedOpBytes =
        Blake2bHash.hashWithDigestSize(256, hexStringToListOfInt as Uint8List);
    Uint8List privateKeyBytes = bs58check.decode(privateKey);
    List<int> pkB = List.from(privateKeyBytes);
    pkB.removeRange(0, 4);
    Uint8List finalPKb = Uint8List.fromList(pkB);
    Uint8List value = sodiumUtils.cryptoSignDetached(
      hashedWatermarkedOpBytes,
      finalPKb,
    );
    String opSignatureHex = hex.encode(value);
    String hexStringToEncode = '09f5cd8612' + opSignatureHex;
    Uint8List hexDeco = hex.decode(hexStringToEncode) as Uint8List;
    String hexSignature = bs58check.encode(hexDeco);
    String signedOpBytes = forgedOperation + opSignatureHex;
    return [hexSignature, signedOpBytes];
  }

  static Future<List<String>> _unlockKeys({
    required String mnemonic,
    String passphrase = "",
    String email = "",
  }) async {
    List<int> stringNormalize(String stringToNormalize) {
      String normalizedString = unorm.nfkd(stringToNormalize);
      List<int> stringToBuffer = utf8.encode(normalizedString);
      return stringToBuffer;
    }

    List<int> mnemonicsBuffer = stringNormalize(mnemonic);
    String m = String.fromCharCodes(mnemonicsBuffer);
    List<int> normalizedPassphrase = stringNormalize("$email" + "$passphrase");
    String normString = String.fromCharCodes(normalizedPassphrase);
    String p = "mnemonic" + normString;
    Uint8List seed =
        PBKDF2(hashAlgorithm: sha512).generateKey(m, p, 2048, 32) as Uint8List;
    KeyPair keyPair = sodiumUtils.cryptoSignSeedKeypair(seed);
    String skKey = GenerateKeys.readKeysWithHint(keyPair.sk, '2bf64e07');
    String pkKey = GenerateKeys.readKeysWithHint(keyPair.pk, '0d0f25d9');
    String pkKeyHash = GenerateKeys.computeKeyHash(
        keyPair.pk, GenerateKeys.keyPrefixes[PrefixEnum.tz1]!);
    return [skKey, pkKey, pkKeyHash];
  }

  static Future<String> getBalance(String publicKeyHash, String rpc) async {
    var response = await HttpHelper.performGetRequest(rpc,
        'chains/main/blocks/head/context/contracts/$publicKeyHash/balance');
    return response.toString();
  }

  static Uint8List writeKeyWithHint(key, hint) {
    assert(key != null);
    assert(hint != null);
    return GenerateKeys.writeKeyWithHint(key, hint);
  }

  static String writeAddress(address) {
    //writes tz1 address in base58. Used in micheline while sending contract
    assert(address != null);
    return TezosMessageUtils.writeAddress(address);
  }

  static String normalizePrimitiveRecordOrder(String data) {
    return jsonEncode(
        TezosLanguageUtil.normalizePrimitiveRecordOrder(jsonDecode(data)));
  }

  static SoftSigner createSigner(Uint8List secretKey,
      {int validity = 60, SignerCurve signerCurve = SignerCurve.ED25519}) {
    return SoftSigner.createSigner(secretKey, validity,
        signerCurve: signerCurve);
  }

  static sendTransactionOperation(String server, SoftSigner signer,
      KeyStoreModel keyStore, String to, int amount, int fee,
      {int offset = 54, bool isKeyRevealed = false}) async {
    assert(keyStore.publicKey != null);
    assert(keyStore.secretKey != null);

    return await TezosNodeWriter.sendTransactionOperation(
      server,
      signer,
      keyStore,
      to,
      amount,
      fee,
      isKeyRevealed: isKeyRevealed,
    );
  }

  static sendDelegationOperation(String server, SoftSigner signer,
      KeyStoreModel keyStore, String delegate, int fee,
      {offset = 54}) async {
    assert(keyStore.publicKey != null);
    assert(keyStore.secretKey != null);
    assert(offset != null);
    if (fee == 0) fee = TezosConstants.DefaultDelegationFee;
    return await TezosNodeWriter.sendDelegationOperation(
        server, signer, keyStore, delegate, fee, offset);
  }

  static sendContractOriginationOperation(
    String server,
    SoftSigner signer,
    KeyStoreModel keyStore,
    int amount,
    String delegate,
    int fee,
    int storageLimit,
    int gasLimit,
    String code,
    String storage, {
    TezosParameterFormat codeFormat = TezosParameterFormat.Micheline,
    int offset = 54,
  }) async {
    assert(keyStore.publicKey != null);
    assert(keyStore.secretKey != null);
    return await TezosNodeWriter.sendContractOriginationOperation(
      server,
      signer,
      keyStore,
      amount,
      delegate,
      fee,
      storageLimit,
      gasLimit,
      code,
      storage,
      codeFormat,
      offset,
    );
  }

  static awaitOperationConfirmation(serverInfo, network, hash, duration,
      {blocktime}) async {
    assert(serverInfo != null);
    assert(network != null);
    assert(hash != null);
    assert(duration != null);
    return await TezosConseilClient.awaitOperationConfirmation(
        serverInfo, network, hash, duration,
        blocktime: blocktime);
  }

  static sendContractInvocationOperation(
    String server,
    SoftSigner signer,
    KeyStoreModel keyStore,
    List<String> contract,
    List<int> amount,
    int fee,
    int storageLimit,
    int gasLimit,
    List<String> entrypoint,
    List<String> parameters, {
    var codeFormat = TezosParameterFormat.Micheline,
    offset = 54,
  }) async {
    assert(keyStore.publicKey != null);
    assert(keyStore.secretKey != null);
    return await TezosNodeWriter.sendContractInvocationOperation(
        server,
        signer,
        keyStore,
        contract,
        amount,
        fee,
        storageLimit,
        gasLimit,
        entrypoint,
        parameters,
        parameterFormat: codeFormat ?? TezosParameterFormat.Michelson,
        offset: offset ?? 54);
  }

  static sendIdentityActivationOperation(String server, SoftSigner signer,
      KeyStoreModel keyStore, String activationCode) async {
    return await TezosNodeWriter.sendIdentityActivationOperation(
        server, signer, keyStore, activationCode);
  }

  static sendKeyRevealOperation(String server, signer, KeyStoreModel keyStore,
      {fee = TezosConstants.DefaultKeyRevealFee,
      offset = TezosConstants.HeadBranchOffset}) async {
    assert(signer != null);
    assert(fee != null);
    assert(offset != null);
    return await TezosNodeWriter.sendKeyRevealOperation(
        server, signer, keyStore, fee, offset);
  }

  static getContractStorage(String server, String accountHash) async {
    return await TezosNodeReader.getContractStorage(server, accountHash);
  }

  static encodeBigMapKey(Uint8List key) {
    return TezosMessageUtils.encodeBigMapKey(key);
  }

  static Uint8List writePackedData(String value, String type,
      {format = TezosParameterFormat.Micheline}) {
    assert(format != null);
    return Uint8List.fromList(
        hex.decode(TezosMessageUtils.writePackedData(value, type, format)));
  }

  static getValueForBigMapKey(String server, String index, String key,
      {block = 'head', chainid = 'main'}) async {
    return await TezosNodeReader.getValueForBigMapKey(server, index, key,
        block: 'head', chainid: 'main');
  }

  static injectOperation(String server, Map<String, dynamic> opPair) async {
    return await TezosNodeWriter.injectOperation(server, opPair);
  }

  static Future<Map<String, dynamic>> preapplyContractInvocationOperation(
    String server,
    SoftSigner signer,
    KeyStoreModel keyStore,
    List<String> contract,
    List<int> amount,
    int fee,
    int storageLimit,
    int gasLimit,
    List<String> entrypoint,
    List<String> parameters, {
    var codeFormat = TezosParameterFormat.Micheline,
    offset = 54,
  }) async {
    assert(keyStore.publicKey != null);
    assert(keyStore.secretKey != null);
    return await TezosNodeWriter.sendContractInvocationOperation(
        server,
        signer,
        keyStore,
        contract,
        amount,
        fee,
        storageLimit,
        gasLimit,
        entrypoint,
        parameters,
        parameterFormat: codeFormat ?? TezosParameterFormat.Michelson,
        offset: offset ?? 54,
        preapply: true,
        gasEstimation: true);
  }

  static Future<String?> getOperationStatus(
      String server, String opHash) async {
    return await OperationHelper().getOperationStatus(server, opHash);
  }

  static Future<String> getBlock(String server) async {
    return await OperationHelper().getBlock(server);
  }

  static Future<dynamic> sendContractInvocatoinOperation(
    String server,
    SoftSigner signer,
    KeyStoreModel keyStore,
    List<String> contract,
    List<int> amount,
    int fee,
    int storageLimit,
    int gasLimit,
    List<String> entrypoint,
    List<String> parameters, {
    var codeFormat = TezosParameterFormat.Micheline,
    offset = 54,
  }) async {
    assert(keyStore.publicKey != null);
    assert(keyStore.secretKey != null);

    return await TezosNodeWriter.sendContractInvocationOperation(
        server,
        signer,
        keyStore,
        contract,
        amount,
        fee,
        storageLimit,
        gasLimit,
        entrypoint,
        parameters,
        parameterFormat: codeFormat ?? TezosParameterFormat.Michelson,
        offset: offset ?? 54,
        preapply: false);
  }
}
