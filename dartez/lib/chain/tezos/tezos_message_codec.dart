import 'dart:convert';

import 'package:dartez/chain/tezos/tezos_language_util.dart';
import 'package:dartez/chain/tezos/tezos_message_utils.dart';
import 'package:dartez/models/operation_model.dart';

class TezosMessageCodec {
  static String encodeOperation(OperationModel message) {
    if (message.pkh != null && message.secret != null)
      return encodeActivation(message);
    if (message.kind == 'transaction') return encodeTransaction(message);
    if (message.kind == 'reveal') return encodeReveal(message);
    if (message.kind == 'delegation') return encodeDelegation(message);
    if (message.kind == 'origination') return encodeOrigination(message);
    return '';
  }

  static String encodeActivation(activation) {
    var hex = TezosMessageUtils.writeInt(4);
    hex += TezosMessageUtils.writeAddress(activation.pkh).substring(4);
    hex += activation.secret;
    return hex;
  }

  static String encodeTransaction(OperationModel message) {
    String hex = "6c"; // TezosMessageUtils.writeInt(108);
    hex += TezosMessageUtils.writeAddress(message.source!).substring(2);
    hex += TezosMessageUtils.writeInt(int.parse(message.fee!));
    hex += TezosMessageUtils.writeInt(message.counter!);
    hex += TezosMessageUtils.writeInt(message.gasLimit!);
    hex += TezosMessageUtils.writeInt(message.storageLimit);
    hex += TezosMessageUtils.writeInt(int.parse(message.amount!));
    hex += TezosMessageUtils.writeAddress(message.destination!);
    if (message.parameters != null) {
      var composite = message.parameters!;
      var result = TezosLanguageUtil.translateMichelineToHex(
          jsonEncode(composite['value']));
      if ((composite['entrypoint'] == 'default' ||
              composite['entrypoint'] == '') &&
          result == '030b') {
        hex += '00';
      } else {
        hex += 'ff';
        if (composite['entrypoint'] == 'default' ||
            composite['entrypoint'] == '') {
          hex += '00';
        } else if (composite['entrypoint'] == 'root') {
          hex += '01';
        } else if (composite['entrypoint'] == 'do') {
          hex += '02';
        } else if (composite['entrypoint'] == 'set_delegate') {
          hex += '03';
        } else if (composite['entrypoint'] == 'remove_delegate') {
          hex += '04';
        } else {
          var entryStr = '0' + composite['entrypoint'].length.toRadixString(16);
          hex += 'ff' +
              (entryStr.substring(entryStr.length - 2)) +
              composite['entrypoint']
                  .split('')
                  .map((String c) => c.codeUnitAt(0).toRadixString(16))
                  .join('');
        }
        var resRad = '0000000' + (result!.length ~/ 2).toRadixString(16);
        hex += resRad.substring(resRad.length - 8) + result;
      }
    } else {
      hex += '00';
    }
    return hex;
  }

  static String encodeReveal(OperationModel message) {
    var hex = TezosMessageUtils.writeInt(107);
    hex += TezosMessageUtils.writeAddress(message.source!).substring(2);
    hex += TezosMessageUtils.writeInt(int.parse(message.fee!));
    hex += TezosMessageUtils.writeInt(message.counter!);
    hex += TezosMessageUtils.writeInt(message.gasLimit!);
    hex += TezosMessageUtils.writeInt(message.storageLimit);
    hex += TezosMessageUtils.writePublicKey(message.publicKey!);
    return hex;
  }

  static String encodeDelegation(OperationModel delegation) {
    var hex = TezosMessageUtils.writeInt(110);
    hex += TezosMessageUtils.writeAddress(delegation.source!).substring(2);
    hex += TezosMessageUtils.writeInt(int.parse(delegation.fee!));
    hex += TezosMessageUtils.writeInt(delegation.counter!);
    hex += TezosMessageUtils.writeInt(delegation.gasLimit!);
    hex += TezosMessageUtils.writeInt(delegation.storageLimit);
    if (delegation.delegate != null && delegation.delegate!.isNotEmpty) {
      hex += TezosMessageUtils.writeBoolean(true);
      hex += TezosMessageUtils.writeAddress(delegation.delegate!).substring(2);
    } else {
      hex += TezosMessageUtils.writeBoolean(false);
    }
    return hex;
  }

  static String encodeOrigination(OperationModel origination) {
    var hex = TezosMessageUtils.writeInt(109);
    hex += TezosMessageUtils.writeAddress(origination.source!).substring(2);
    hex += TezosMessageUtils.writeInt(int.parse(origination.fee!));
    hex += TezosMessageUtils.writeInt(origination.counter!);
    hex += TezosMessageUtils.writeInt(origination.gasLimit!);
    hex += TezosMessageUtils.writeInt(origination.storageLimit);
    hex += TezosMessageUtils.writeInt(int.parse(origination.amount!));

    if (origination.delegate != null) {
      hex += TezosMessageUtils.writeBoolean(true);
      hex += TezosMessageUtils.writeAddress(origination.delegate!).substring(2);
    } else {
      hex += TezosMessageUtils.writeBoolean(false);
    }
    if (origination.script != null) {
      var parts = [];
      parts.add(origination.script!['code']);
      parts.add(origination.script!['storage']);
      hex += parts
          .map((p) => jsonEncode(p))
          .map((p) => TezosLanguageUtil.translateMichelineToHex(p))
          .fold('', (m, p) {
        var result = ('0000000' + (p!.length ~/ 2).toRadixString(16));
        result = result.substring(result.length - 8) + p;
        return m + result;
      });
    }

    return hex;
  }
}
