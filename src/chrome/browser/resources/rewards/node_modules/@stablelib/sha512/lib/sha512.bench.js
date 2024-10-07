"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var sha512_1 = require("./sha512");
var benchmark_1 = require("@stablelib/benchmark");
var buf1M = benchmark_1.byteSeq(1024 << 10);
var buf8K = benchmark_1.byteSeq(8 << 10);
var buf1K = benchmark_1.byteSeq(1 << 10);
var buf32 = benchmark_1.byteSeq(32);
benchmark_1.report("SHA512 1M", benchmark_1.benchmark(function () { return sha512_1.hash(buf1M); }, buf1M.length));
benchmark_1.report("SHA512 8K", benchmark_1.benchmark(function () { return sha512_1.hash(buf8K); }, buf8K.length));
benchmark_1.report("SHA512 1K", benchmark_1.benchmark(function () { return sha512_1.hash(buf1K); }, buf1K.length));
benchmark_1.report("SHA512 32", benchmark_1.benchmark(function () { return sha512_1.hash(buf32); }, buf32.length));
//# sourceMappingURL=sha512.bench.js.map