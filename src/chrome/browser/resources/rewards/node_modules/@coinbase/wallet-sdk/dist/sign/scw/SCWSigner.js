"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.SCWSigner = void 0;
const SCWKeyManager_1 = require("./SCWKeyManager");
const SCWStateManager_1 = require("./SCWStateManager");
const error_1 = require("../../core/error");
const util_1 = require("../../core/type/util");
const cipher_1 = require("../../util/cipher");
class SCWSigner {
    constructor(params) {
        this.metadata = params.metadata;
        this.communicator = params.communicator;
        this.keyManager = new SCWKeyManager_1.SCWKeyManager();
        this.stateManager = new SCWStateManager_1.SCWStateManager({
            appChainIds: this.metadata.appChainIds,
            updateListener: params.updateListener,
        });
        this.handshake = this.handshake.bind(this);
        this.request = this.request.bind(this);
        this.createRequestMessage = this.createRequestMessage.bind(this);
        this.decryptResponseMessage = this.decryptResponseMessage.bind(this);
    }
    async handshake() {
        const handshakeMessage = await this.createRequestMessage({
            handshake: {
                method: 'eth_requestAccounts',
                params: this.metadata,
            },
        });
        const response = await this.communicator.postRequestAndWaitForResponse(handshakeMessage);
        // store peer's public key
        if ('failure' in response.content)
            throw response.content.failure;
        const peerPublicKey = await (0, cipher_1.importKeyFromHexString)('public', response.sender);
        await this.keyManager.setPeerPublicKey(peerPublicKey);
        const decrypted = await this.decryptResponseMessage(response);
        this.updateInternalState({ method: 'eth_requestAccounts' }, decrypted);
        const result = decrypted.result;
        if ('error' in result)
            throw result.error;
        return this.stateManager.accounts;
    }
    async request(request) {
        const localResult = this.tryLocalHandling(request);
        if (localResult !== undefined) {
            if (localResult instanceof Error)
                throw localResult;
            return localResult;
        }
        // Open the popup before constructing the request message.
        // This is to ensure that the popup is not blocked by some browsers (i.e. Safari)
        await this.communicator.waitForPopupLoaded();
        const response = await this.sendEncryptedRequest(request);
        const decrypted = await this.decryptResponseMessage(response);
        this.updateInternalState(request, decrypted);
        const result = decrypted.result;
        if ('error' in result)
            throw result.error;
        return result.value;
    }
    async disconnect() {
        this.stateManager.clear();
        await this.keyManager.clear();
    }
    tryLocalHandling(request) {
        var _a;
        switch (request.method) {
            case 'wallet_switchEthereumChain': {
                const params = request.params;
                if (!params || !((_a = params[0]) === null || _a === void 0 ? void 0 : _a.chainId)) {
                    throw error_1.standardErrors.rpc.invalidParams();
                }
                const chainId = (0, util_1.ensureIntNumber)(params[0].chainId);
                const switched = this.stateManager.switchChain(chainId);
                // "return null if the request was successful"
                // https://eips.ethereum.org/EIPS/eip-3326#wallet_switchethereumchain
                return switched ? null : undefined;
            }
            case 'wallet_getCapabilities': {
                const walletCapabilities = this.stateManager.walletCapabilities;
                if (!walletCapabilities) {
                    // This should never be the case for scw connections as capabilities are set during handshake
                    throw error_1.standardErrors.provider.unauthorized('No wallet capabilities found, please disconnect and reconnect');
                }
                return walletCapabilities;
            }
            default:
                return undefined;
        }
    }
    async sendEncryptedRequest(request) {
        const sharedSecret = await this.keyManager.getSharedSecret();
        if (!sharedSecret) {
            throw error_1.standardErrors.provider.unauthorized('No valid session found, try requestAccounts before other methods');
        }
        const encrypted = await (0, cipher_1.encryptContent)({
            action: request,
            chainId: this.stateManager.activeChain.id,
        }, sharedSecret);
        const message = await this.createRequestMessage({ encrypted });
        return this.communicator.postRequestAndWaitForResponse(message);
    }
    async createRequestMessage(content) {
        const publicKey = await (0, cipher_1.exportKeyToHexString)('public', await this.keyManager.getOwnPublicKey());
        return {
            id: crypto.randomUUID(),
            sender: publicKey,
            content,
            timestamp: new Date(),
        };
    }
    async decryptResponseMessage(message) {
        const content = message.content;
        // throw protocol level error
        if ('failure' in content) {
            throw content.failure;
        }
        const sharedSecret = await this.keyManager.getSharedSecret();
        if (!sharedSecret) {
            throw error_1.standardErrors.provider.unauthorized('Invalid session');
        }
        return (0, cipher_1.decryptContent)(content.encrypted, sharedSecret);
    }
    updateInternalState(request, response) {
        var _a, _b;
        const availableChains = (_a = response.data) === null || _a === void 0 ? void 0 : _a.chains;
        if (availableChains) {
            this.stateManager.updateAvailableChains(availableChains);
        }
        const walletCapabilities = (_b = response.data) === null || _b === void 0 ? void 0 : _b.capabilities;
        if (walletCapabilities) {
            this.stateManager.updateWalletCapabilities(walletCapabilities);
        }
        const result = response.result;
        if ('error' in result)
            return;
        switch (request.method) {
            case 'eth_requestAccounts': {
                const accounts = result.value;
                this.stateManager.updateAccounts(accounts);
                break;
            }
            case 'wallet_switchEthereumChain': {
                // "return null if the request was successful"
                // https://eips.ethereum.org/EIPS/eip-3326#wallet_switchethereumchain
                if (result.value !== null)
                    return;
                const params = request.params;
                const chainId = (0, util_1.ensureIntNumber)(params[0].chainId);
                this.stateManager.switchChain(chainId);
                break;
            }
            default:
                break;
        }
    }
}
exports.SCWSigner = SCWSigner;
