"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
const hex_1 = require("@stablelib/hex");
const x25519_1 = require("./x25519");
const keyagreement_1 = require("./keyagreement");
describe("x25519.scalarMultBase", () => {
    it("should return correct result", () => {
        // This takes takes a bit of time.
        // Similar to https://code.google.com/p/go/source/browse/curve25519/curve25519_test.go?repo=crypto
        const golden = (0, hex_1.decode)("89161fde887b2b53de549af483940106ecc114d6982daa98256de23bdf77661a");
        let input = new Uint8Array(32);
        input[0] = 1;
        for (let i = 0; i < 200; i++) {
            input = (0, x25519_1.scalarMultBase)(input);
        }
        expect(input).toEqual(golden);
    });
    it("should calculate shared key", () => {
        const k0 = (0, x25519_1.generateKeyPair)();
        const k1 = (0, x25519_1.generateKeyPair)();
        const s0 = (0, x25519_1.sharedKey)(k0.secretKey, k1.publicKey);
        const s1 = (0, x25519_1.sharedKey)(k1.secretKey, k0.publicKey);
        expect(s0).toEqual(s1);
    });
    it("should throw if shared key is all-zero and rejectZero = true", () => {
        const k = (0, x25519_1.generateKeyPair)();
        const z = new Uint8Array(32);
        expect(() => (0, x25519_1.sharedKey)(k.secretKey, z, true)).toThrowError(/invalid/);
    });
    it("should not throw if shared key is all-zero and rejectZero = false", () => {
        const k = (0, x25519_1.generateKeyPair)();
        const z = new Uint8Array(32);
        expect((0, x25519_1.sharedKey)(k.secretKey, z)).toBeTruthy();
    });
});
// For testing with generated test vectors, instead of proper PRNG
// use the same deterministic generator that generates byte sequences
// of 0, 1, 2, 3, ... that was used to create vectors.
//
// Note: in the original implementation to generate test vectors
// poly.c, crypto_stream_chacha20 needs to be replaced with randombytes,
// and randombytes must generate the same sequence as this. It also
// needs to be reset to 0 for server part of handshake and 64 for
// client part, like in the test below.
class BadSource {
    constructor(v = 0) {
        this.v = v;
        this.isAvailable = true;
    }
    randomBytes(length) {
        const out = new Uint8Array(length);
        for (let i = 0; i < out.length; i++) {
            out[i] = this.v;
            this.v = (this.v + 1) & 0xff;
        }
        return out;
    }
}
const testVector = {
    offerMsg: "8F40C5ADB68F25624AE5B214EA767A6EC94D829D3D7B5E1AD1BA6F3E2138285F",
    acceptMsg: "79A631EEDE1BF9C98F12032CDEADD0E7A079398FC786B88CC846EC89AF85A51A",
    sharedKey: "6D54CC9C397E31691401110F58DA1E182A635D7E44C21DC2D7BE93624652AB15"
};
describe("X25519KeyAgreement", () => {
    it("should establish shared key", () => {
        for (let i = 0; i < 5; i++) {
            const server = new keyagreement_1.X25519KeyAgreement();
            const offerMsg = server.offer();
            // console.log("offerMsg:", encode(offerMsg));
            const client = new keyagreement_1.X25519KeyAgreement();
            const acceptMsg = client.accept(offerMsg);
            // console.log("acceptMsg:", encode(acceptMsg));
            server.finish(acceptMsg);
            const serverKey = server.getSharedKey();
            const clientKey = client.getSharedKey();
            // console.log("serverKey:", encode(serverKey));
            // console.log("clientKey:", encode(clientKey));
            expect((0, hex_1.encode)(serverKey)).toEqual((0, hex_1.encode)(clientKey));
        }
    });
    it("should match test vector", () => {
        const serverPrng = new BadSource(0);
        const server = new keyagreement_1.X25519KeyAgreement(undefined, serverPrng);
        const offerMsg = server.offer();
        expect("offerMsg: " + (0, hex_1.encode)(offerMsg))
            .toEqual("offerMsg: " + testVector.offerMsg);
        const clientPrng = new BadSource(64);
        const client = new keyagreement_1.X25519KeyAgreement(undefined, clientPrng);
        const acceptMsg = client.accept(offerMsg);
        expect("acceptMsg: " + (0, hex_1.encode)(acceptMsg))
            .toEqual("acceptMsg: " + testVector.acceptMsg);
        server.finish(acceptMsg);
        expect((0, hex_1.encode)(server.getSharedKey())).toEqual(testVector.sharedKey);
        expect((0, hex_1.encode)(client.getSharedKey())).toEqual(testVector.sharedKey);
    });
});
//# sourceMappingURL=x25519.test.js.map