"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
const ed25519_1 = require("./ed25519");
const benchmark_1 = require("@stablelib/benchmark");
const k = (0, ed25519_1.generateKeyPair)();
const buf = new Uint8Array(256);
const seed = k.secretKey.subarray(0, 32);
const sig = (0, ed25519_1.sign)(k.secretKey, buf);
const badsig = new Uint8Array(sig);
badsig[0] = 1;
(0, benchmark_1.report)("ed25519.generateKeyPairFromSeed", (0, benchmark_1.benchmark)(() => (0, ed25519_1.generateKeyPairFromSeed)(seed)));
(0, benchmark_1.report)("ed25519.sign", (0, benchmark_1.benchmark)(() => (0, ed25519_1.sign)(k.secretKey, buf)));
(0, benchmark_1.report)("ed25519.verify", (0, benchmark_1.benchmark)(() => (0, ed25519_1.verify)(k.publicKey, buf, sig)));
(0, benchmark_1.report)("ed25519.verify (bad)", (0, benchmark_1.benchmark)(() => (0, ed25519_1.verify)(k.publicKey, buf, badsig)));
//# sourceMappingURL=ed25519.bench.js.map