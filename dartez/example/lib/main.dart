// All the functions are called in initState(){}
// For reference please check Debug console for outputs.
// Just run the project you must see the print statement outputs in debug console. It may take few seconds to reflect the output.

// NOTE: please get the dartez package under pubspec.yaml before running the project

import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:dartez/dartez.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  tezosWalletUtil() async {
    await Dartez().init();

    var server = '';

    // Get Balance.
    String balance =
        await Dartez.getBalance('tz1c....ozGGs', 'your rpc server');
    print("Accoutn Balance ===> $balance");

    // Generate mnemonics.
    String mnemonic = Dartez
        .generateMnemonic(); // strength is optional, by default it's 256 ==> Generates 24 words.
    print("mnemonic ===> $mnemonic");
    //mnemonic ===> 24 random words, [If strength parameter is changed the words length differs.]

    // Generate keys from mnemonic.
    List<String> keys = await Dartez.getKeysFromMnemonic(
      mnemonic: mnemonic,
    );
    print("keys ===> $keys");
    //keys ===> [privateKey, publicKey, publicKeyHash]
    //Accessing: private key ===> keys[0] | public key ===> keys[1] | public Key Hash ===> identity[2] all of type string

    // Generate keys from mnemonics and passphrase.
    List<String> identity = await Dartez.getKeysFromMnemonicAndPassphrase(
      mnemonic: "cannon rabbit ..... plunge winter",
      passphrase: "",
    );
    print("identity ===> $identity");
    // identityWithMnemonic ===> [privateKey, publicKey, publicKeyHash]
    // Accessing: private key ===> identity[0] | public key ===> identity[1] | public Key Hash ===> identity[2] all of type string.

    // Reveal an account

    var keyStore = KeyStoreModel(
      publicKeyHash: 'tz1U.....W5MHgi',
      secretKey: 'edskRp......bL2B6g',
      publicKey: 'edpktt.....U1gYJu2',
    );

    var signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    var result = await Dartez.sendKeyRevealOperation(server, signer, keyStore);

    print('${result['operationGroupID']}');

    // Activating a fundraiser account

    var faucetKeyStore = KeyStoreModel(
      publicKeyHash: 'tz1ga.....trZNA6A',
      seed: [
        "wife",
        "filter",
        "wage",
        "thunder",
        "forget",
        "scale",
        "punch",
        "mammal",
        "offer",
        "car",
        "cash",
        "defy",
        "vehicle",
        "romance",
        "green"
      ],
      secret: '',
      email: '',
      password: '',
    );

    var unloackedIdentityKeys = await Dartez.unlockFundraiserIdentity(
        email: faucetKeyStore.email ?? '',
        passphrase: faucetKeyStore.password ?? '',
        mnemonic: faucetKeyStore.seed.join(' '));

    faucetKeyStore
      ..publicKey = unloackedIdentityKeys[1]
      ..secretKey = unloackedIdentityKeys[0]
      ..publicKeyHash = unloackedIdentityKeys[2];

    var activationOperationSigner = await Dartez.createSigner(
        Dartez.writeKeyWithHint(faucetKeyStore.secretKey, 'edsk'));

    var activationOperationResult =
        await Dartez.sendIdentityActivationOperation(
            server,
            activationOperationSigner,
            faucetKeyStore,
            faucetKeyStore.secret ?? '');

    print('${activationOperationResult['operationGroupID']}');

    // Unlock fundraiser identity.
    List<String> identityFundraiser = await Dartez.unlockFundraiserIdentity(
      mnemonic: "cannon rabbit ..... plunge winter",
      email: "lkbpoife.tobqgidu@tezos.example.org",
      passphrase: "5tj...imq",
    );
    print("identityFundraiser ===> $identityFundraiser");
    //identityFundraiser ===> [privateKey, publicKey, publicKeyHash]
    //Accessing: private key ===> identityFundraiser[0] | public key ===> identityFundraiser[1] | public Key Hash ===> identityFundraiser[2] all of type string.

    // restoreIdentityFromDerivationPath
    var derivationPath = 'your derivation path';

    List<String> listkeyStore = await Dartez.restoreIdentityFromDerivationPath(
        derivationPath, mnemonic);                   

    /* [edskRzNDm2dpqe2yd5zYAw1vmjr8sAwMubfcXajxdCNNr4Ud39BoppeqMAzoCPmb14mzfXRhjtydQjCbqU2VzWrsq6JP4D9GVb,
        edpkvASxrq16v5Awxpz4XPTA2d6QFaCL8expPrPNcVgVbWxT84Kdw2,
        tz1hhkSbaocSWm3wawZUuUdX57L3maSH16Pv] */

    // getKeysFromSecretKey
    var restoredKeys = Dartez.getKeysFromSecretKey("eds.....vdC");
    print("Restored account keys ===> $restoredKeys");
    // restoredKeys ===> [privateKey, publicKey, publicKeyHash]
    // Accessing: private key ===> restoredKeys[0] | public key ===> restoredKeys[1] | public Key Hash ===> restoredKeys[2] all of type string.

    // Delegate an Account.
    server = '';

    keyStore = KeyStoreModel(
      publicKey: 'edp......rrj',
      secretKey: 'eds.....yHH',
      publicKeyHash: 'tz1.....hxy',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    result = await Dartez.sendDelegationOperation(
      server,
      signer,
      keyStore,
      'tz1.....Lnc',
      10000,
    );

    print("Applied operation ===> $result['appliedOp']");
    print("Operation groupID ===> $result['operationGroupID']");

    // Sign Operation Group.
    List<String> signOperationGroup = await Dartez.signOperationGroup(
        privateKey: "edskRdV..... .XezixvdA",
        forgedOperation: "713cb068fe.... .b940ee");
    print(signOperationGroup);
    //signOperationGroup ===> [hexSignature, signedOpBytes]
    //Accessing: hex signature ===> signOpGrp[0] | signed Operation bytes ===> signOpGrp[1] all of type string

    // Transfer Balance.
    server = '';

    keyStore = KeyStoreModel(
      publicKey: 'edp.....rrj',
      secretKey: 'eds.....yHH',
      publicKeyHash: 'tz1.....hxy',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    result = await Dartez.sendTransactionOperation(
      server,
      signer,
      keyStore,
      'tz1.....Lnc',
      500000,
      1500,
    );

    print("Applied operation ===> $result['appliedOp']");
    print("Operation groupID ===> $result['operationGroupID']");

    // Deploy a contract.
    server = '';

    var contract = """parameter string;
    storage string;
    code { DUP;
        DIP { CDR ; NIL string ; SWAP ; CONS } ;
        CAR ; CONS ;
        CONCAT;
        NIL operation; PAIR}""";

    var storage = '"Sample"';

    keyStore = KeyStoreModel(
      publicKey: 'edp.....rrj',
      secretKey: 'eds.....yHH',
      publicKeyHash: 'tz1.....hxy',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    result = await Dartez.sendContractOriginationOperation(
      server,
      signer,
      keyStore,
      0,
      '',
      100000,
      1000,
      100000,
      contract,
      storage,
      codeFormat: TezosParameterFormat.Michelson,
    );

    print("Operation groupID ===> $result['operationGroupID']");

    // Call a contract.
    server = '';

    keyStore = KeyStoreModel(
      publicKey: 'edp.....rrj',
      secretKey: 'eds.....yHH',
      publicKeyHash: 'tz1.....hxy',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    var contractAddress = ['KT1.....xMY'];

    var resultInvoke = await Dartez.sendContractInvocationOperation(
        server,
        signer,
        keyStore,
        contractAddress,
        [10000],
        100000,
        1000,
        100000,
        [''],
        ["Cryptonomicon"],
        codeFormat: TezosParameterFormat.Michelson);

    print("Operation groupID ===> $result['operationGroupID']");

    // Operation confirmation.
    server = '';

    var network = 'carthagenet';

    var serverInfo = {'url': '', 'apiKey': '', 'network': network};

    contract = """parameter string;
    storage string;
    code { DUP;
        DIP { CDR ; NIL string ; SWAP ; CONS } ;
        CAR ; CONS ;
        CONCAT;
        NIL operation; PAIR}""";

    storage = '"Sample"';

    keyStore = KeyStoreModel(
      publicKey: 'edp....rrj',
      secretKey: 'eds.....yHH',
      publicKeyHash: 'tz1.....hxy',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    result = await Dartez.sendContractOriginationOperation(
      server,
      signer,
      keyStore,
      0,
      '',
      100000,
      1000,
      100000,
      contract,
      storage,
      codeFormat: TezosParameterFormat.Michelson,
    );

    print("Operation groupID ===> $result['operationGroupID']");

    var groupId = result['operationGroupID'];

    var conseilResult = await Dartez.awaitOperationConfirmation(
        serverInfo, network, groupId, 5);

    print('Originated contract at ${conseilResult['originated_contracts']}');

    // sendTransactionOperation
    server = '';

    keyStore = KeyStoreModel(
      publicKeyHash: 'tz1U.....W5MHgi',
      secretKey: 'edskRp......bL2B6g',
      publicKey: 'edpktt.....U1gYJu2',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));
    var to = "tz1....VLdc";
    var amount = (12.5 * 1000000).ceil(); // sending 12.5 tez
    var fee = 1500;
    var transactionOperation = await Dartez.sendTransactionOperation(
        server, signer, keyStore, to, amount, fee);

    // preapplyContractInvocationOperation
    server = '';

    keyStore = KeyStoreModel(
      publicKeyHash: 'tz1U.....W5MHgi',
      secretKey: 'edskRp......bL2B6g',
      publicKey: 'edpktt.....U1gYJu2',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    var contracts = ["KT1...fgH"];

    var parameters = ["parameters"];

    Map<String, dynamic> opPair =
        await Dartez.preapplyContractInvocationOperation(
            server,
            signer,
            keyStore,
            contracts,
            [0],
            120000,
            1000,
            100000,
            ['transfer'],
            parameters);

    // injectOperation
    server = '';

    keyStore = KeyStoreModel(
      publicKeyHash: 'tz1U.....W5MHgi',
      secretKey: 'edskRp......bL2B6g',
      publicKey: 'edpktt.....U1gYJu2',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    contracts = ["KT1...fgH"];

    parameters = ["parameters"];

    opPair = await Dartez.preapplyContractInvocationOperation(
        server,
        signer,
        keyStore,
        contracts,
        [0],
        120000,
        1000,
        100000,
        ['transfer'],
        parameters);

    var opHash = await Dartez.injectOperation(server, opPair);

    // getOperationStatus
    server = '';
    opHash = '';

    var status = Dartez.getOperationStatus(server, opHash);

    // sendContractInvocatoinOperation
    server = '';

    keyStore = KeyStoreModel(
      publicKeyHash: 'tz1U.....W5MHgi',
      secretKey: 'edskRp......bL2B6g',
      publicKey: 'edpktt.....U1gYJu2',
    );

    signer = await Dartez.createSigner(
        Dartez.writeKeyWithHint(keyStore.secretKey, 'edsk'));

    contracts = ["KT1...fgH"];

    parameters = ['parameters'];

    var invocation = Dartez.sendContractInvocatoinOperation(
        server,
        signer,
        keyStore,
        contracts,
        [0],
        120000,
        1000,
        100000,
        ['transfer'],
        parameters);

    // signPayload
    signer = Dartez.createSigner(Dartez.writeKeyWithHint('secretKey', 'edsk'));

    var payload = "03...";

    String base58signature =
        Dartez.signPayload(signer: signer, payload: payload);

    // encodeBigMapKey
    Uint8List key = Uint8List.fromList(
        [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]);

    var encodedKey = Dartez.encodeBigMapKey(key);

    // writePackedData
    var value = "10";
    var type = "int";

    var packedData = Dartez.writePackedData(value, type);

    // getValueForBigMapKey
    server = '';

    var bigMapId = '0';

    var stringKey = '';

    var bigMapValue = Dartez.getValueForBigMapKey(server, bigMapId, stringKey);

    // normalizePrimitiveRecordOrder
    var data = 'MICHELINE_CODE';

    var recordOrder = Dartez.normalizePrimitiveRecordOrder(data);

    // getBlock
    server = '';

    var block = await Dartez.getBlock(server);

    // writeAddress
    var address = 'tz1.....VLdc';

    var writeAddress = Dartez.writeAddress(address);

    // getContractStorage
    server = '';

    var accountHash = 'tz1.....VLdc';

    storage = Dartez.getContractStorage(server, accountHash);
  }

  @override
  void initState() {
    super.initState();
    tezosWalletUtil();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: Scaffold(
        body: Padding(
          padding: EdgeInsets.all(8.0),
          child: Center(
            child: Text(
              "Welcome to Dartez package.\n Please check the debug console for the outputs",
              textAlign: TextAlign.center,
            ),
          ),
        ),
      ),
    );
  }
}
