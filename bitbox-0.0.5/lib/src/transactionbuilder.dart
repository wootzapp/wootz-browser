import 'dart:typed_data';
import 'package:hex/hex.dart';
import 'package:bs58check/bs58check.dart' as bs58check;
import 'address.dart';
import 'crypto/crypto.dart';
import 'utils/network.dart';
import 'utils/opcodes.dart';
import 'utils/p2pkh.dart';
import 'utils/script.dart' as bscript;
import 'ecpair.dart';
import 'transaction.dart';
import 'bitcoincash.dart';

/// Toolbox for creating a transaction, that can be broadcasted to BCH network. Works only as an instance created
/// through one of the factories or a constructor
class TransactionBuilder {
  static const DEFAULT_SEQUENCE = 0xffffffff;
  static const SIGHASH_ALL = 0x01;
  static const SIGHASH_NONE = 0x02;
  static const SIGHASH_SINGLE = 0x03;
  static const SIGHASH_ANYONECANPAY = 0x80;
  static const SIGHASH_BITCOINCASHBIP143 = 0x40;

  final Network _network;
  final int _maximumFeeRate;
  final List<Input> _inputs;
  final Transaction _tx;
  final Map _prevTxSet = {};

  /// Creates an empty transaction builder
  TransactionBuilder({Network network, int maximumFeeRate})
      : this._network = network ?? Network.bitcoinCash(),
        this._maximumFeeRate = maximumFeeRate ?? 2500,
        this._inputs = [],
        this._tx = new Transaction();

  /// Creates a builder from pre-built transaction
  factory TransactionBuilder.fromTransaction(Transaction transaction,
      [Network network]) {
    final txb = new TransactionBuilder(network: network);
    // Copy transaction fields
    txb.setVersion(transaction.version);
    txb.setLockTime(transaction.locktime);

    // Copy outputs (done first to avoid signature invalidation)
    transaction.outputs.forEach((txOut) {
      txb.addOutput(txOut.script, txOut.value);
    });

    // Copy inputs
    transaction.inputs.forEach((txIn) {
      txb._addInputUnsafe(txIn.hash, txIn.index,
          new Input(sequence: txIn.sequence, script: txIn.script));
    });

    return txb;
  }

  /// Sets version of the transaction. Throws [ArgumentError] if the [version] is incompatible
  setVersion(int version) {
    if (version < 0 || version > 0xFFFFFFFF) {
      throw ArgumentError("Expected Uint32");
    }
    _tx.version = version;
  }

  /// Sets transaction's locktime. Throw's [ArgumentError] if [locktime] is out of range
  setLockTime(int locktime) {
    if (locktime < 0 || locktime > 0xFFFFFFFF) {
      throw ArgumentError("Expected Uint32");
    }
    // if any signatures exist, throw
    if (this._inputs.map((input) {
      if (input.signatures == null) return false;
      return input.signatures.map((s) {
        return s != null;
      }).contains(true);
    }).contains(true)) {
      throw new ArgumentError('No, this would invalidate signatures');
    }

    _tx.locktime = locktime;
  }

  /// Add input from transaction data, which can be:
  /// * hash as a String
  /// * hash as a Uint8List
  /// * instance of [Transaction]
  ///
  /// Returns vin of the input
  ///
  /// Throws [ArgumentError] if the inputs of this transaction can't be modified or if [txHashOrInstance] is invalid
  int addInput(dynamic txHashOrInstance, int vout,
      [int sequence, Uint8List prevOutScript]) {
    assert(txHashOrInstance is String ||
        txHashOrInstance is Uint8List ||
        txHashOrInstance is Transaction);

    if (!_canModifyInputs()) {
      throw new ArgumentError('No, this would invalidate signatures');
    }
    Uint8List hash;
    var value;
    if (txHashOrInstance is String) {
      hash = Uint8List.fromList(HEX.decode(txHashOrInstance).reversed.toList());
    } else if (txHashOrInstance is Uint8List) {
      hash = txHashOrInstance;
    } else if (txHashOrInstance is Transaction) {
      final txOut = txHashOrInstance.outputs[vout];
      prevOutScript = txOut.script;
      value = txOut.value;
      hash = txHashOrInstance.getHash();
    } else {
      throw ArgumentError('txHash invalid');
    }

    return _addInputUnsafe(
        hash,
        vout,
        new Input(
            sequence: sequence, prevOutScript: prevOutScript, value: value));
  }

  /// Adds transaction output, which can be provided as:
  /// * Address as [String] in either _legacy_ or _cashAddr_ format
  /// * scriptPubKey
  ///
  /// Returns output id
  ///
  /// Throws [ArgumentError] if outputs can't be modified or the output format is invalid
  int addOutput(dynamic data, int value) {
    assert(data is String || data is Uint8List);

    Uint8List scriptPubKey;
    if (data is String) {
      if (Address.detectFormat(data) == Address.formatCashAddr) {
        data = Address.toLegacyAddress(data);
      }
      scriptPubKey = _addressToOutputScript(data, _network);
    } else if (data is Uint8List) {
      scriptPubKey = data;
    } else {
      throw ArgumentError('Address invalid');
    }

    if (!_canModifyOutputs()) {
      throw ArgumentError('No, this would invalidate signatures');
    }

    return _tx.addOutput(scriptPubKey, value);
  }

