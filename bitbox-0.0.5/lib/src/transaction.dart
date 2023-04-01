import 'dart:typed_data';
import 'package:bitbox/src/utils/rest_api.dart';
import 'package:hex/hex.dart';
import 'utils/p2pkh.dart' show P2PKH, P2PKHData;
import 'crypto/crypto.dart' as bcrypto;
import 'utils/script.dart' as bscript;
import 'utils/opcodes.dart';
import 'utils/check_types.dart';
import 'varuint.dart' as varuint;

/// Container for storing and working with Bitcoin Cash transaction data
///
/// Heavily based on https://github.com/anicdh/bitcoin_flutter/blob/master/lib/src/transaction.dart
/// and updated to work with BCH
class Transaction {
  static const DEFAULT_SEQUENCE = 0xffffffff;
  static const SIGHASH_ALL = 0x01;
  static const SIGHASH_NONE = 0x02;
  static const SIGHASH_SINGLE = 0x03;
  static const SIGHASH_ANYONECANPAY = 0x80;
  static const SIGHASH_BITCOINCASHBIP143 = 0x40;
  static const ADVANCED_TRANSACTION_MARKER = 0x00;
  static const ADVANCED_TRANSACTION_FLAG = 0x01;
  static final emptyScript = Uint8List.fromList([]);
  static final zero = HEX.decode(
      '0000000000000000000000000000000000000000000000000000000000000000');
  static final one = HEX.decode(
      '0000000000000000000000000000000000000000000000000000000000000001');
  static final valueUint64Max = HEX.decode('ffffffffffffffff');
  static final blankOutput =
      Output(script: emptyScript, valueBuffer: valueUint64Max);
  static const SATOSHI_MAX = 21 * 1e14;

  int version;
  int locktime;
  List<Input> inputs;
  List<Output> outputs;

  /// If [inputs] or [outputs] are not defined, empty lists are created for each
  Transaction([version = 2, locktime = 0, ins, outs])
      : version = version,
        locktime = locktime,
        inputs = ins ?? <Input>[],
        outputs = outs ?? <Output>[];

  /// Creates transaction from its hex representation
  factory Transaction.fromHex(String hex) {
    return Transaction.fromBuffer(HEX.decode(hex));
  }

  /// Creates transaction from its hex representation stored in list of integers
  factory Transaction.fromBuffer(Uint8List buffer) {
    var offset = 0;
    ByteData bytes = buffer.buffer.asByteData();
    Uint8List readSlice(n) {
      offset += n;
      return buffer.sublist(offset - n, offset);
    }

    int readUInt32() {
      final i = bytes.getUint32(offset, Endian.little);
      offset += 4;
      return i;
    }

    int readInt32() {
      final i = bytes.getInt32(offset, Endian.little);
      offset += 4;
      return i;
    }

    int readUInt64() {
      final i = bytes.getUint64(offset, Endian.little);
      offset += 8;
      return i;
    }

    int readVarInt() {
      final vi = varuint.decode(buffer, offset);
      offset += varuint.encodingLength(vi);
      return vi;
    }

    Uint8List readVarSlice() {
      return readSlice(readVarInt());
    }

    final tx = new Transaction(readInt32(), readUInt32());

    final vinLen = readVarInt();
    for (var i = 0; i < vinLen; ++i) {
      tx.inputs.add(new Input(
          hash: readSlice(32),
          index: readUInt32(),
          script: readVarSlice(),
          sequence: readUInt32()));
    }
    final voutLen = readVarInt();
    for (var i = 0; i < voutLen; ++i) {
      tx.outputs.add(new Output(value: readUInt64(), script: readVarSlice()));
    }

    if (offset != buffer.length)
      throw new ArgumentError('Transaction has unexpected data');
    return tx;
  }

  /// Returns details about a Transaction or Transactions
  ///
  /// [txIds] can be either [String] or [List] of Strings, otherwise throws [TypeError]
  ///
  /// If [returnAsMap] is true, it will return [Map] with txid used as key. Otherwise it will return [List]
  static details(txIds, [bool returnAsMap = false]) async {
    assert(txIds is String || txIds is List<String>);

    if (txIds is String) {
      return await RestApi.sendGetRequest("transaction/details", txIds) as Map;
    } else if (txIds is List<String>) {
      return await RestApi.sendPostRequest(
          "transaction/details", "txids", txIds,
          returnKey: returnAsMap ? "txid" : null);
    } else {
      throw TypeError();
    }
  }

  bool isCoinbaseHash(buffer) {
    assert(buffer.length == 32);

    for (var i = 0; i < 32; ++i) {
      if (buffer[i] != 0) return false;
    }
    return true;
  }

