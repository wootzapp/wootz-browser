class JsCallbackModel {
  int id;
  String name;
  Map<String, dynamic> object;
  String network;

  JsCallbackModel({this.id, this.name, this.object, this.network});

  JsCallbackModel.fromJson(Map<String, dynamic> json) {
    id = json['id'];
    name = json['name'];
    object = json['object'];
    network = json['network'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['id'] = id;
    data['name'] = name;
    data['object'] = object;
    data['network'] = network;
    return data;
  }
}

class JsDataModel {
  String data;

  JsDataModel({this.data});

  JsDataModel.fromJson(Map<String, dynamic> json) {
    data = json['data'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['data'] = this.data;
    return data;
  }
}

class JsSolanaTransactionObject {
  String data;

  JsSolanaTransactionObject({this.data});

  JsSolanaTransactionObject.fromJson(Map<String, dynamic> json) {
    data = json['data'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};

    data['data'] = data;
    return data;
  }
}

class JsTransactionObject {
  String gas;
  String value;
  String from;
  String to;
  String data;
  String nonce;
  String gasPrice;

  JsTransactionObject({
    this.gas,
    this.value,
    this.from,
    this.to,
    this.data,
    this.nonce,
    this.gasPrice,
  });

  JsTransactionObject.fromJson(Map<String, dynamic> json) {
    gas = json['gas'];
    value = json['value'];
    from = json['from'];
    to = json['to'];
    data = json['data'];
    nonce = json['nonce'];
    gasPrice = json['gasPrice'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['gas'] = gas;
    data['value'] = value;
    data['from'] = from;
    data['to'] = to;
    data['data'] = data;
    data['nonce'] = nonce;
    data['gasPrice'] = gasPrice;
    return data;
  }
}

class JsEcRecoverObject {
  String signature;
  String message;

  JsEcRecoverObject({this.signature, this.message});

  JsEcRecoverObject.fromJson(Map<String, dynamic> json) {
    signature = json['signature'];
    message = json['message'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['signature'] = signature;
    data['message'] = message;
    return data;
  }
}

class JsEthSignTypedData {
  String data;
  String raw;

  JsEthSignTypedData({this.data, this.raw});

  JsEthSignTypedData.fromJson(Map<String, dynamic> json) {
    data = json['data'];
    raw = json['raw'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['data'] = data;
    data['raw'] = raw;
    return data;
  }
}

class JsWatchAsset {
  String type;
  String contract;
  String symbol;
  int decimals;

  JsWatchAsset({
    this.type,
    this.contract,
    this.symbol,
    this.decimals,
  });

  JsWatchAsset.fromJson(Map<String, dynamic> json) {
    type = json['type'];
    contract = json['contract'];
    symbol = json['symbol'];
    decimals = json['decimals'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['type'] = type;
    data['contract'] = contract;
    data['symbol'] = symbol;
    data['decimals'] = decimals;
    return data;
  }
}

class JsAddEthereumChain {
  String chainId;
  String chainName;
  List rpcUrls;
  List blockExplorerUrls;
  String symbol;
  String name;

  JsAddEthereumChain({this.chainId, this.chainName});

  JsAddEthereumChain.fromJson(Map<String, dynamic> json) {
    chainId = json['chainId'];
    chainName = json['chainName'];
    rpcUrls = json['rpcUrls'];
    blockExplorerUrls = json['blockExplorerUrls'];
    if (json['nativeCurrency'] != null) {
      symbol = json['nativeCurrency']['symbol'];
      name = json['nativeCurrency']['name'];
    }
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['chainId'] = chainId;
    data['chainName'] = chainName;
    data['rpcUrls'] = rpcUrls;
    data['blockExplorerUrls'] = blockExplorerUrls;
    data['symbol'] = symbol;
    data['name'] = name;
    return data;
  }
}

class IncomingAccountsModel {
  int chainId;
  String rpcUrl;
  String address;

  IncomingAccountsModel({
    this.chainId,
    this.rpcUrl,
    this.address,
  });

  IncomingAccountsModel.fromJson(Map<String, dynamic> json) {
    chainId = json['chainId'];
    rpcUrl = json['rpcUrl'];
    address = json['address'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = <String, dynamic>{};
    data['chainId'] = chainId;
    data['rpcUrl'] = rpcUrl;
    data['address'] = address;

    return data;
  }
}
