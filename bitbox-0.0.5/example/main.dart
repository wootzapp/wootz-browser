import 'package:bitbox/bitbox.dart' as Bitbox;

void main() async {
  // set this to false to use mainnet
  final testnet = true;

  // After running the code for the first time, depositing an amount to the address displayed in the console,
  // and waiting for confirmation, paste the generated mnemonic here,
  // so the code continues below with address withdrawal
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

  // if you are using testnet, set the appropriate rest api url before making
  // any API calls (like getting address or transaction details or broadcasting a transaction
  if (testnet) {
    Bitbox.Bitbox.setRestUrl(Bitbox.Bitbox.trestUrl);
  }

  // get address details
  final addressDetails = await Bitbox.Address.details(address);

  print(addressDetails);

  // If there is a confirmed balance, attempt to withdraw it to the address defined below
  if (addressDetails["balance"] > 0) {
    final builder = Bitbox.Bitbox.transactionBuilder(testnet: testnet);

    // retrieve address' utxos from the rest api
    final utxos = await Bitbox.Address.utxo(address) as List<Bitbox.Utxo>;

    // placeholder for input signatures
    final signatures = <Map>[];

    // placeholder for total input balance
    int totalBalance = 0;

    // iterate through the list of address utxos and use them as inputs for the withdrawal transaction
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
      final txid =
          (await Bitbox.RawTransactions.sendRawTransaction([tx.toHex()])).first;

      // Yatta!
      print("Transaction broadcasted: $txid");
    } else if (totalBalance > 0) {
      print("Enter an output address to test withdrawal transaction");
    }
  }
}
