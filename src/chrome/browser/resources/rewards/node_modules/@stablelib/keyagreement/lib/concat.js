"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var bytes_1 = require("@stablelib/bytes");
/**
 * ConcatKeyAgreement concatenates many key agreements into one.
 * Each message is a concatenation of underlying key agreement
 * messages, and shared key is a concatenation of all shared keys.
 */
var ConcatKeyAgreement = /** @class */ (function () {
    function ConcatKeyAgreement(agreements) {
        this.agreements = agreements;
        this.offerMessageLength = 0;
        this.acceptMessageLength = 0;
        this.sharedKeyLength = 0;
        this.savedStateLength = 0;
        for (var i = 0; i < agreements.length; i++) {
            var a = agreements[i];
            this.offerMessageLength += a.offerMessageLength;
            this.acceptMessageLength += a.acceptMessageLength;
            this.sharedKeyLength += a.sharedKeyLength;
            this.savedStateLength += a.savedStateLength;
        }
    }
    ConcatKeyAgreement.prototype.saveState = function () {
        return bytes_1.concat.apply(null, this.agreements.map(function (a) { return a.saveState(); }));
    };
    ConcatKeyAgreement.prototype.restoreState = function (savedState) {
        var offset = 0;
        this.agreements.forEach(function (a) {
            a.restoreState(savedState.subarray(offset, offset + a.savedStateLength));
            offset += a.savedStateLength;
        });
        return this;
    };
    ConcatKeyAgreement.prototype.clean = function () {
        this.agreements.forEach(function (a) { return a.clean(); });
    };
    ConcatKeyAgreement.prototype.offer = function () {
        return bytes_1.concat.apply(null, this.agreements.map(function (a) { return a.offer(); }));
    };
    ConcatKeyAgreement.prototype.accept = function (offerMsg) {
        if (offerMsg.length !== this.offerMessageLength) {
            throw new Error("KeyAgreement: incorrect offer message length");
        }
        var offset = 0;
        var results = [];
        this.agreements.forEach(function (a) {
            results.push(a.accept(offerMsg.subarray(offset, offset + a.offerMessageLength)));
            offset += a.offerMessageLength;
        });
        return bytes_1.concat.apply(null, results);
    };
    ConcatKeyAgreement.prototype.finish = function (acceptMsg) {
        if (acceptMsg.length !== this.acceptMessageLength) {
            throw new Error("KeyAgreement: incorrect accept message length");
        }
        var offset = 0;
        this.agreements.forEach(function (a) {
            a.finish(acceptMsg.subarray(offset, offset + a.acceptMessageLength));
            offset += a.acceptMessageLength;
        });
        return this;
    };
    ConcatKeyAgreement.prototype.getSharedKey = function () {
        return bytes_1.concat.apply(null, this.agreements.map(function (a) { return a.getSharedKey(); }));
    };
    return ConcatKeyAgreement;
}());
exports.ConcatKeyAgreement = ConcatKeyAgreement;
//# sourceMappingURL=concat.js.map