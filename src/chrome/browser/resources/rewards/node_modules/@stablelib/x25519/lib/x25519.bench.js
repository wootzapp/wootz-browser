"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
const x25519_1 = require("./x25519");
const keyagreement_1 = require("./keyagreement");
const benchmark_1 = require("@stablelib/benchmark");
const r = new Uint8Array(32);
r[0] = 1;
(0, benchmark_1.report)("x25519.scalarMultBase", (0, benchmark_1.benchmark)(() => (0, x25519_1.scalarMultBase)(r)));
const seed = (0, benchmark_1.byteSeq)(32);
const offerMsg = new keyagreement_1.X25519KeyAgreement(seed).offer();
const acceptMsg = new keyagreement_1.X25519KeyAgreement(seed).accept(offerMsg);
(0, benchmark_1.report)("X25519KeyAgreement offer/finish", (0, benchmark_1.benchmark)(() => {
    const state = new keyagreement_1.X25519KeyAgreement(seed);
    state.offer();
    state.finish(acceptMsg);
}));
(0, benchmark_1.report)("X25519KeyAgreement accept", (0, benchmark_1.benchmark)(() => new keyagreement_1.X25519KeyAgreement(seed).accept(offerMsg)));
//# sourceMappingURL=x25519.bench.js.map