  bool isCoinbase() {
    return inputs.length == 1 && isCoinbaseHash(inputs[0].hash);
  }

  /// Add input to the transaction. If [sequence] is not provided, defaults to [DEFAULT_SEQUENCE]
  int addInput(Uint8List hash, int index, [int sequence, Uint8List scriptSig]) {
    inputs.add(new Input(
        hash: hash,
        index: index,
        sequence: sequence ?? DEFAULT_SEQUENCE,
        script: scriptSig ?? emptyScript));
    return inputs.length - 1;
  }

  /// Add input to the transaction
  int addOutput(Uint8List scriptPubKey, int value) {
    outputs.add(new Output(script: scriptPubKey, value: value));
    return outputs.length - 1;
  }

  setInputScript(int index, Uint8List scriptSig) {
    inputs[index].script = scriptSig;
  }

  /// Create hash for legacy signature
  hashForSignature(int inIndex, Uint8List prevOutScript, int hashType) {
    if (inIndex >= inputs.length) return one;
    // ignore OP_CODESEPARATOR
    final ourScript =
        bscript.compile(bscript.decompile(prevOutScript).where((x) {
      return x != Opcodes.OP_CODESEPARATOR;
    }).toList());
    final txTmp = Transaction.clone(this);
    // SIGHASH_NONE: ignore all outputs? (wildcard payee)
    if ((hashType & 0x1f) == SIGHASH_NONE) {
      txTmp.outputs = [];
      // ignore sequence numbers (except at inIndex)
      for (var i = 0; i < txTmp.inputs.length; i++) {
        if (i != inIndex) {
          txTmp.inputs[i].sequence = 0;
        }
      }

      // SIGHASH_SINGLE: ignore all outputs, except at the same index?
    } else if ((hashType & 0x1f) == SIGHASH_SINGLE) {
      // https://github.com/bitcoin/bitcoin/blob/master/src/test/sighash_tests.cpp#L60
      if (inIndex >= outputs.length) return one;

      // truncate outputs after
      txTmp.outputs.length = inIndex + 1;

      // "blank" outputs before
      for (var i = 0; i < inIndex; i++) {
        txTmp.outputs[i] = blankOutput;
      }
      // ignore sequence numbers (except at inIndex)
      for (var i = 0; i < txTmp.inputs.length; i++) {
        if (i != inIndex) {
          txTmp.inputs[i].sequence = 0;
        }
      }
    }
  }

  /// Create a hash for Bitcoin Cash signature. If this is legacy format transaction however, it will revert to the
  /// legacy signature
  ///
  /// [amount] must not be null for BCH signatures
  hashForCashSignature(
      int inIndex, Uint8List prevOutScript, int amount, int hashType) {
    if ((hashType & SIGHASH_BITCOINCASHBIP143) > 0) {
      if (amount == null) {
        throw ArgumentError(
            'Bitcoin Cash sighash requires value of input to be signed.');
      }

      return _hashForWitnessV0(inIndex, prevOutScript, amount, hashType);
    } else {
      return hashForSignature(inIndex, prevOutScript, hashType);
    }
  }

  int virtualSize() {
    return 8 +
        varuint.encodingLength(inputs.length) +
        varuint.encodingLength(outputs.length) +
        inputs.fold(0, (sum, input) => sum + 40 + _varSliceSize(input.script)) +
        outputs.fold(
            0, (sum, output) => sum + 8 + _varSliceSize(output.script));
  }

  Uint8List toBuffer([Uint8List buffer, int initialOffset]) {
    return this._toBuffer(buffer, initialOffset);
  }

  String toHex() {
    return HEX.encode(this.toBuffer());
  }

  Uint8List getHash() {
    return bcrypto.hash256(_toBuffer());
  }

  String getId() {
    return HEX.encode(getHash().reversed.toList());
  }

