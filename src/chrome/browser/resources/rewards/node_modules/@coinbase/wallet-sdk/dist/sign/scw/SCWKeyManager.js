"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.SCWKeyManager = void 0;
const cipher_1 = require("../../util/cipher");
const ScopedLocalStorage_1 = require("../../util/ScopedLocalStorage");
const OWN_PRIVATE_KEY = {
    storageKey: 'ownPrivateKey',
    keyType: 'private',
};
const OWN_PUBLIC_KEY = {
    storageKey: 'ownPublicKey',
    keyType: 'public',
};
const PEER_PUBLIC_KEY = {
    storageKey: 'peerPublicKey',
    keyType: 'public',
};
class SCWKeyManager {
    constructor() {
        this.storage = new ScopedLocalStorage_1.ScopedLocalStorage('CBWSDK', 'SCWKeyManager');
        this.ownPrivateKey = null;
        this.ownPublicKey = null;
        this.peerPublicKey = null;
        this.sharedSecret = null;
    }
    async getOwnPublicKey() {
        await this.loadKeysIfNeeded();
        return this.ownPublicKey;
    }
    // returns null if the shared secret is not yet derived
    async getSharedSecret() {
        await this.loadKeysIfNeeded();
        return this.sharedSecret;
    }
    async setPeerPublicKey(key) {
        this.sharedSecret = null;
        this.peerPublicKey = key;
        await this.storeKey(PEER_PUBLIC_KEY, key);
        await this.loadKeysIfNeeded();
    }
    async clear() {
        this.ownPrivateKey = null;
        this.ownPublicKey = null;
        this.peerPublicKey = null;
        this.sharedSecret = null;
        this.storage.removeItem(OWN_PUBLIC_KEY.storageKey);
        this.storage.removeItem(OWN_PRIVATE_KEY.storageKey);
        this.storage.removeItem(PEER_PUBLIC_KEY.storageKey);
    }
    async generateKeyPair() {
        const newKeyPair = await (0, cipher_1.generateKeyPair)();
        this.ownPrivateKey = newKeyPair.privateKey;
        this.ownPublicKey = newKeyPair.publicKey;
        await this.storeKey(OWN_PRIVATE_KEY, newKeyPair.privateKey);
        await this.storeKey(OWN_PUBLIC_KEY, newKeyPair.publicKey);
    }
    async loadKeysIfNeeded() {
        if (this.ownPrivateKey === null) {
            this.ownPrivateKey = await this.loadKey(OWN_PRIVATE_KEY);
        }
        if (this.ownPublicKey === null) {
            this.ownPublicKey = await this.loadKey(OWN_PUBLIC_KEY);
        }
        if (this.ownPrivateKey === null || this.ownPublicKey === null) {
            await this.generateKeyPair();
        }
        if (this.peerPublicKey === null) {
            this.peerPublicKey = await this.loadKey(PEER_PUBLIC_KEY);
        }
        if (this.sharedSecret === null) {
            if (this.ownPrivateKey === null || this.peerPublicKey === null)
                return;
            this.sharedSecret = await (0, cipher_1.deriveSharedSecret)(this.ownPrivateKey, this.peerPublicKey);
        }
    }
    // storage methods
    async loadKey(item) {
        const key = this.storage.getItem(item.storageKey);
        if (!key)
            return null;
        return (0, cipher_1.importKeyFromHexString)(item.keyType, key);
    }
    async storeKey(item, key) {
        const hexString = await (0, cipher_1.exportKeyToHexString)(item.keyType, key);
        this.storage.setItem(item.storageKey, hexString);
    }
}
exports.SCWKeyManager = SCWKeyManager;