  /// Calculates byte count of this transaction. If [addChangeOutput] is true, it will arbitrarily add one output to
  /// indicate, that the developer plans to add change address later based on a result of this calculation
  ///
  /// Throws [ArgumentError] if something goes wrong
  int getByteCount([bool addChangeOutput = true]) => BitcoinCash.getByteCount(
      this._inputs.length, this._tx.outputs.length + (addChangeOutput ? 1 : 0));

  /// Add signature for the input [vin] using [keyPair] and with a specified [value]
  ///
  /// Throws [ArgumentError] if something goes wrong
  sign(int vin, ECPair keyPair, int value,
      [int hashType = Transaction.SIGHASH_ALL]) {
    hashType = hashType | Transaction.SIGHASH_BITCOINCASHBIP143;

    if (keyPair.network != null &&
        keyPair.network.toString().compareTo(_network.toString()) != 0) {
      throw ArgumentError('Inconsistent network');
    }

    if (vin >= _inputs.length) {
      throw ArgumentError('No input at index: $vin');
    }

    if (this._needsOutputs(hashType)) {
      throw ArgumentError('Transaction needs outputs');
    }

    final input = _inputs[vin];
    final ourPubKey = keyPair.publicKey;

    if (!_canSign(input)) {
      //      Uint8List prevOutScript = pubkeyToOutputScript(ourPubKey);
      _prepareInput(input, ourPubKey, value);
    }

    var signatureHash =
        this._tx.hashForCashSignature(vin, input.signScript, value, hashType);

    // enforce in order signing of public keys
    var signed = false;
    for (var i = 0; i < input.pubkeys.length; i++) {
      if (HEX.encode(ourPubKey).compareTo(HEX.encode(input.pubkeys[i])) != 0) {
        continue;
      }

      if (input.signatures[i] != null) {
        throw ArgumentError('Signature already exists');
      }

      final signature = keyPair.sign(signatureHash);
      input.signatures[i] = bscript.encodeSignature(signature, hashType);

      signed = true;
    }

    if (!signed) {
      throw ArgumentError('Key pair cannot sign for this input');
    }
  }

  /// Build signed transaction
  Transaction build() {
    return _build(false);
  }

  /// Untested so far
  Transaction buildIncomplete() {
    return _build(true);
  }

  _build(bool allowIncomplete) {
    if (!allowIncomplete) {
      if (_tx.inputs.length == 0)
        throw ArgumentError('Transaction has no inputs');
      if (_tx.outputs.length == 0)
        throw ArgumentError('Transaction has no outputs');
    }

    final tx = Transaction.clone(_tx);

    if (!allowIncomplete) {
      // do not rely on this, it is merely a last resort
      if (_overMaximumFees(tx.virtualSize())) {
        throw new ArgumentError('Transaction has absurd fees');
      }
    }

    for (var i = 0; i < _inputs.length; i++) {
      if (_inputs[i].pubkeys != null &&
          _inputs[i].signatures != null &&
          _inputs[i].pubkeys.length != 0 &&
          _inputs[i].signatures.length != 0) {
        final result = _buildInput(_inputs[i]);
        tx.setInputScript(i, result);
      } else if (!allowIncomplete) {
        throw new ArgumentError('Transaction is not complete');
      }
    }
    return tx;
  }

  bool _overMaximumFees(int bytes) {
    int incoming = _inputs.fold(0, (cur, acc) => cur + (acc.value ?? 0));
    int outgoing = _tx.outputs.fold(0, (cur, acc) => cur + (acc.value ?? 0));
    int fee = incoming - outgoing;
    int feeRate = fee ~/ bytes;
    return feeRate > _maximumFeeRate;
  }

  bool _canModifyInputs() {
    return _inputs.every((input) {
      if (input.signatures == null) return true;
      return input.signatures.every((signature) {
        if (signature == null) return true;
        return _signatureHashType(signature) & SIGHASH_ANYONECANPAY != 0;
      });
    });
  }

  bool _canModifyOutputs() {
    final nInputs = _tx.inputs.length;
    final nOutputs = _tx.outputs.length;
    return _inputs.every((input) {
      if (input.signatures == null) return true;
      return input.signatures.every((signature) {
        if (signature == null) return true;
        final hashType = _signatureHashType(signature);
        final hashTypeMod = hashType & 0x1f;
        if (hashTypeMod == SIGHASH_NONE) return true;
        if (hashTypeMod == SIGHASH_SINGLE) {
          // if SIGHASH_SINGLE is set, and nInputs > nOutputs
          // some signatures would be invalidated by the addition
          // of more outputs
          return nInputs <= nOutputs;
        }
        return false;
      });
    });
  }