  _hashForWitnessV0(
      int inIndex, Uint8List prevOutScript, int amount, int hashType) {
    Uint8List tBuffer;
    int tOffset;

    void writeSlice(Uint8List slice) {
      tBuffer.setRange(tOffset, slice.length + tOffset, slice);
      tOffset += slice.length;
    }

    void writeUint32(int i) {
      tBuffer.buffer.asByteData().setUint32(tOffset, i, Endian.little);
      tOffset += 4;
    }

    void writeUint64(int i) {
      tBuffer.buffer.asByteData().setUint64(tOffset, i, Endian.little);
      tOffset += 8;
    }

    void writeVarInt(int i) {
      varuint.encode(i, tBuffer, tOffset);
      tOffset += varuint.encodingLength(i);
    }

    writeVarSlice(slice) {
      writeVarInt(slice.length);
      writeSlice(slice);
    }

    Uint8List hashPrevoutputs;
    Uint8List hashSequence;
    Uint8List hashOutputs;

    if ((hashType & SIGHASH_ANYONECANPAY == 0)) {
      tBuffer = Uint8List(36 * this.inputs.length);
      tOffset = 0;

      this.inputs.forEach((txInput) {
        writeSlice(txInput.hash);
        writeUint32(txInput.index);
      });

      hashPrevoutputs = bcrypto.hash256(tBuffer);
    }

    if ((hashType & SIGHASH_ANYONECANPAY) == 0 &&
        (hashType & 0x1f) != SIGHASH_SINGLE &&
        (hashType & 0x1f) != SIGHASH_NONE) {
      tBuffer = Uint8List(4 * this.inputs.length);
      tOffset = 0;

      this.inputs.forEach((txInput) {
        writeUint32(txInput.sequence);
      });

      hashSequence = bcrypto.hash256(tBuffer);
    }

    if ((hashType & 0x1f) != SIGHASH_SINGLE &&
        (hashType & 0x1f) != SIGHASH_NONE) {
      final txOutputsSize = this.outputs.fold(0, (int sum, Output output) {
        return sum + 8 + _varSliceSize(output.script);
      });

      tBuffer = Uint8List(txOutputsSize);
      tOffset = 0;

      this.outputs.forEach((Output output) {
        writeUint64(output.value);
        writeVarSlice(output.script);
      });

      hashOutputs = bcrypto.hash256(tBuffer);
    } else if ((hashType & 0x1f) == SIGHASH_SINGLE &&
        (inIndex < this.outputs.length)) {
      final output = this.outputs[inIndex];

      tBuffer = Uint8List(8 + _varSliceSize(output.script));
      tOffset = 0;
      writeUint64(output.value);
      writeVarSlice(output.script);

      hashOutputs = bcrypto.hash256(tBuffer);
    }

    tBuffer = Uint8List(156 + _varSliceSize(prevOutScript));
    tOffset = 0;

    final input = this.inputs[inIndex];
    writeUint32(this.version);
    writeSlice(hashPrevoutputs);
    writeSlice(hashSequence);
    writeSlice(input.hash);
    writeUint32(input.index);
    writeVarSlice(prevOutScript);
    writeUint64(amount);
    writeUint32(input.sequence);
    writeSlice(hashOutputs);
    writeUint32(this.locktime);
    writeUint32(hashType);
    return bcrypto.hash256(tBuffer);
  }

  _toBuffer([Uint8List buffer, initialOffset]) {
    if (buffer == null) buffer = new Uint8List(virtualSize());
    var bytes = buffer.buffer.asByteData();
    var offset = initialOffset ?? 0;
    writeSlice(slice) {
      buffer.setRange(offset, offset + slice.length, slice);
      offset += slice.length;
    }

    writeUInt32(i) {
      bytes.setUint32(offset, i, Endian.little);
      offset += 4;
    }

    writeInt32(i) {
      bytes.setInt32(offset, i, Endian.little);
      offset += 4;
    }

    writeUInt64(i) {
      bytes.setUint64(offset, i, Endian.little);
      offset += 8;
    }

    writeVarInt(i) {
      varuint.encode(i, buffer, offset);
      offset += varuint.encodingLength(i);
    }

    writeVarSlice(slice) {
      writeVarInt(slice.length);
      writeSlice(slice);
    }

    writeInt32(version);
    writeVarInt(this.inputs.length);
    inputs.forEach((txIn) {
      writeSlice(txIn.hash);
      writeUInt32(txIn.index);
      writeVarSlice(txIn.script);
      writeUInt32(txIn.sequence);
    });
    varuint.encode(outputs.length, buffer, offset);
    offset += varuint.encodingLength(outputs.length);
    outputs.forEach((txOut) {
      if (txOut.valueBuffer == null) {
        writeUInt64(txOut.value);
      } else {
        writeSlice(txOut.valueBuffer);
      }
      writeVarSlice(txOut.script);
    });
    writeUInt32(this.locktime);
    // avoid slicing unless necessary
    if (initialOffset != null) return buffer.sublist(initialOffset, offset);
    return buffer;
  }

  factory Transaction.clone(Transaction originalTx) {
    List<Input> inputs = originalTx.inputs.map((input) {
      return Input.clone(input);
    }).toList();
    List<Output> outputs = originalTx.outputs.map((output) {
      return Output.clone(output);
    }).toList();
    Transaction clonedTx = new Transaction(
        originalTx.version, originalTx.locktime, inputs, outputs);

    return clonedTx;
  }

