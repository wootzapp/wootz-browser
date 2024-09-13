"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var hkdf_1 = require("./hkdf");
var sha256_1 = require("@stablelib/sha256");
var hex_1 = require("@stablelib/hex");
var testVectors = [
    // From RFC5869, SHA256-only.
    {
        hash: sha256_1.SHA256,
        key: "0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B",
        salt: "000102030405060708090A0B0C",
        info: "F0F1F2F3F4F5F6F7F8F9",
        result: "3CB25F25FAACD57A90434F64D0362F2A2D2D0A90CF1A5A4C5DB02D56ECC4C5BF34007208D5B887185865"
    },
    {
        hash: sha256_1.SHA256,
        key: "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F2021222324252627" +
            "28292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F404142434445464748494A4B4C4D4E4F",
        salt: "606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F808182838" +
            "485868788898A8B8C8D8E8F909192939495969798999A9B9C9D9E9FA0A1A2A3A4A5A6A" +
            "7A8A9AAABACADAEAF",
        info: "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBFC0C1C2C3C4C5C6C7C8C9CACBCCCDCECFD0" +
            "D1D2D3D4D5D6D7D8D9DADBDCDDDEDFE0E1E2E3E4E5E6E7E8E9EAEBECEDEEEFF0F1F2F3F4" +
            "F5F6F7F8F9FAFBFCFDFEFF",
        result: "B11E398DC80327A1C8E7F78C596A49344F012EDA2D4EFAD8A050CC4C19AFA97C59045A99" +
            "CAC7827271CB41C65E590E09DA3275600C2F09B8367793A9ACA3DB71CC30C58179EC3E87C14" +
            "C01D5C1F3434F1D87"
    },
    {
        hash: sha256_1.SHA256,
        key: "0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B0B",
        salt: "",
        info: "",
        result: "8DA4E775A563C18F715F802A063C5A31B8A11F5C5EE1879EC3454E5F3C738D2D9D201395FAA4B61A96C8"
    }
];
describe("HKDF", function () {
    it("should derive correct keys", function () {
        testVectors.forEach(function (v) {
            var h = new hkdf_1.HKDF(v.hash, hex_1.decode(v.key), hex_1.decode(v.salt), hex_1.decode(v.info));
            var result = h.expand(v.result.length / 2);
            expect(hex_1.encode(result)).toBe(v.result);
        });
    });
    it("should throw if generated more than 255 blocks", function () {
        var h = new hkdf_1.HKDF(sha256_1.SHA256, new Uint8Array([0, 1, 2, 3, 4]));
        for (var i = 0; i < 255; i++) {
            h.expand(sha256_1.DIGEST_LENGTH);
        }
        expect(function () { return h.expand(1); }).toThrowError("hkdf: cannot expand more");
    });
    it("should expand in parts like in full", function () {
        var key = new Uint8Array([0, 1, 2, 3, 4]);
        var salt = new Uint8Array([5, 6, 7, 8, 9, 10, 11]);
        var info = new Uint8Array([12, 13, 14, 15, 16, 17, 18]);
        // Full expansion.
        var full = new hkdf_1.HKDF(sha256_1.SHA256, key, salt, info).expand(300);
        // Sliced expansions.
        var h = new hkdf_1.HKDF(sha256_1.SHA256, key, salt, info);
        var s1 = h.expand(1); // 1
        var s2 = h.expand(100); // 101
        var s3 = h.expand(27); // 128
        var s4 = h.expand(32); // 160
        var s5 = h.expand(140); // 300
        var se = hex_1.encode(s1) + hex_1.encode(s2) + hex_1.encode(s3) + hex_1.encode(s4) + hex_1.encode(s5);
        expect(se).toBe(hex_1.encode(full));
    });
});
//# sourceMappingURL=hkdf.test.js.map