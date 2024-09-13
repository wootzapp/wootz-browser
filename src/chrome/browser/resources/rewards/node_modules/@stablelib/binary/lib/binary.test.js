"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var binary_1 = require("./binary");
var int16BEVectors = [
    [0, [0, 0]],
    [1, [0, 1]],
    [255, [0, 255]],
    [-2, [255, 254]],
    [-1, [255, 255]],
    [32767, [127, 255]]
];
var int16LEVectors = int16BEVectors.map(function (v) {
    return [v[0], v[1].slice().reverse()];
});
describe("readInt16BE", function () {
    it("should read correct value", function () {
        int16BEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readInt16BE(buf)).toEqual(v[0]);
        });
    });
});
describe("readInt16LE", function () {
    it("should read correct value", function () {
        int16LEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readInt16LE(buf)).toEqual(v[0]);
        });
    });
});
var uint16BEVectors = [
    [0, [0, 0]],
    [1, [0, 1]],
    [255, [0, 255]],
    [256, [1, 0]],
    [65535, [255, 255]]
];
var uint16LEVectors = uint16BEVectors.map(function (v) {
    return [v[0], v[1].slice().reverse()];
});
describe("readUint16BE", function () {
    it("should read correct value", function () {
        uint16BEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readUint16BE(buf)).toEqual(v[0]);
        });
    });
    it("should read from correct offset", function () {
        var orig = new Uint8Array([0, 1, 2]);
        var value = binary_1.readUint16BE(orig, 1);
        expect(value).toBe(258);
    });
});
describe("writeUint16BE", function () {
    it("should write correct value", function () {
        uint16BEVectors.forEach(function (v) {
            var buf = new Uint8Array(2);
            var good = new Uint8Array(v[1]);
            var value = v[0];
            expect(binary_1.writeUint16BE(value, buf)).toEqual(good);
        });
    });
    it("should allocate new array if not given", function () {
        expect(Object.prototype.toString.call(binary_1.writeUint16BE(1))).toBe("[object Uint8Array]");
    });
    it("should write to correct offset", function () {
        var orig = new Uint8Array([0, 1, 2]);
        var result = binary_1.writeUint16BE(258, new Uint8Array(3), 1);
        expect(result).toEqual(orig);
    });
});
describe("readUint16LE", function () {
    it("should read correct value", function () {
        uint16LEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readUint16LE(buf)).toEqual(v[0]);
        });
    });
    it("should read from correct offset", function () {
        var orig = new Uint8Array([0, 1, 2]);
        var value = binary_1.readUint16LE(orig, 1);
        expect(value).toBe(513);
    });
});
describe("writeUint16LE", function () {
    it("should write correct value", function () {
        uint16LEVectors.forEach(function (v) {
            var buf = new Uint8Array(2);
            var good = new Uint8Array(v[1]);
            var value = v[0];
            expect(binary_1.writeUint16LE(value, buf)).toEqual(good);
        });
    });
    it("should allocate new array if not given", function () {
        expect(Object.prototype.toString.call(binary_1.writeUint16LE(1))).toBe("[object Uint8Array]");
    });
    it("should write to correct offset", function () {
        var orig = new Uint8Array([0, 1, 2]);
        var result = binary_1.writeUint16LE(513, new Uint8Array(3), 1);
        expect(result).toEqual(orig);
    });
});
var int32BEVectors = [
    [0, [0, 0, 0, 0]],
    [1, [0, 0, 0, 1]],
    [255, [0, 0, 0, 255]],
    [-2, [255, 255, 255, 254]],
    [-1, [255, 255, 255, 255]],
    [32767, [0, 0, 127, 255]],
    [2147483647, [127, 255, 255, 255]],
    [-2147483647, [128, 0, 0, 1]]
];
var int32LEVectors = int32BEVectors.map(function (v) {
    return [v[0], v[1].slice().reverse()];
});
describe("readInt32BE", function () {
    it("should read correct value", function () {
        int32BEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readInt32BE(buf)).toEqual(v[0]);
        });
    });
});
describe("readInt32LE", function () {
    it("should read correct value", function () {
        int32LEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readInt32LE(buf)).toEqual(v[0]);
        });
    });
});
var uint32BEVectors = [
    [0, [0, 0, 0, 0]],
    [1, [0, 0, 0, 1]],
    [255, [0, 0, 0, 255]],
    [256, [0, 0, 1, 0]],
    [65535, [0, 0, 255, 255]],
    [16777215, [0, 255, 255, 255]],
    [2147483647, [127, 255, 255, 255]],
    [4294901660, [255, 254, 255, 156]],
    [4294967295, [255, 255, 255, 255]],
];
var uint32LEVectors = uint32BEVectors.map(function (v) {
    return [v[0], v[1].slice().reverse()];
});
describe("readUint32BE", function () {
    it("should read correct value", function () {
        uint32BEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readUint32BE(buf)).toEqual(v[0]);
        });
    });
    it("should read from correct offset", function () {
        var orig = new Uint8Array([0, 1, 2, 3, 4]);
        var value = binary_1.readUint32BE(orig, 1);
        expect(value).toBe(16909060);
    });
});
describe("writeUint32BE", function () {
    it("should write correct value", function () {
        uint32BEVectors.forEach(function (v) {
            var buf = new Uint8Array(4);
            var good = new Uint8Array(v[1]);
            var value = v[0];
            expect(binary_1.writeUint32BE(value, buf)).toEqual(good);
        });
    });
    it("should allocate new array if not given", function () {
        expect(Object.prototype.toString.call(binary_1.writeUint32BE(1))).toBe("[object Uint8Array]");
    });
    it("should write to correct offset", function () {
        var orig = new Uint8Array([0, 1, 2, 3, 4]);
        var result = binary_1.writeUint32BE(16909060, new Uint8Array(5), 1);
        expect(result).toEqual(orig);
    });
});
describe("readUint32LE", function () {
    it("should read correct value", function () {
        uint32LEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readUint32LE(buf)).toEqual(v[0]);
        });
    });
    it("should read from correct offset", function () {
        var orig = new Uint8Array([0, 1, 2, 3, 4]);
        var value = binary_1.readUint32LE(orig, 1);
        expect(value).toBe(67305985);
    });
});
describe("writeUint32LE", function () {
    it("should write correct value", function () {
        uint32LEVectors.forEach(function (v) {
            var buf = new Uint8Array(4);
            var good = new Uint8Array(v[1]);
            var value = v[0];
            expect(binary_1.writeUint32LE(value, buf)).toEqual(good);
        });
    });
    it("should allocate new array if not given", function () {
        expect(Object.prototype.toString.call(binary_1.writeUint32LE(1))).toBe("[object Uint8Array]");
    });
    it("should write to correct offset", function () {
        var orig = new Uint8Array([0, 1, 2, 3, 4]);
        var result = binary_1.writeUint32LE(67305985, new Uint8Array(5), 1);
        expect(result).toEqual(orig);
    });
});
var int64BEVectors = [
    [0, [0, 0, 0, 0, 0, 0, 0, 0]],
    [1, [0, 0, 0, 0, 0, 0, 0, 1]],
    [255, [0, 0, 0, 0, 0, 0, 0, 255]],
    [256, [0, 0, 0, 0, 0, 0, 1, 0]],
    [65535, [0, 0, 0, 0, 0, 0, 255, 255]],
    [16777215, [0, 0, 0, 0, 0, 255, 255, 255]],
    [2147483647, [0, 0, 0, 0, 127, 255, 255, 255]],
    [4294901660, [0, 0, 0, 0, 255, 254, 255, 156]],
    [4294967295, [0, 0, 0, 0, 255, 255, 255, 255]],
    [2146252406, [0, 0, 0, 0, 127, 237, 54, 118]],
    [2147483648, [0, 0, 0, 0, 128, 0, 0, 0]],
    [4294967296, [0, 0, 0, 1, 0, 0, 0, 0]],
    [4295450643, [0, 0, 0, 1, 0, 7, 96, 19]],
    [8589934592, [0, 0, 0, 2, 0, 0, 0, 0]],
    [35184372088610, [0, 0, 31, 255, 255, 255, 255, 34]],
    [-35184372088610, [255, 255, 224, 0, 0, 0, 0, 222]],
    [140737488355326, [0, 0, 127, 255, 255, 255, 255, 254]],
    [9007199254740991, [0, 31, 255, 255, 255, 255, 255, 255]]
];
var int64LEVectors = int64BEVectors.map(function (v) {
    return [v[0], v[1].slice().reverse()];
});
describe("readInt64BE", function () {
    it("should read correct value", function () {
        int64BEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readInt64BE(buf)).toEqual(v[0]);
        });
    });
});
describe("readInt64LE", function () {
    it("should read correct value", function () {
        int64LEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readInt64LE(buf)).toEqual(v[0]);
        });
    });
});
var uint64BEVectors = [
    [0, [0, 0, 0, 0, 0, 0, 0, 0]],
    [1, [0, 0, 0, 0, 0, 0, 0, 1]],
    [255, [0, 0, 0, 0, 0, 0, 0, 255]],
    [256, [0, 0, 0, 0, 0, 0, 1, 0]],
    [65535, [0, 0, 0, 0, 0, 0, 255, 255]],
    [16777215, [0, 0, 0, 0, 0, 255, 255, 255]],
    [2147483647, [0, 0, 0, 0, 127, 255, 255, 255]],
    [4294901660, [0, 0, 0, 0, 255, 254, 255, 156]],
    [4294967295, [0, 0, 0, 0, 255, 255, 255, 255]],
    [2146252406, [0, 0, 0, 0, 127, 237, 54, 118]],
    [2147483648, [0, 0, 0, 0, 128, 0, 0, 0]],
    [4294967296, [0, 0, 0, 1, 0, 0, 0, 0]],
    [4295450643, [0, 0, 0, 1, 0, 7, 96, 19]],
    [8589934592, [0, 0, 0, 2, 0, 0, 0, 0]],
    [35184372088610, [0, 0, 31, 255, 255, 255, 255, 34]],
    [281474976710655, [0, 0, 255, 255, 255, 255, 255, 255]],
    [140737488355326, [0, 0, 127, 255, 255, 255, 255, 254]],
    [9007199254740991, [0, 31, 255, 255, 255, 255, 255, 255]]
];
var uint64LEVectors = uint64BEVectors.map(function (v) {
    return [v[0], v[1].slice().reverse()];
});
describe("readUint64BE", function () {
    it("should read correct value", function () {
        uint64BEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readUint64BE(buf)).toEqual(v[0]);
        });
    });
    it("should read from correct offset", function () {
        var orig = new Uint8Array([0, 0, 1, 2, 3, 4, 5, 6, 7]);
        var value = binary_1.readUint64BE(orig, 1);
        expect(value).toBe(283686952306183);
    });
});
describe("writeUint64BE", function () {
    it("should write correct value", function () {
        uint64BEVectors.forEach(function (v) {
            var buf = new Uint8Array(8);
            var good = new Uint8Array(v[1]);
            var value = v[0];
            expect(binary_1.writeUint64BE(value, buf)).toEqual(good);
        });
    });
    it("should allocate new array if not given", function () {
        expect(Object.prototype.toString.call(binary_1.writeUint32BE(1))).toBe("[object Uint8Array]");
    });
    it("should write to correct offset", function () {
        var orig = new Uint8Array([0, 0, 1, 2, 3, 4, 5, 6, 7]);
        var result = binary_1.writeUint64BE(283686952306183, new Uint8Array(9), 1);
        expect(result).toEqual(orig);
    });
});
describe("readUint64LE", function () {
    it("should read correct value", function () {
        uint64LEVectors.forEach(function (v) {
            var buf = new Uint8Array(v[1]);
            expect(binary_1.readUint64LE(buf)).toEqual(v[0]);
        });
    });
    it("should read from correct offset", function () {
        var orig = new Uint8Array([0, 7, 6, 5, 4, 3, 2, 1, 0]);
        var value = binary_1.readUint64LE(orig, 1);
        expect(value).toBe(283686952306183);
    });
});
describe("writeUint64LE", function () {
    it("should write correct value", function () {
        uint64LEVectors.forEach(function (v) {
            var buf = new Uint8Array(8);
            var good = new Uint8Array(v[1]);
            var value = v[0];
            expect(binary_1.writeUint64LE(value, buf)).toEqual(good);
        });
    });
    it("should allocate new array if not given", function () {
        expect(Object.prototype.toString.call(binary_1.writeUint32LE(1))).toBe("[object Uint8Array]");
    });
    it("should write to correct offset", function () {
        var orig = new Uint8Array([0, 7, 6, 5, 4, 3, 2, 1, 0]);
        var result = binary_1.writeUint64LE(283686952306183, new Uint8Array(9), 1);
        expect(result).toEqual(orig);
    });
});
describe("readUintLE/writeUintLE", function () {
    it("should write and read back 32-bit value", function () {
        var orig = 1234567891;
        var offset = 2;
        var wrote = binary_1.writeUintLE(32, orig, new Uint8Array(6), offset);
        expect(wrote).toEqual(binary_1.writeUint32LE(orig, new Uint8Array(6), offset));
        expect(binary_1.readUint32LE(wrote, offset)).toBe(orig);
        var read = binary_1.readUintLE(32, wrote, offset);
        expect(read).toBe(orig);
    });
    it("should write and read back 48-bit value", function () {
        var orig = Math.pow(2, 48) - 3;
        var offset = 2;
        var wrote = binary_1.writeUintLE(48, orig, new Uint8Array(8), offset);
        var read = binary_1.readUintLE(48, wrote, offset);
        expect(read).toBe(orig);
    });
    it("write throws if given non-integer numbers", function () {
        expect(function () { return binary_1.writeUintLE(56, Math.pow(2, 54)); })
            .toThrowError("writeUintLE value must be an integer");
    });
});
describe("readUintBE/writeUintBE", function () {
    it("should write and read back 32-bit value", function () {
        var orig = 1234567891;
        var offset = 2;
        var wrote = binary_1.writeUintBE(32, orig, new Uint8Array(6), offset);
        expect(wrote).toEqual(binary_1.writeUint32BE(orig, new Uint8Array(6), offset));
        expect(binary_1.readUint32BE(wrote, offset)).toBe(orig);
        var read = binary_1.readUintBE(32, wrote, offset);
        expect(read).toBe(orig);
    });
    it("should write and read back 48-bit value", function () {
        var orig = Math.pow(2, 48) - 3;
        var offset = 2;
        var wrote = binary_1.writeUintBE(48, orig, new Uint8Array(8), offset);
        var read = binary_1.readUintBE(48, wrote, offset);
        expect(read).toBe(orig);
    });
    it("write throws if given non-integer numbers", function () {
        expect(function () { return binary_1.writeUintBE(56, Math.pow(2, 54)); })
            .toThrowError("writeUintBE value must be an integer");
    });
});
describe("readFloat32BE/writeFloat32BE", function () {
    it("should write and read back value", function () {
        var orig = 123456.1;
        var wrote = binary_1.writeFloat32BE(orig);
        expect(wrote.length).toBe(4);
        var read = binary_1.readFloat32BE(wrote);
        expect(Math.abs(orig - read)).toBeLessThan(0.1);
    });
});
describe("readFloat32LE/writeFloat32LE", function () {
    it("should write and read back value", function () {
        var orig = 123456.1;
        var wrote = binary_1.writeFloat32LE(orig);
        expect(wrote.length).toBe(4);
        var read = binary_1.readFloat32LE(wrote);
        expect(Math.abs(orig - read)).toBeLessThan(0.1);
    });
});
describe("readFloat64BE/writeFloat64BE", function () {
    it("should write and read back value", function () {
        var orig = 123456891.123;
        var wrote = binary_1.writeFloat64BE(orig);
        expect(wrote.length).toBe(8);
        var read = binary_1.readFloat64BE(wrote);
        expect(read).toBe(orig);
    });
});
describe("readFloat64LE/writeFloat64LE", function () {
    it("should write and read back value", function () {
        var orig = 123456891.123;
        var wrote = binary_1.writeFloat64LE(orig);
        expect(wrote.length).toBe(8);
        var read = binary_1.readFloat64LE(wrote);
        expect(read).toBe(orig);
    });
});
//# sourceMappingURL=binary.test.js.map