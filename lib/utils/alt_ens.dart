import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:http/http.dart';
import 'package:convert/convert.dart';
import 'package:sha3/sha3.dart';
import 'package:web3dart/web3dart.dart' as web3;

Future<Map> unstoppableDomainENS({
  String cryptoDomainName,
  String currency,
}) async {
  try {
    final udResolvers = {
      "0x049aba7510f45BA5b64ea9E658E342F904DB358D": "Ethereum",
      "0x1BDc0fD4fbABeed3E611fd6195fCd5d41dcEF393": "Ethereum",
      "0xa9a6A3626993D487d2Dbda3173cf58cA1a9D9e9f": "Polygon Matic",
      "0x3E67b8c702a1292d1CEb025494C84367fcb12b45": "Polygon Matic"
    };
    for (String contractAddr in udResolvers.keys) {
      final web3.EthereumAddress proxyReader =
          web3.EthereumAddress.fromHex(contractAddr);

      web3.DeployedContract contract = web3.DeployedContract(
        web3.ContractAbi.fromJson(unstoppableDomainAbi, ''),
        proxyReader,
      );

      final rpcUrl = getEVMBlockchains()[udResolvers[contractAddr]]['rpc'];

      final client = web3.Web3Client(rpcUrl, Client());
      String udCryptoAddress = (await client.call(
        contract: contract,
        function: contract.function('get'),
        params: [
          "crypto.${currency ?? 'ETH'}.address",
          BigInt.parse(nameHash(cryptoDomainName))
        ],
      ))
          .first;

      if (udCryptoAddress.isNotEmpty) {
        return {'success': true, 'msg': udCryptoAddress};
      }
    }
    return {'success': false, 'msg': 'Error resolving unstoppable domain ens'};
  } catch (e) {
    return {'success': false, 'msg': 'Error resolving unstoppable domain ens'};
  }
}

String nameHash(String inputName) {
  String node = '';
  for (int i = 0; i < 32; i++) {
    node += '00';
  }
  if (inputName != null) {
    final labels = inputName.split('.');

    for (int i = labels.length - 1; i >= 0; i--) {
      String labelSha;
      if (_isEncodedLabelhash(labels[i])) {
        labelSha = _decodeLabelhash(labels[i]);
      } else {
        final normalisedLabel = labels[i];

        labelSha = sha3(normalisedLabel);
      }
      node = sha3(String.fromCharCodes(hex.decode('$node$labelSha')));
    }
  }

  return '0x' + node;
}

bool _isEncodedLabelhash(hash) {
  return hash.startsWith('[') && hash.endsWith(']') && hash.length == 66;
}

String _decodeLabelhash(String hash) {
  if (!(hash.startsWith('[') && hash.endsWith(']'))) {
    throw 'Expected encoded labelhash to start and end with square brackets';
  }

  if (hash.length != 66) {
    throw 'Expected encoded labelhash to have a length of 66';
  }

  return hash.slice(1, -1);
}

String sha3(String string) {
  const keccakPadding = [1, 256, 65536, 16777216];
  final hash =
      SHA3(256, keccakPadding, 256).update(string.runes.toList()).digest();

  return hex.encode(hash);
}

extension Slice on String {
  String slice(int start, [int end]) {
    if (end != null && end.isNegative) {
      return substring(start, length - end.abs());
    }
    return substring(start, end);
  }
}