  bool _needsOutputs(int signingHashType) {
    if (signingHashType == SIGHASH_ALL) {
      return this._tx.outputs.length == 0;
    }
    // if inputs are being signed with SIGHASH_NONE, we don't strictly need outputs
    // .build() will fail, but .buildIncomplete() is OK
    return (this._tx.outputs.length == 0) &&
        _inputs.map((input) {
          if (input.signatures == null || input.signatures.length == 0)
            return false;
          return input.signatures.map((signature) {
            if (signature == null) return false; // no signature, no issue
            final hashType = _signatureHashType(signature);
            if (hashType & SIGHASH_NONE != 0)
              return false; // SIGHASH_NONE doesn't care about outputs
            return true; // SIGHASH_* does care
          }).contains(true);
        }).contains(true);
  }

  bool _canSign(Input input) {
    return input.pubkeys != null &&
        input.signScript != null &&
        input.signatures != null &&
        input.signatures.length == input.pubkeys.length &&
        input.pubkeys.length > 0;
  }

  _addInputUnsafe(Uint8List hash, int vout, Input options) {
    String txHash = HEX.encode(hash);
    Input input;
    if (_isCoinbaseHash(hash)) {
      throw new ArgumentError('coinbase inputs not supported');
    }

    final prevTxOut = '$txHash:$vout';
    if (_prevTxSet[prevTxOut] != null)
      throw new ArgumentError('Duplicate TxOut: ' + prevTxOut);
    if (options.script != null) {
      input = Input.expandInput(options.script);
    } else {
      input = new Input();
    }

    if (options.value != null) input.value = options.value;
    if (input.prevOutScript == null && options.prevOutScript != null) {
      input.prevOutScript = options.prevOutScript;
    }
    int vin = _tx.addInput(hash, vout, options.sequence);

    _inputs.add(input);
    _prevTxSet[prevTxOut] = true;

    return vin;
  }

  int _signatureHashType(Uint8List buffer) {
    return buffer.buffer.asByteData().getUint8(buffer.length - 1);
  }

  Transaction get tx => _tx;

  Map get prevTxSet => _prevTxSet;

  Input _prepareInput(Input input, Uint8List kpPubKey, int value) {
    final prevOutScript = bscript.compile([
      Opcodes.OP_DUP,
      Opcodes.OP_HASH160,
      hash160(kpPubKey),
      Opcodes.OP_EQUALVERIFY,
      Opcodes.OP_CHECKSIG
    ]);

    final expanded = _expandOutput(prevOutScript, kpPubKey);

    input.pubkeys = expanded.pubkeys;
    input.signatures = expanded.signatures;
    input.signScript = prevOutScript;
    input.prevOutScript = prevOutScript;
    return input;
  }

  // returns input script
  Uint8List _buildInput(Input input) {
    // this is quite rudimentary for P2PKH purposes
    return bscript.compile([input.signatures.first, input.pubkeys.first]);
  }

  Output _expandOutput(Uint8List script, Uint8List ourPubKey) {
    if (_isP2PKHOutput(script) == false) {
      //TODO: implement other script types too
      throw ArgumentError("Unsupport script!");
    }

    final scriptChunks = bscript.decompile(script);

    // does our hash160(pubKey) match the output scripts?
    Uint8List pkh1 = scriptChunks[
        2]; //new P2PKH(data: new P2PKHData(output: script)).data.hash;
    Uint8List pkh2 = hash160(ourPubKey);

    // this check should work, but for some reason doesn't - it returns false even if both lists are the same
    // TODO: debug and re-enable this validation
//    if (pkh1 != pkh2) throw ArgumentError("Hash mismatch!");

    return new Output(pubkeys: [ourPubKey], signatures: [null]);
  }

  Uint8List _addressToOutputScript(String address, [Network nw]) {
    final network = nw ?? Network.bitcoinCash();
    final payload = bs58check.decode(address);
    if (payload.length < 21) throw ArgumentError(address + ' is too short');
    if (payload.length > 21) throw ArgumentError(address + ' is too long');
    final p2pkh = P2PKH(data: P2PKHData(address: address), network: network);
    return p2pkh.data.output;
  }

  Uint8List _pubkeyToOutputScript(Uint8List pubkey, [Network nw]) {
    final network = nw ?? Network.bitcoinCash();
    final p2pkh = P2PKH(data: P2PKHData(pubkey: pubkey), network: network);
    return p2pkh.data.output;
  }

  Uint8List _toInputScript(Uint8List pubkey, Uint8List signature,
      [Network nw]) {
    final network = nw ?? Network.bitcoinCash();
    final p2pkh = P2PKH(
        data: P2PKHData(pubkey: pubkey, signature: signature),
        network: network);
    return p2pkh.data.input;
  }

  bool _isP2PKHOutput(script) {
    final buffer = bscript.compile(script);
    return buffer.length == 25 &&
        buffer[0] == Opcodes.OP_DUP &&
        buffer[1] == Opcodes.OP_HASH160 &&
        buffer[2] == 0x14 &&
        buffer[23] == Opcodes.OP_EQUALVERIFY &&
        buffer[24] == Opcodes.OP_CHECKSIG;
  }

  bool _isCoinbaseHash(Uint8List buffer) {
    if (buffer.length != 32) throw new ArgumentError("Invalid hash");
    for (var i = 0; i < 32; ++i) {
      if (buffer[i] != 0) return false;
    }
    return true;
  }
}
