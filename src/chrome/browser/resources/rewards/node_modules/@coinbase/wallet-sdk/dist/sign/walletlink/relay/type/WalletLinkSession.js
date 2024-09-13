"use strict";
// Copyright (c) 2018-2023 Coinbase, Inc. <https://www.coinbase.com/>
Object.defineProperty(exports, "__esModule", { value: true });
exports.WalletLinkSession = void 0;
const sha_js_1 = require("sha.js");
const util_1 = require("../../../../core/type/util");
const STORAGE_KEY_SESSION_ID = 'session:id';
const STORAGE_KEY_SESSION_SECRET = 'session:secret';
const STORAGE_KEY_SESSION_LINKED = 'session:linked';
class WalletLinkSession {
    constructor(storage, id, secret, linked) {
        this._storage = storage;
        this._id = id || (0, util_1.randomBytesHex)(16);
        this._secret = secret || (0, util_1.randomBytesHex)(32);
        this._key = new sha_js_1.sha256()
            .update(`${this._id}, ${this._secret} WalletLink`) // ensure old sessions stay connected
            .digest('hex');
        this._linked = !!linked;
    }
    static load(storage) {
        const id = storage.getItem(STORAGE_KEY_SESSION_ID);
        const linked = storage.getItem(STORAGE_KEY_SESSION_LINKED);
        const secret = storage.getItem(STORAGE_KEY_SESSION_SECRET);
        if (id && secret) {
            return new WalletLinkSession(storage, id, secret, linked === '1');
        }
        return null;
    }
    get id() {
        return this._id;
    }
    get secret() {
        return this._secret;
    }
    get key() {
        return this._key;
    }
    get linked() {
        return this._linked;
    }
    set linked(val) {
        this._linked = val;
        this.persistLinked();
    }
    save() {
        this._storage.setItem(STORAGE_KEY_SESSION_ID, this._id);
        this._storage.setItem(STORAGE_KEY_SESSION_SECRET, this._secret);
        this.persistLinked();
        return this;
    }
    persistLinked() {
        this._storage.setItem(STORAGE_KEY_SESSION_LINKED, this._linked ? '1' : '0');
    }
}
exports.WalletLinkSession = WalletLinkSession;
