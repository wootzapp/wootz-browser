// import { initializeProvider } from '@metamask/providers';
// import detectEthereumProvider from '@metamask/detect-provider';
(

  function () {
  // setMetatag();
  const __addressHex = "%1$s";
  const __rpcURL = "%2$s";
  const __chainID = "%3$s";
  let isFlutterInAppWebViewReady = false;

  function executeCallback(id, error, value) {
    AlphaWallet.executeCallback(id, error, value);
  }

  /**
   * Checks if the given string is an address
   *
   * @method isAddress
   * @param {String} address the given HEX adress
   * @return {Boolean}
   */
  var isAddress = function (address) {
    //FIXME: temporary fix
    address = address.toLowerCase();
    if (!/^(0x)?[0-9a-f]{40}$/i.test(address)) {
      // check if it has the basic requirements of an address
      return false;
    } else if (
      /^(0x)?[0-9a-f]{40}$/.test(address) ||
      /^(0x)?[0-9A-F]{40}$/.test(address)
    ) {
      // If it's all small caps or all all caps, return true
      return true;
    } else {
      // Otherwise check each case
      return isChecksumAddress(address);
    }
  };

  /**
   * Checks if the given string is a checksummed address
   *
   * @method isChecksumAddress
   * @param {String} address the given HEX adress
   * @return {Boolean}
   */
  var isChecksumAddress = function (address) {
    // Check each case
    address = address.replace("0x", "");
    var addressHash = sha3(address.toLowerCase());
    for (var i = 0; i < 40; i++) {
      // the nth letter should be uppercase if the nth digit of casemap is 1
      if (
        (parseInt(addressHash[i], 16) > 7 &&
          address[i].toUpperCase() !== address[i]) ||
        (parseInt(addressHash[i], 16) <= 7 &&
          address[i].toLowerCase() !== address[i])
      ) {
        return false;
      }
    }
    return true;
  };

  window.addEventListener("flutterInAppWebViewPlatformReady", function (event) {
    isFlutterInAppWebViewReady = true;
    console.log("done and ready");

    // metamaskProvider();
    // console.log('metamaskProvider called');

  });

  function setMetatag(){
    var meta=document.createElement('meta');
    meta.httpEquiv = 'Content-Security-Policy';
    meta.content = "script-src 'self'; font-src 'self' data: blob:; img-src 'self' data: blob:; style-src 'self' 'unsafe-inline'"

    document.getElementsByTagName('head')[0].appendChild(meta);
  }
  
  // async function metamaskProvider() {
  //   if(window.ethereum !== undefined)
  //   {
  //     // ethereum.isMetamask = true;
      
  //     // console.log("metmask set to true");
  //     console.log('before requestAccounts');
  //     await ethereum.request({method:'eth_requestAccounts'});
  //     console.log('requestAccounts done');
  //   }
  // }

  window.AlphaWallet.init(
    __rpcURL,
    {

      getAccounts: function (cb) {
        cb(null, [__addressHex]);
      },
      processTransaction: function (tx, cb) {
        console.log("signing a transaction", JSON.stringify(tx));
        const { id = 8888 } = tx;
        AlphaWallet.addCallback(id, cb);

        var gasLimit = tx.gasLimit || tx.gas || null;
        var gasPrice = tx.gasPrice || null;
        var data = tx.data || null;
        var nonce = tx.nonce || -1;
        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler(
              "signTransaction",
              id,
              tx.to || null,
              tx.value,
              nonce,
              gasLimit,
              gasPrice,
              data
            );
          }
        }, 100);
      },
      signMessage: function (msgParams, cb) {
        console.log("signMessage", msgParams);
        var { data, from, chainType } = msgParams;
        if (!isAddress(from)) {
          const from_ = from;
          from = data;
          data = from_;
        }
        console.log(JSON.stringify(msgParams));
        const { id = 8888 } = msgParams;
        AlphaWallet.addCallback(id, cb);

        console.log("normal sign");

        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler(
              "signMessage",
              id,
              data,
              "Normal Sign"
            );
          }
        }, 100);
      },
      signPersonalMessage: function (msgParams, cb) {
        console.log("signPersonalMessage", msgParams);
        var { data, chainType, from } = msgParams;
        if (!isAddress(from)) {
          const from_ = from;
          from = data;
          data = from_;
        }
        console.log(JSON.stringify(msgParams));
        const { id = 8888 } = msgParams;
        console.log("personal sign");
        AlphaWallet.addCallback(id, cb);
        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler(
              "signMessage",
              id,
              data,
              "Personal"
            );
          }
        }, 100);
      },
      signTypedMessage: function (msgParams, cb) {
        console.log("signTypedMessage ", msgParams);
        var { data, from } = msgParams;
        console.log(JSON.stringify(msgParams));
        if (!isAddress(from)) {
          const from_ = from;
          from = data;
          data = from_;
        }

        const { id = 8888 } = msgParams;
        AlphaWallet.addCallback(id, cb);
        console.log("typed message sign");
        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler(
              "signMessage",
              id,
              JSON.stringify(msgParams),
              "Typed Message"
            );
          }
        }, 100);
      },
      ethCall: function (msgParams, cb) {
        const data = msgParams;
        const { id = Math.floor(Math.random() * 100000 + 1) } = msgParams;
        AlphaWallet.addCallback(id, cb);

        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler(
              "ethCall",
              id,
              JSON.stringify(msgParams)
            );
          }
        }, 100);

        // alpha.ethCall(id, JSON.stringify(msgParams));
        //alpha.ethCall(id, msgParams.to, msgParams.data, msgParams.value);
      },
      walletAddEthereumChain: function (msgParams, cb) {
        const data = msgParams;
        const { id = Math.floor(Math.random() * 100000 + 1) } = msgParams;
        console.log("walletAddEthereumChain", msgParams);
        AlphaWallet.addCallback(id, cb);
        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler(
              "walletAddEthereumChain",
              id,
              JSON.stringify(msgParams)
            );
            // window.flutter_inappwebview.callHandler("walletAddEthereumChain", id);
          } else {
            console.log("handler not ready");
          }
        }, 100);
        //webkit.messageHandlers.walletAddEthereumChain.postMessage({"name": "walletAddEthereumChain", "object": data, id: id})
      },
      walletSwitchEthereumChain: function (msgParams, cb) {
        const data = msgParams;
        const { id = Math.floor(Math.random() * 100000 + 1) } = msgParams;
        console.log("walletSwitchEthereumChain", msgParams);
        AlphaWallet.addCallback(id, cb);
        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler(
              "walletSwitchEthereumChain",
              id,
              JSON.stringify(msgParams)
            );
          }
        }, 100);
        //webkit.messageHandlers.walletSwitchEthereumChain.postMessage({"name": "walletSwitchEthereumChain", "object": data, id: id})
      },
      requestAccounts: function (cb) {
        id = Math.floor(Math.random() * 100000 + 1);
        console.log("requestAccounts", id);
        AlphaWallet.addCallback(id, cb);

        const interval = setInterval(() => {
          if (isFlutterInAppWebViewReady) {
            clearInterval(interval);
            window.flutter_inappwebview.callHandler("requestAccounts", id);
          } else {
            console.log("handler not ready");
          }
        }, 100);
      },
      enable: function () {
        return new Promise(function (resolve, reject) {
          //send back the coinbase account as an array of one
          resolve([__addressHex]);
        });
      },
    },
    {
      address: __addressHex,
      networkVersion: __chainID,
      //networkVersion: "0x" + parseInt(__chainID).toString(16) || null
    }
  );

  window.web3.setProvider = function () {
    console.debug("Alpha Wallet - overrode web3.setProvider");
  };

  window.web3.version.getNetwork = function (cb) {
    cb(null, __chainID);
  };
  window.web3.eth.getCoinbase = function (cb) {
    return cb(null, __addressHex);
  };
  window.web3.eth.defaultAccount = __addressHex;

  window.ethereum = web3.currentProvider;
})();
