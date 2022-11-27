(function () {
  const __addressHex = "%1$s";
  const __rpcURL = "%2$s";
  const __chainID = "%3$s";
  let isFlutterInAppWebViewReady = false;

  function executeCallback(id, error, value) {
    AlphaWallet.executeCallback(id, error, value);
  }

  window.addEventListener("flutterInAppWebViewPlatformReady", function (event) {
    isFlutterInAppWebViewReady = true;
    console.log("done and ready");
  });

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
        const { data, chainType } = msgParams;
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
        const { data, chainType } = msgParams;
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
        const { data } = msgParams;
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
