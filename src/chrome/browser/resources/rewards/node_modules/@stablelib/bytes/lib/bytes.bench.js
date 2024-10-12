"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var bytes_1 = require("./bytes");
var benchmark_1 = require("@stablelib/benchmark");
var a0 = new Uint8Array([1, 2, 3]);
var a1 = new Uint8Array([4]);
var a2 = new Uint8Array(0); // empty
var a3 = new Uint8Array([5, 6, 7, 8, 9, 10]);
benchmark_1.report("bytes.concat", benchmark_1.benchmark(function () { return bytes_1.concat(a0, a1, a2, a3); }));
//# sourceMappingURL=bytes.bench.js.map