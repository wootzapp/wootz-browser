"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var sha256_1 = require("./sha256");
var benchmark_1 = require("@stablelib/benchmark");
var buf1M = benchmark_1.byteSeq(1024 << 10);
var buf8K = benchmark_1.byteSeq(8 << 10);
var buf1K = benchmark_1.byteSeq(1 << 10);
var buf32 = benchmark_1.byteSeq(32);
benchmark_1.report("SHA256 1M", benchmark_1.benchmark(function () { return sha256_1.hash(buf1M); }, buf1M.length));
benchmark_1.report("SHA256 8K", benchmark_1.benchmark(function () { return sha256_1.hash(buf8K); }, buf8K.length));
benchmark_1.report("SHA256 1K", benchmark_1.benchmark(function () { return sha256_1.hash(buf1K); }, buf1K.length));
benchmark_1.report("SHA256 32", benchmark_1.benchmark(function () { return sha256_1.hash(buf32); }, buf32.length));
//# sourceMappingURL=sha256.bench.js.map