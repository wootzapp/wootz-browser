"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var constant_time_1 = require("./constant-time");
describe("constant-time.select", function () {
    it("should select correct value", function () {
        expect(constant_time_1.select(1, 2, 3)).toBe(2);
        expect(constant_time_1.select(1, 3, 2)).toBe(3);
        expect(constant_time_1.select(0, 2, 3)).toBe(3);
        expect(constant_time_1.select(0, 3, 2)).toBe(2);
    });
});
describe("constant-time.lessOrEqual", function () {
    it("should return correct result", function () {
        expect(constant_time_1.lessOrEqual(0, 0)).toBe(1);
        expect(constant_time_1.lessOrEqual(0, Math.pow(2, 31) - 1)).toBe(1);
        expect(constant_time_1.lessOrEqual(2, 3)).toBe(1);
        expect(constant_time_1.lessOrEqual(3, 3)).toBe(1);
        expect(constant_time_1.lessOrEqual(4, 3)).toBe(0);
        expect(constant_time_1.lessOrEqual(5, 3)).toBe(0);
    });
});
describe("constant-time.compare", function () {
    var a = new Uint8Array([1, 2, 3, 4]);
    var b = new Uint8Array([1, 2, 3, 4]);
    var c = new Uint8Array([1, 2, 5, 4]);
    var d = new Uint8Array([1, 2, 3]);
    var z = new Uint8Array(0);
    it("should return 0 if inputs have different lenghts", function () {
        expect(constant_time_1.compare(a, d)).toBe(0);
    });
    it("should return 1 if inputs are equal", function () {
        expect(constant_time_1.compare(a, b)).toBe(1);
    });
    it("should return 0 if inputs are not equal", function () {
        expect(constant_time_1.compare(a, c)).toBe(0);
    });
    it("should return 1 if given zero-length inputs ", function () {
        expect(constant_time_1.compare(z, z)).toBe(1);
    });
});
describe("constant-time.equal", function () {
    var a = new Uint8Array([1, 2, 3, 4]);
    var b = new Uint8Array([1, 2, 3, 4]);
    var c = new Uint8Array([1, 2, 5, 4]);
    var d = new Uint8Array([1, 2, 3]);
    var z = new Uint8Array(0);
    it("should return false if inputs have different lenghts", function () {
        expect(constant_time_1.equal(a, d)).toBe(false);
    });
    it("should return true if inputs are equal", function () {
        expect(constant_time_1.equal(a, b)).toBe(true);
    });
    it("should return false if inputs are not equal", function () {
        expect(constant_time_1.equal(a, c)).toBe(false);
    });
    it("should return false if given zero-length inputs ", function () {
        expect(constant_time_1.equal(z, z)).toBe(false);
    });
});
//# sourceMappingURL=constant-time.test.js.map