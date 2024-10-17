"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var chacha20poly1305_1 = require("./chacha20poly1305");
var benchmark_1 = require("@stablelib/benchmark");
var buf8192 = benchmark_1.byteSeq(8192);
var buf1111 = benchmark_1.byteSeq(1111);
var buf0 = new Uint8Array(0);
var key = benchmark_1.byteSeq(32);
var nonce = benchmark_1.byteSeq(12);
var aead = new chacha20poly1305_1.ChaCha20Poly1305(key);
benchmark_1.report("ChaCha20Poly1305 seal 8K", benchmark_1.benchmark(function () { return aead.seal(nonce, buf8192); }, buf8192.length));
benchmark_1.report("ChaCha20Poly1305 seal 1111", benchmark_1.benchmark(function () { return aead.seal(nonce, buf1111); }, buf1111.length));
benchmark_1.report("ChaCha20Poly1305 seal 8K + AD", benchmark_1.benchmark(function () { return aead.seal(nonce, buf8192, buf8192); }, buf8192.length * 2));
benchmark_1.report("ChaCha20Poly1305 seal 1111 + AD", benchmark_1.benchmark(function () { return aead.seal(nonce, buf1111, buf1111); }, buf1111.length * 2));
benchmark_1.report("ChaCha20Poly1305 seal 0 + AD 8K", benchmark_1.benchmark(function () { return aead.seal(nonce, buf0, buf8192); }, buf8192.length));
var sealed8192 = aead.seal(nonce, buf8192);
var sealed1111 = aead.seal(nonce, buf1111);
var sealed8192ad = aead.seal(nonce, buf8192, buf8192);
var sealed1111ad = aead.seal(nonce, buf1111, buf1111);
benchmark_1.report("ChaCha20Poly1305 open 8K", benchmark_1.benchmark(function () { return aead.open(nonce, sealed8192); }, buf8192.length));
benchmark_1.report("ChaCha20Poly1305 open 1111", benchmark_1.benchmark(function () { return aead.open(nonce, sealed1111); }, buf1111.length));
benchmark_1.report("ChaCha20Poly1305 open 8K + AD", benchmark_1.benchmark(function () { return aead.open(nonce, sealed8192ad, buf8192); }, buf8192.length * 2));
benchmark_1.report("ChaCha20Poly1305 open 1111 + AD", benchmark_1.benchmark(function () { return aead.seal(nonce, sealed1111ad, buf1111); }, buf1111.length * 2));
sealed8192[0] ^= sealed8192[0];
benchmark_1.report("ChaCha20Poly1305 open (bad)", benchmark_1.benchmark(function () { return aead.open(nonce, sealed8192); }, buf8192.length));
//# sourceMappingURL=chacha20poly1305.bench.js.map