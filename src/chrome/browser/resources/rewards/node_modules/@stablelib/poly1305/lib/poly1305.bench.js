"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var poly1305_1 = require("./poly1305");
var benchmark_1 = require("@stablelib/benchmark");
var buf8192 = benchmark_1.byteSeq(8192);
var buf1024 = benchmark_1.byteSeq(1024);
var buf32 = benchmark_1.byteSeq(32);
var key = benchmark_1.byteSeq(32);
benchmark_1.report("Poly1305 8K", benchmark_1.benchmark(function () { return poly1305_1.oneTimeAuth(key, buf8192); }, buf8192.length));
benchmark_1.report("Poly1305 1K", benchmark_1.benchmark(function () { return poly1305_1.oneTimeAuth(key, buf1024); }, buf1024.length));
benchmark_1.report("Poly1305 32", benchmark_1.benchmark(function () { return poly1305_1.oneTimeAuth(key, buf32); }, buf32.length));
//# sourceMappingURL=poly1305.bench.js.map