  static int _varSliceSize(Uint8List someScript) {
    final length = someScript.length;
    return varuint.encodingLength(length) + length;
  }
}

/// Container for input data and factories to create them
class Input {
  Uint8List hash;
  int index;
  int sequence;
  int value;
  Uint8List script;
  Uint8List signScript;
  Uint8List prevOutScript;
  List<Uint8List> pubkeys;
  List<Uint8List> signatures;
  Input(
      {this.hash,
      this.index,
      this.script,
      this.sequence,
      this.value,
      this.prevOutScript,
      this.pubkeys,
      this.signatures}) {
    if (this.hash != null && this.hash.length != 32)
      throw new ArgumentError("Invalid input hash");
    if (this.index != null && !isUint(this.index, 32))
      throw new ArgumentError("Invalid input index");
    if (this.sequence != null && !isUint(this.sequence, 32))
      throw new ArgumentError("Invalid input sequence");
    if (this.value != null && !isSatoshi(this.value))
      throw ArgumentError("Invalid ouput value");
  }

  factory Input.expandInput(Uint8List scriptSig) {
    if (_isP2PKHInput(scriptSig) == false) {
      return new Input(prevOutScript: scriptSig);
    }
    P2PKH p2pkh = new P2PKH(data: new P2PKHData(input: scriptSig));
    return new Input(
        prevOutScript: p2pkh.data.output,
        pubkeys: [p2pkh.data.pubkey],
        signatures: [p2pkh.data.signature]);
  }

  factory Input.clone(Input input) {
    return new Input(
      hash: input.hash != null ? Uint8List.fromList(input.hash) : null,
      index: input.index,
      script: input.script != null ? Uint8List.fromList(input.script) : null,
      sequence: input.sequence,
      value: input.value,
      prevOutScript: input.prevOutScript != null
          ? Uint8List.fromList(input.prevOutScript)
          : null,
      pubkeys: input.pubkeys != null
          ? input.pubkeys.map(
              (pubkey) => pubkey != null ? Uint8List.fromList(pubkey) : null)
          : null,
      signatures: input.signatures != null
          ? input.signatures.map((signature) =>
              signature != null ? Uint8List.fromList(signature) : null)
          : null,
    );
  }

  @override
  String toString() {
    return 'Input{hash: $hash, index: $index, sequence: $sequence, value: $value, script: $script, ' +
        'signScript: $signScript, prevOutScript: $prevOutScript, pubkeys: $pubkeys, signatures: $signatures}';
  }

  static bool _isP2PKHInput(script) {
    final chunks = bscript.decompile(script);
    return chunks != null &&
        chunks.length == 2 &&
        bscript.isCanonicalScriptSignature(chunks[0]) &&
        bscript.isCanonicalPubKey(chunks[1]);
  }
}

/// Container for storing outputs and factories for working with them
class Output {
  Uint8List script;
  int value;
  Uint8List valueBuffer;
  List<Uint8List> pubkeys;
  List<Uint8List> signatures;

  Output(
      {this.script,
      this.value,
      this.pubkeys,
      this.signatures,
      this.valueBuffer}) {
    if (value != null && !isSatoshi(value))
      throw ArgumentError("Invalid ouput value");
  }
  /*
  factory Output.expandOutput(Uint8List script, Uint8List ourPubKey) {
    if (_isP2PKHOutput(script) == false) {
      throw ArgumentError("Unsupport script!");
    }
    // does our hash160(pubKey) match the output scripts?
    Uint8List pkh1 = new P2PKH(data: new P2PKHData(output: script)).data.hash;
    Uint8List pkh2 = bcrypto.hash160(ourPubKey);
    if (pkh1 != pkh2) throw ArgumentError("Hash mismatch!");
    return new Output(pubkeys: [ourPubKey], signatures: [null]);
  }*/
  factory Output.clone(Output output) {
    return new Output(
      script: output.script != null ? Uint8List.fromList(output.script) : null,
      value: output.value,
      valueBuffer: output.valueBuffer != null
          ? Uint8List.fromList(output.valueBuffer)
          : null,
      pubkeys: output.pubkeys != null
          ? output.pubkeys.map(
              (pubkey) => pubkey != null ? Uint8List.fromList(pubkey) : null)
          : null,
      signatures: output.signatures != null
          ? output.signatures.map((signature) =>
              signature != null ? Uint8List.fromList(signature) : null)
          : null,
    );
  }

  @override
  String toString() {
    return 'Output{script: $script, value: $value, valueBuffer: $valueBuffer, pubkeys: $pubkeys, signatures: $signatures}';
  }
}
