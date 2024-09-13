"use strict";
// Copyright (c) 2018-2023 Coinbase, Inc. <https://www.coinbase.com/>
Object.defineProperty(exports, "__esModule", { value: true });
exports.WalletLinkRelay = void 0;
const WalletLinkConnection_1 = require("./connection/WalletLinkConnection");
const constants_1 = require("./constants");
const RelayEventManager_1 = require("./RelayEventManager");
const WalletLinkSession_1 = require("./type/WalletLinkSession");
const Web3Response_1 = require("./type/Web3Response");
const util_1 = require("./ui/components/util");
const WalletLinkRelayUI_1 = require("./ui/WalletLinkRelayUI");
const WLMobileRelayUI_1 = require("./ui/WLMobileRelayUI");
const error_1 = require("../../../core/error");
const util_2 = require("../../../core/type/util");
const ScopedLocalStorage_1 = require("../../../util/ScopedLocalStorage");
class WalletLinkRelay {
    constructor(options) {
        this.accountsCallback = null;
        this.chainCallbackParams = { chainId: '', jsonRpcUrl: '' }; // to implement distinctUntilChanged
        this.chainCallback = null;
        this.dappDefaultChain = 1;
        this.isMobileWeb = (0, util_1.isMobileWeb)();
        this.appName = '';
        this.appLogoUrl = null;
        this.linkedUpdated = (linked) => {
            this.isLinked = linked;
            const cachedAddresses = this.storage.getItem(constants_1.LOCAL_STORAGE_ADDRESSES_KEY);
            if (linked) {
                // Only set linked session variable one way
                this._session.linked = linked;
            }
            this.isUnlinkedErrorState = false;
            if (cachedAddresses) {
                const addresses = cachedAddresses.split(' ');
                const wasConnectedViaStandalone = this.storage.getItem('IsStandaloneSigning') === 'true';
                if (addresses[0] !== '' && !linked && this._session.linked && !wasConnectedViaStandalone) {
                    this.isUnlinkedErrorState = true;
                }
            }
        };
        this.metadataUpdated = (key, value) => {
            this.storage.setItem(key, value);
        };
        this.chainUpdated = (chainId, jsonRpcUrl) => {
            if (this.chainCallbackParams.chainId === chainId &&
                this.chainCallbackParams.jsonRpcUrl === jsonRpcUrl) {
                return;
            }
            this.chainCallbackParams = {
                chainId,
                jsonRpcUrl,
            };
            if (this.chainCallback) {
                this.chainCallback(chainId, jsonRpcUrl);
            }
        };
        this.accountUpdated = (selectedAddress) => {
            if (this.accountsCallback) {
                this.accountsCallback([selectedAddress]);
            }
            if (WalletLinkRelay.accountRequestCallbackIds.size > 0) {
                // We get the ethereum address from the metadata.  If for whatever
                // reason we don't get a response via an explicit web3 message
                // we can still fulfill the eip1102 request.
                Array.from(WalletLinkRelay.accountRequestCallbackIds.values()).forEach((id) => {
                    const message = {
                        type: 'WEB3_RESPONSE',
                        id,
                        response: {
                            method: 'requestEthereumAccounts',
                            result: [selectedAddress],
                        },
                    };
                    this.invokeCallback(Object.assign(Object.assign({}, message), { id }));
                });
                WalletLinkRelay.accountRequestCallbackIds.clear();
            }
        };
        this.resetAndReload = this.resetAndReload.bind(this);
        this.linkAPIUrl = options.linkAPIUrl;
        this.storage = options.storage;
        const { session, ui, connection } = this.subscribe();
        this._session = session;
        this.connection = connection;
        this.relayEventManager = new RelayEventManager_1.RelayEventManager();
        this.ui = ui;
    }
    subscribe() {
        const session = WalletLinkSession_1.WalletLinkSession.load(this.storage) || new WalletLinkSession_1.WalletLinkSession(this.storage).save();
        const { linkAPIUrl } = this;
        const connection = new WalletLinkConnection_1.WalletLinkConnection({
            session,
            linkAPIUrl,
            listener: this,
        });
        const ui = this.isMobileWeb ? new WLMobileRelayUI_1.WLMobileRelayUI() : new WalletLinkRelayUI_1.WalletLinkRelayUI();
        connection.connect();
        return { session, ui, connection };
    }
    attachUI() {
        this.ui.attach();
    }
    resetAndReload() {
        Promise.race([
            this.connection.setSessionMetadata('__destroyed', '1'),
            new Promise((resolve) => setTimeout(() => resolve(null), 1000)),
        ])
            .then(() => {
            this.connection.destroy();
            /**
             * Only clear storage if the session id we have in memory matches the one on disk
             * Otherwise, in the case where we have 2 tabs, another tab might have cleared
             * storage already.  In that case if we clear storage again, the user will be in
             * a state where the first tab allows the user to connect but the session that
             * was used isn't persisted.  This leaves the user in a state where they aren't
             * connected to the mobile app.
             */
            const storedSession = WalletLinkSession_1.WalletLinkSession.load(this.storage);
            if ((storedSession === null || storedSession === void 0 ? void 0 : storedSession.id) === this._session.id) {
                ScopedLocalStorage_1.ScopedLocalStorage.clearAll();
            }
            document.location.reload();
        })
            .catch((_) => { });
    }
    setAppInfo(appName, appLogoUrl) {
        this.appName = appName;
        this.appLogoUrl = appLogoUrl;
    }
    getStorageItem(key) {
        return this.storage.getItem(key);
    }
    setStorageItem(key, value) {
        this.storage.setItem(key, value);
    }
    signEthereumMessage(message, address, addPrefix, typedDataJson) {
        return this.sendRequest({
            method: 'signEthereumMessage',
            params: {
                message: (0, util_2.hexStringFromBuffer)(message, true),
                address,
                addPrefix,
                typedDataJson: typedDataJson || null,
            },
        });
    }
    ethereumAddressFromSignedMessage(message, signature, addPrefix) {
        return this.sendRequest({
            method: 'ethereumAddressFromSignedMessage',
            params: {
                message: (0, util_2.hexStringFromBuffer)(message, true),
                signature: (0, util_2.hexStringFromBuffer)(signature, true),
                addPrefix,
            },
        });
    }
    signEthereumTransaction(params) {
        return this.sendRequest({
            method: 'signEthereumTransaction',
            params: {
                fromAddress: params.fromAddress,
                toAddress: params.toAddress,
                weiValue: (0, util_2.bigIntStringFromBigInt)(params.weiValue),
                data: (0, util_2.hexStringFromBuffer)(params.data, true),
                nonce: params.nonce,
                gasPriceInWei: params.gasPriceInWei ? (0, util_2.bigIntStringFromBigInt)(params.gasPriceInWei) : null,
                maxFeePerGas: params.gasPriceInWei ? (0, util_2.bigIntStringFromBigInt)(params.gasPriceInWei) : null,
                maxPriorityFeePerGas: params.gasPriceInWei
                    ? (0, util_2.bigIntStringFromBigInt)(params.gasPriceInWei)
                    : null,
                gasLimit: params.gasLimit ? (0, util_2.bigIntStringFromBigInt)(params.gasLimit) : null,
                chainId: params.chainId,
                shouldSubmit: false,
            },
        });
    }
    signAndSubmitEthereumTransaction(params) {
        return this.sendRequest({
            method: 'signEthereumTransaction',
            params: {
                fromAddress: params.fromAddress,
                toAddress: params.toAddress,
                weiValue: (0, util_2.bigIntStringFromBigInt)(params.weiValue),
                data: (0, util_2.hexStringFromBuffer)(params.data, true),
                nonce: params.nonce,
                gasPriceInWei: params.gasPriceInWei ? (0, util_2.bigIntStringFromBigInt)(params.gasPriceInWei) : null,
                maxFeePerGas: params.maxFeePerGas ? (0, util_2.bigIntStringFromBigInt)(params.maxFeePerGas) : null,
                maxPriorityFeePerGas: params.maxPriorityFeePerGas
                    ? (0, util_2.bigIntStringFromBigInt)(params.maxPriorityFeePerGas)
                    : null,
                gasLimit: params.gasLimit ? (0, util_2.bigIntStringFromBigInt)(params.gasLimit) : null,
                chainId: params.chainId,
                shouldSubmit: true,
            },
        });
    }
    submitEthereumTransaction(signedTransaction, chainId) {
        return this.sendRequest({
            method: 'submitEthereumTransaction',
            params: {
                signedTransaction: (0, util_2.hexStringFromBuffer)(signedTransaction, true),
                chainId,
            },
        });
    }
    scanQRCode(regExp) {
        return this.sendRequest({
            method: 'scanQRCode',
            params: {
                regExp,
            },
        });
    }
    getWalletLinkSession() {
        return this._session;
    }
    genericRequest(data, action) {
        return this.sendRequest({
            method: 'generic',
            params: {
                action,
                data,
            },
        });
    }
    sendGenericMessage(request) {
        return this.sendRequest(request);
    }
    sendRequest(request) {
        let hideSnackbarItem = null;
        const id = (0, util_2.randomBytesHex)(8);
        const cancel = (error) => {
            this.publishWeb3RequestCanceledEvent(id);
            this.handleErrorResponse(id, request.method, error);
            hideSnackbarItem === null || hideSnackbarItem === void 0 ? void 0 : hideSnackbarItem();
        };
        return new Promise((resolve, reject) => {
            {
                hideSnackbarItem = this.ui.showConnecting({
                    isUnlinkedErrorState: this.isUnlinkedErrorState,
                    onCancel: cancel,
                    onResetConnection: this.resetAndReload, // eslint-disable-line @typescript-eslint/unbound-method
                });
            }
            this.relayEventManager.callbacks.set(id, (response) => {
                hideSnackbarItem === null || hideSnackbarItem === void 0 ? void 0 : hideSnackbarItem();
                if ((0, Web3Response_1.isErrorResponse)(response)) {
                    return reject(new Error(response.errorMessage));
                }
                resolve(response);
            });
            this.publishWeb3RequestEvent(id, request);
        });
    }
    setAccountsCallback(accountsCallback) {
        this.accountsCallback = accountsCallback;
    }
    setChainCallback(chainCallback) {
        this.chainCallback = chainCallback;
    }
    setDappDefaultChainCallback(chainId) {
        this.dappDefaultChain = chainId;
    }
    publishWeb3RequestEvent(id, request) {
        const message = { type: 'WEB3_REQUEST', id, request };
        this.publishEvent('Web3Request', message, true)
            .then((_) => { })
            .catch((err) => {
            this.handleWeb3ResponseMessage({
                type: 'WEB3_RESPONSE',
                id: message.id,
                response: {
                    method: request.method,
                    errorMessage: err.message,
                },
            });
        });
        if (this.isMobileWeb) {
            this.openCoinbaseWalletDeeplink(request.method);
        }
    }
    // copied from MobileRelay
    openCoinbaseWalletDeeplink(method) {
        if (!(this.ui instanceof WLMobileRelayUI_1.WLMobileRelayUI))
            return;
        // For mobile relay requests, open the Coinbase Wallet app
        switch (method) {
            case 'requestEthereumAccounts': // requestEthereumAccounts is handled via popup
            case 'switchEthereumChain': // switchEthereumChain doesn't need to open the app
                return;
            default:
                window.addEventListener('blur', () => {
                    window.addEventListener('focus', () => {
                        this.connection.checkUnseenEvents();
                    }, { once: true });
                }, { once: true });
                this.ui.openCoinbaseWalletDeeplink();
                break;
        }
    }
    publishWeb3RequestCanceledEvent(id) {
        const message = {
            type: 'WEB3_REQUEST_CANCELED',
            id,
        };
        this.publishEvent('Web3RequestCanceled', message, false).then();
    }
    publishEvent(event, message, callWebhook) {
        return this.connection.publishEvent(event, message, callWebhook);
    }
    handleWeb3ResponseMessage(message) {
        const { response } = message;
        if (response.method === 'requestEthereumAccounts') {
            WalletLinkRelay.accountRequestCallbackIds.forEach((id) => this.invokeCallback(Object.assign(Object.assign({}, message), { id })));
            WalletLinkRelay.accountRequestCallbackIds.clear();
            return;
        }
        this.invokeCallback(message);
    }
    handleErrorResponse(id, method, error) {
        var _a;
        const errorMessage = (_a = error === null || error === void 0 ? void 0 : error.message) !== null && _a !== void 0 ? _a : 'Unspecified error message.';
        this.handleWeb3ResponseMessage({
            type: 'WEB3_RESPONSE',
            id,
            response: {
                method,
                errorMessage,
            },
        });
    }
    invokeCallback(message) {
        const callback = this.relayEventManager.callbacks.get(message.id);
        if (callback) {
            callback(message.response);
            this.relayEventManager.callbacks.delete(message.id);
        }
    }
    requestEthereumAccounts() {
        const request = {
            method: 'requestEthereumAccounts',
            params: {
                appName: this.appName,
                appLogoUrl: this.appLogoUrl || null,
            },
        };
        const hideSnackbarItem = null;
        const id = (0, util_2.randomBytesHex)(8);
        return new Promise((resolve, reject) => {
            this.relayEventManager.callbacks.set(id, (response) => {
                // eslint-disable-next-line @typescript-eslint/ban-ts-comment
                // @ts-ignore
                hideSnackbarItem === null || hideSnackbarItem === void 0 ? void 0 : hideSnackbarItem();
                if ((0, Web3Response_1.isErrorResponse)(response)) {
                    return reject(new Error(response.errorMessage));
                }
                resolve(response);
            });
            WalletLinkRelay.accountRequestCallbackIds.add(id);
            this.publishWeb3RequestEvent(id, request);
        });
    }
    watchAsset(type, address, symbol, decimals, image, chainId) {
        const request = {
            method: 'watchAsset',
            params: {
                type,
                options: {
                    address,
                    symbol,
                    decimals,
                    image,
                },
                chainId,
            },
        };
        let hideSnackbarItem = null;
        const id = (0, util_2.randomBytesHex)(8);
        const cancel = (error) => {
            this.publishWeb3RequestCanceledEvent(id);
            this.handleErrorResponse(id, request.method, error);
            hideSnackbarItem === null || hideSnackbarItem === void 0 ? void 0 : hideSnackbarItem();
        };
        {
            hideSnackbarItem = this.ui.showConnecting({
                isUnlinkedErrorState: this.isUnlinkedErrorState,
                onCancel: cancel,
                onResetConnection: this.resetAndReload, // eslint-disable-line @typescript-eslint/unbound-method
            });
        }
        return new Promise((resolve, reject) => {
            this.relayEventManager.callbacks.set(id, (response) => {
                hideSnackbarItem === null || hideSnackbarItem === void 0 ? void 0 : hideSnackbarItem();
                if ((0, Web3Response_1.isErrorResponse)(response)) {
                    return reject(new Error(response.errorMessage));
                }
                resolve(response);
            });
            this.publishWeb3RequestEvent(id, request);
        });
    }
    addEthereumChain(chainId, rpcUrls, iconUrls, blockExplorerUrls, chainName, nativeCurrency) {
        const request = {
            method: 'addEthereumChain',
            params: {
                chainId,
                rpcUrls,
                blockExplorerUrls,
                chainName,
                iconUrls,
                nativeCurrency,
            },
        };
        let hideSnackbarItem = null;
        const id = (0, util_2.randomBytesHex)(8);
        const cancel = (error) => {
            this.publishWeb3RequestCanceledEvent(id);
            this.handleErrorResponse(id, request.method, error);
            hideSnackbarItem === null || hideSnackbarItem === void 0 ? void 0 : hideSnackbarItem();
        };
        {
            hideSnackbarItem = this.ui.showConnecting({
                isUnlinkedErrorState: this.isUnlinkedErrorState,
                onCancel: cancel,
                onResetConnection: this.resetAndReload, // eslint-disable-line @typescript-eslint/unbound-method
            });
        }
        return new Promise((resolve, reject) => {
            this.relayEventManager.callbacks.set(id, (response) => {
                hideSnackbarItem === null || hideSnackbarItem === void 0 ? void 0 : hideSnackbarItem();
                if ((0, Web3Response_1.isErrorResponse)(response)) {
                    return reject(new Error(response.errorMessage));
                }
                resolve(response);
            });
            this.publishWeb3RequestEvent(id, request);
        });
    }
    switchEthereumChain(chainId, address) {
        const request = {
            method: 'switchEthereumChain',
            params: Object.assign({ chainId }, { address }),
        };
        const id = (0, util_2.randomBytesHex)(8);
        return new Promise((resolve, reject) => {
            this.relayEventManager.callbacks.set(id, (response) => {
                if ((0, Web3Response_1.isErrorResponse)(response) && response.errorCode) {
                    return reject(error_1.standardErrors.provider.custom({
                        code: response.errorCode,
                        message: `Unrecognized chain ID. Try adding the chain using addEthereumChain first.`,
                    }));
                }
                else if ((0, Web3Response_1.isErrorResponse)(response)) {
                    return reject(new Error(response.errorMessage));
                }
                resolve(response);
            });
            this.publishWeb3RequestEvent(id, request);
        });
    }
}
exports.WalletLinkRelay = WalletLinkRelay;
WalletLinkRelay.accountRequestCallbackIds = new Set();
