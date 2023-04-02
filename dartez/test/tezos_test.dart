import 'dart:io';
import 'dart:typed_data';

import 'package:convert/convert.dart';
import 'package:dartez/helper/generateKeys.dart';
import 'package:dartez/src/soft-signer/soft_signer.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:dartez/dartez.dart';
import 'package:secp256k1/secp256k1.dart';

class MyHttpOverrides extends HttpOverrides {
  @override
  HttpClient createHttpClient(SecurityContext? context) {
    return super.createHttpClient(context)
      ..badCertificateCallback =
          (X509Certificate cert, String host, int port) => true;
  }
}

void main() {
  HttpOverrides.global = new MyHttpOverrides();

  String server = "RPC_NODE";

  String testPrivateKey = "edskR.....iXezixvdC";
  String testForgedOperation =
      "713cb068fe3ac078351727eb5c34279e22b75b0cf4dc0a8d3d599e27031db136040cb9f9da085607c05cac1ca4c62a3f3cfb8146aa9b7f631e52f877a1d363474404da8130b0b940ee";
  String testMnemonics =
      "luxury bulb roast timber sense stove sugar sketch goddess host meadow decorate gather salmon funny person canoe daring machine network camp moment wrong dice";

  KeyStoreModel _keyStoreModel = KeyStoreModel(
    secretKey: "",
    publicKey: "",
    publicKeyHash: "",
  );

  test("Get balance", () async {
    String balance =
        await Dartez.getBalance(_keyStoreModel.publicKeyHash, server);
    expect(balance, isNotNull);
  });

  test("Generate Mnemonic", () async {
    String mnemonics = Dartez.generateMnemonic();
    expect(mnemonics, isNotNull);
  });

  test('Get Keys From Mnemonics', () async {
    List<String> keys =
        await Dartez.getKeysFromMnemonic(mnemonic: testMnemonics);
    expect(keys[0],
        "edskRdVS5H9YCRAG8yqZkX2nUTbGcaDqjYgopkJwRuPUnYzCn3t9ZGksncTLYe33bFjq29pRhpvjQizCCzmugMGhJiXezixvdC");
    expect(keys[1], "edpkuLog552hecagkykJ3fTvop6grTMhfZY4TWbvchDWdYyxCHcrQL");
    expect(keys[2], "tz1g85oYHLFKDpNfDHPeBUbi3S7pUsgCB28q");
  });

  test("Get Keys From Mnemonic and PassPhrase", () async {
    List<String> keys = await Dartez.getKeysFromMnemonicAndPassphrase(
        mnemonic: testMnemonics, passphrase: "");
    expect(keys, isNotNull);
  });

  test("Reveal an account", () async {
    var signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(_keyStoreModel.secretKey, 'edsk'));

    var result =
        await Dartez.sendKeyRevealOperation(server, signer, _keyStoreModel);
    expect(result['operationGroupID'], isNotNull);
  });

  test("Unlock fundraise identity", () async {
    List<String> identityFundraiser = await Dartez.unlockFundraiserIdentity(
        mnemonic: "your mnemonic",
        email: "test@example.com",
        passphrase: "pasw0rd");

    expect(identityFundraiser, isNotNull);
  });

  test("Restore identity from derivation path", () async {
    var derivationPath = '';

    List<String> keyStore = await Dartez.restoreIdentityFromDerivationPath(
        derivationPath, testMnemonics);

    expect(keyStore, isNotNull);
  });

  test("Get keys from secret key", () {
    var keys = Dartez.getKeysFromSecretKey(_keyStoreModel.secretKey ?? '');
    expect(keys, isNotNull);
  });

  test("Delegate an Account", () async {
    var signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(_keyStoreModel.secretKey, 'edsk'));

    var result = await Dartez.sendDelegationOperation(
      server,
      signer,
      _keyStoreModel,
      'tz1RVcUP9nUurgEJMDou8eW3bVDs6qmP5Lnc',
      15000,
    );

    expect(result['operationGroupID'], isNotNull);
    expect(result['appliedOp'], isNotNull);
  });

  test("Sign Operation Group", () async {
    List<String> signOperationGroup = await Dartez.signOperationGroup(
        privateKey: _keyStoreModel.secretKey ?? '', forgedOperation: "");

    expect(signOperationGroup, isNotNull);
  });

  test("Transfer Balance", () async {
    var signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(_keyStoreModel.secretKey, 'edsk'));

    var result = await Dartez.sendTransactionOperation(
      server,
      signer,
      _keyStoreModel,
      'tz1QQpKV6gd6VvGeSVddpXv85Y7mSJ4MVLdc',
      10,
      1500,
    );

    expect(result['operationGroupID'], isNotNull);
    expect(result['appliedOp'], isNotNull);
  });

  test("Send transaction operation", () async {
    var signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(_keyStoreModel.secretKey, 'edsk'));
    var to = "tz1QQpKV6gd6VvGeSVddpXv85Y7mSJ4MVLdc";
    var amount = (0.0001 * 1000000).ceil();
    var fee = 1500;
    var transactionOperation = await Dartez.sendTransactionOperation(
        server, signer, _keyStoreModel, to, amount, fee);
    expect(transactionOperation, isNotNull);
  });

  test("Pre apply contract invocation operation", () async {
    var signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(_keyStoreModel.secretKey, 'edsk'));

    var contract = ["KT1...fgH"];

    var parameters = ["parameters"];

    var opPair = Dartez.preapplyContractInvocationOperation(
        server,
        signer,
        _keyStoreModel,
        contract,
        [0],
        120000,
        1000,
        100000,
        ['transfer'],
        parameters);

    expect(opPair, isNotNull);
  });

  test("Get operation status", () {
    var opHash = '';

    var status = Dartez.getOperationStatus(server, opHash);

    expect(status, isNotNull);
  });

  test("Send contract invocation operation", () async {
    var signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(_keyStoreModel.secretKey, 'edsk'));

    var contract = ["KT1...fgH"];

    var parameters = ['parameters'];

    var invocation = Dartez.sendContractInvocatoinOperation(
        server,
        signer,
        _keyStoreModel,
        contract,
        [0],
        120000,
        1000,
        100000,
        ['transfer'],
        parameters);

    expect(invocation, isNotNull);
  });

  test("Sign payload", () {
    var signer = Dartez.createSigner(
        Dartez.writeKeyWithHint(_keyStoreModel.secretKey, 'edsk'));

    var payload =
        "05010000008654657a6f73205369676e6564204d6573736167653a20436f6e6669726d696e67206d79206964656e7469747920617320747a315151704b56366764365676476553566464705876383559376d534a344d564c6463206f6e206f626a6b742e636f6d2c207369673a595061474962567450584c6c33646668525145746b554a417749376b575055";

    String base58signature =
        Dartez.signPayload(signer: signer, payload: payload);

    expect(base58signature, isNotNull);
  });

  test("Encode BigMap Key", () {
    Uint8List key = Uint8List.fromList(
        [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]);

    var encodedKey = Dartez.encodeBigMapKey(key);

    expect(encodedKey, isNotNull);
  });

  test("Normalize primitive record order", () {
    var data =
        '{"prim":"Pair","args":[{"prim":"Pair","args":[{"prim":"Pair","args":[{"string":"KT1GpTEq4p2XZ8w9p5xM7Wayyw5VR7tb3UaW"},{"int":"3943"}]},{"int":"2208216"},{"int":"3944"}]},{"prim":"Pair","args":[{"prim":"False"},{"int":"3945"}]},{"int":"30000000000000000000"},{"int":"36133183489902370637585786"}]}';

    var recordOrder = Dartez.normalizePrimitiveRecordOrder(data);

    expect(recordOrder, isNotNull);
  });

  test("Get block", () async {
    var block = await Dartez.getBlock(server);

    expect(block, isNotNull);
  });

  test("Write address", () {
    var address = _keyStoreModel.publicKeyHash;

    var writeAddress = Dartez.writeAddress(address);

    expect(writeAddress, isNotNull);
  });

  test("Get contract storage", () async {
    var accountHash = "KT1K5Qw59FaiWYafNsYy1xYvk89NJr58rUPS";

    var storage = await Dartez.getContractStorage(server, accountHash);

    expect(storage, isNotNull);
  });

  test("Create new wallet tz2&tz1 prefix", () async {
    KeyStoreModel keyStoreModel =
        KeyStoreModel(publicKeyHash: "", secretKey: "", publicKey: "");

    var signer = Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStoreModel.secretKey, 'spsk'),
        signerCurve: SignerCurve.SECP256K1);

    var result = await Dartez.sendTransactionOperation(
      "",
      signer,
      keyStoreModel,
      '',
      10,
      1500,
    );

    expect(result['operationGroupID'], isNotNull);
    expect(result['appliedOp'], isNotNull);

    print(signer.getKey());
  });
}
