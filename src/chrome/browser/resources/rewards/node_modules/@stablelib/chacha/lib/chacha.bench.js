"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var chacha_1 = require("./chacha");
var benchmark_1 = require("@stablelib/benchmark");
var buf16K = benchmark_1.byteSeq(16 << 10);
var buf8192 = benchmark_1.byteSeq(8192);
var buf1111 = benchmark_1.byteSeq(1111);
var buf64 = benchmark_1.byteSeq(64);
var key = benchmark_1.byteSeq(32);
var nonce = benchmark_1.byteSeq(8);
benchmark_1.report("ChaCha20 xor 16K", benchmark_1.benchmark(function () { return chacha_1.streamXOR(key, nonce, buf16K, buf16K); }, buf16K.length));
benchmark_1.report("ChaCha20 xor 8K", benchmark_1.benchmark(function () { return chacha_1.streamXOR(key, nonce, buf8192, buf8192); }, buf8192.length));
benchmark_1.report("ChaCha20 xor 1111", benchmark_1.benchmark(function () { return chacha_1.streamXOR(key, nonce, buf1111, buf1111); }, buf1111.length));
benchmark_1.report("ChaCha20 xor 64", benchmark_1.benchmark(function () { return chacha_1.streamXOR(key, nonce, buf64, buf64); }, buf64.length));
//# sourceMappingURL=chacha.bench.js.map