"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var wipe_1 = require("./wipe");
describe("wipe", function () {
    it("should wipe bytes", function () {
        var a = new Uint8Array([1, 2, 3, 4]);
        wipe_1.wipe(a);
        expect(a).toEqual(new Uint8Array(4));
    });
});
//# sourceMappingURL=wipe.test.js.map