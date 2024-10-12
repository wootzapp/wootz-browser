"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var int_1 = require("./int");
describe("int.mul", function () {
    it("should overflow", function () {
        var float = 0xffffffff * 0x7fffffff;
        var int = int_1.mul(0xffffffff, 0x7fffffff);
        expect(int).toBeLessThan(float);
    });
    it("should return correct result", function () {
        expect(int_1.mul(0x7fffffff, 0x5ffffff5)).toBe(0x2000000b);
    });
    it("should be commutative", function () {
        expect(int_1.mul(0x7fffffff, 0x5ffffff5))
            .toBe(int_1.mul(0x5ffffff5, 0x7fffffff));
    });
});
describe("int.add", function () {
    it("should overflow", function () {
        var float = 0xffffffff + 0x7fffffff;
        var int = int_1.add(0xffffffff, 0x7fffffff);
        expect(int).toBeLessThan(float);
    });
    it("should return correct result", function () {
        expect(int_1.add(0xffffffff, 1)).toBe(0);
        expect(int_1.add(2, 0xffffffff)).toBe(1);
    });
    it("should be commutative", function () {
        expect(int_1.add(0x7fffffff, 0x5ffffff5))
            .toBe(int_1.add(0x5ffffff5, 0x7fffffff));
    });
});
describe("int.sub", function () {
    it("should overflow", function () {
        var float = 0xffffffff + 0x7fffffff;
        var int = int_1.sub(0x7fffffff, 0xffffffff);
        expect(int).toBeLessThan(float);
    });
    it("should return correct result", function () {
        expect(int_1.sub(1, 0xffffffff)).toBe(2);
        expect(int_1.sub(2, 0xffffffff)).toBe(3);
    });
});
//# sourceMappingURL=int.test.js.map