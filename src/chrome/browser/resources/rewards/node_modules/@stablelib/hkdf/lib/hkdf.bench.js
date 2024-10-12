"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var hkdf_1 = require("./hkdf");
var sha256_1 = require("@stablelib/sha256");
var sha512_1 = require("@stablelib/sha512");
var benchmark_1 = require("@stablelib/benchmark");
var key = benchmark_1.byteSeq(32);
var salt = benchmark_1.byteSeq(32);
var info = benchmark_1.byteSeq(5);
benchmark_1.report("hkdf/sha256 64", benchmark_1.benchmark(function () { return new hkdf_1.HKDF(sha256_1.SHA256, key, salt, info).expand(64); }));
benchmark_1.report("hkdf/sha256 1K", benchmark_1.benchmark(function () { return new hkdf_1.HKDF(sha256_1.SHA256, key, salt, info).expand(1024); }));
benchmark_1.report("hkdf/sha512 64", benchmark_1.benchmark(function () { return new hkdf_1.HKDF(sha512_1.SHA512, key, salt, info).expand(64); }));
benchmark_1.report("hkdf/sha512 1K", benchmark_1.benchmark(function () { return new hkdf_1.HKDF(sha512_1.SHA512, key, salt, info).expand(1024); }));
//# sourceMappingURL=hkdf.bench.js.map