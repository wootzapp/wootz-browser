"use strict";
// Copyright (C) 2020 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
exports.X25519KeyAgreement = exports.SECRET_SEED_LENGTH = exports.SAVED_STATE_LENGTH = exports.ACCEPT_MESSAGE_LENGTH = exports.OFFER_MESSAGE_LENGTH = void 0;
const random_1 = require("@stablelib/random");
const wipe_1 = require("@stablelib/wipe");
const x25519_1 = require("./x25519");
/** Constants for key agreement */
exports.OFFER_MESSAGE_LENGTH = x25519_1.PUBLIC_KEY_LENGTH;
exports.ACCEPT_MESSAGE_LENGTH = x25519_1.PUBLIC_KEY_LENGTH;
exports.SAVED_STATE_LENGTH = x25519_1.SECRET_KEY_LENGTH;
exports.SECRET_SEED_LENGTH = x25519_1.SECRET_KEY_LENGTH;
/**
 * X25519 key agreement using ephemeral key pairs.
 *
 * Note that unless this key agreement is combined with an authentication
 * method, such as public key signatures, it's vulnerable to man-in-the-middle
 * attacks.
 */
class X25519KeyAgreement {
    constructor(secretSeed, prng) {
        this.offerMessageLength = exports.OFFER_MESSAGE_LENGTH;
        this.acceptMessageLength = exports.ACCEPT_MESSAGE_LENGTH;
        this.sharedKeyLength = x25519_1.SHARED_KEY_LENGTH;
        this.savedStateLength = exports.SAVED_STATE_LENGTH;
        this._offered = false;
        this._secretKey = secretSeed || (0, random_1.randomBytes)(x25519_1.SECRET_KEY_LENGTH, prng);
    }
    saveState() {
        return new Uint8Array(this._secretKey);
    }
    restoreState(savedState) {
        this._secretKey = new Uint8Array(savedState);
        return this;
    }
    clean() {
        if (this._secretKey) {
            (0, wipe_1.wipe)(this._secretKey);
        }
        if (this._sharedKey) {
            (0, wipe_1.wipe)(this._sharedKey);
        }
    }
    offer() {
        this._offered = true;
        const keyPair = (0, x25519_1.generateKeyPairFromSeed)(this._secretKey);
        return keyPair.publicKey;
    }
    accept(offerMsg) {
        if (this._offered) {
            throw new Error("X25519KeyAgreement: accept shouldn't be called by offering party");
        }
        if (offerMsg.length !== this.offerMessageLength) {
            throw new Error("X25519KeyAgreement: incorrect offer message length");
        }
        const keyPair = (0, x25519_1.generateKeyPairFromSeed)(this._secretKey);
        this._sharedKey = (0, x25519_1.sharedKey)(keyPair.secretKey, offerMsg);
        (0, wipe_1.wipe)(keyPair.secretKey);
        return keyPair.publicKey;
    }
    finish(acceptMsg) {
        if (acceptMsg.length !== this.acceptMessageLength) {
            throw new Error("X25519KeyAgreement: incorrect accept message length");
        }
        if (!this._secretKey) {
            throw new Error("X25519KeyAgreement: no offer state");
        }
        if (this._sharedKey) {
            throw new Error("X25519KeyAgreement: finish was already called");
        }
        this._sharedKey = (0, x25519_1.sharedKey)(this._secretKey, acceptMsg);
        return this;
    }
    getSharedKey() {
        if (!this._sharedKey) {
            throw new Error("X25519KeyAgreement: no shared key established");
        }
        return new Uint8Array(this._sharedKey);
    }
}
exports.X25519KeyAgreement = X25519KeyAgreement;
//# sourceMappingURL=keyagreement.js.map