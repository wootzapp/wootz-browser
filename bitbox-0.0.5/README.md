# Bitbox lite

Lite version of [Bitcoin.com's Bitbox JS library](https://developer.bitcoin.com/bitbox/) for building Bitcoin Cash apps in Flutter. Integrated to Bitcoin.com's REST API.
Works with mainnet and testnet.

## Getting Started

### 1) Depend on it
If you just want to get this from Dart's public package directory:

```
dependencies:
  bitbox: ^0.0.1
```

If you checked this out from Github, add a local dependency into the pubspec.yaml of your testing or development projet:

```
dependencies:
  bitbox_plugin:
    path: <path to the directory>/
```
	
### 2) Import it

```
// There's a good chance your own project will use similar names as some of the 
// classes in this library. A simple way to create some order is to import the 
// library with Bitbox prefix:
import 'package:bitbox/bitbox.dart' as Bitbox;
```

### 2) Use it
```
// set this to true to use testnet
final testnet = true;

// After running the code for the first time, depositing an amount to the address 
// displayed in the console, and waiting for confirmation, paste the generated 
// mnemonic here, so the code continues below with address withdrawal
String mnemonic = "";

if (mnemonic == "") {
  // generate 12-word (128bit) mnemonic
  mnemonic = Bitbox.Mnemonic.generate();

  print(mnemonic);
}

// generate a seed from mnemonic
final seed = Bitbox.Mnemonic.toSeed(mnemonic);

// create an instance of Bitbox.HDNode for mainnet
final masterNode = Bitbox.HDNode.fromSeed(seed, testnet);

// This format is compatible with Bitcoin.com wallet.
// Other wallets use Change to m/44'/145'/0'/0
final accountDerivationPath = "m/44'/0'/0'/0";

// create an account node using the provided derivation path
final accountNode = masterNode.derivePath(accountDerivationPath);

// get account's extended private key
final accountXPriv = accountNode.toXPriv();

// create a Bitbox.HDNode instance of the first child in this account
final childNode = accountNode.derive(0);

// get an address of the child
final address = childNode.toCashAddress();

// if you are using testnet, set the appropriate rest api url before making any API
// calls (like getting address or transaction details or broadcasting a transaction
if (testnet) {
  Bitbox.Bitbox.setRestUrl(restUrl: Bitbox.Bitbox.trestUrl);
}

// get address details
final addressDetails = await Bitbox.Address.details(address);

print(addressDetails);

// If there is a confirmed balance, attempt to withdraw it
if (addressDetails["balance"] > 0) {
  final builder = Bitbox.Bitbox.transactionBuilder(testnet: testnet);

  // retrieve address' utxos from the rest api
  final utxos = await Bitbox.Address.utxo(address) as List<Bitbox.Utxo>;

  // placeholder for input signatures
  final signatures = <Map>[];

  // placeholder for total input balance
  int totalBalance = 0;

  // iterate through the list of address utxos and use them as inputs for the 
  // withdrawal transaction
  utxos.forEach((Bitbox.Utxo utxo) {
    // add the utxo as an input for the transaction
    builder.addInput(utxo.txid, utxo.vout);

    // add a signature to the list to be used later
    signatures.add({
      "vin": signatures.length,
      "key_pair": childNode.keyPair,
      "original_amount": utxo.satoshis
    });

    totalBalance += utxo.satoshis;
  });

  // set an address to send the remaining balance to
  final outputAddress = "";

  // if there is an unspent balance, create a spending transaction
  if (totalBalance > 0 && outputAddress != "") {
    // calculate the fee based on number of inputs and one expected output
    final fee = Bitbox.BitcoinCash.getByteCount(signatures.length, 1);

    // calculate how much balance will be left over to spend after the fee
    final sendAmount = totalBalance - fee;

    // add the output based on the address provided in the testing data
    builder.addOutput(outputAddress, sendAmount);

    // sign all inputs
    signatures.forEach((signature) {
      builder.sign(signature["vin"], signature["key_pair"], 
	    signature["original_amount"]);
    });

    // build the transaction
    final tx = builder.build();

    // broadcast the transaction
    final txid = await Bitbox.RawTransactions.sendRawTransaction(tx.toHex());

    // Yatta!
    print("Transaction broadcasted: $txid");
  } else if (totalBalance > 0) {
    print("Enter an output address to test withdrawal transaction");
  }
}
```

For further documentation, refer to apidoc of this repository

## Testing

There are some unit tests in test/bitbox_test.dart. They use data generated from the original [Bitbox for JS](https://developer.bitcoin.com/bitbox/) and compare them with the output of this library. 
The following is tested for both testnet and mainnet:
- Generating the master node from mnemonic and comparing both its XPub and XPriv
- Generating an account node and comparing XPub and XPriv
- Generating 10 test childs and comparing their private keys and addresses
- Conversion of cashAddr format to legacy and vice versa
- Fetching address details for the rest API
- Fetching utxos for addresses with balance
- Building a transaction spending the deposited balance and comparing its hash with Bitbox JS output
- Optionally broadcasting the transaction and

To run the test:

1. Copy create_test_data.js to a separate directory and download the original Bitbox JS into the directory
2. Generate the testing data by runing create_test_data.js with your local nodeJS engine
3. Update bitbox_test.dart with the path to the generated test_data.json file
4. Run bitbox_test.dart 
Optionally between step 1) and 2), send some balance to either testnet or mainnet addresses (or both), wait for confirmations and run create_test_data.js again to update the data and generate testing transactions


## Acknowledgments

This is a port of the original JS-based Bitbox library by Gabriel Cardona and Bitcoin.com, so first of all huge thanks to Gabriel and the whole Bitcoin.com team for doing so much for the BCH ecosystem.

Also I either re-used a lot of code originally wrote for Bitcoin or called some libraries (bip39 and bip32) by [anicdh](https://github.com/anicdh), so Thanks big time to him. Without that it would take me many more weeks!
