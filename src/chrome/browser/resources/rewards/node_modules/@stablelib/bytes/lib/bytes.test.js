"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
var bytes_1 = require("./bytes");
describe("concat", function () {
    it("should concatenate byte arrays", function () {
        var arrays = [
            new Uint8Array([1, 2, 3]),
            new Uint8Array([4]),
            new Uint8Array(0),
            new Uint8Array([5, 6, 7, 8, 9, 10])
        ];
        var good = new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);
        expect(bytes_1.concat.apply(null, arrays)).toEqual(good);
    });
});
//# sourceMappingURL=bytes.test.js.map