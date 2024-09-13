"use strict";
// Copyright (C) 2016 Dmitry Chestnykh
// MIT License. See LICENSE file for details.
Object.defineProperty(exports, "__esModule", { value: true });
function concat() {
    // Calculate sum of lengths of all arrays.
    var totalLength = 0;
    for (var i = 0; i < arguments.length; i++) {
        totalLength += arguments[i].length;
    }
    // Allocate new array of calculated length.
    var result = new Uint8Array(totalLength);
    // Copy all arrays into result.
    var offset = 0;
    for (var i = 0; i < arguments.length; i++) {
        var arg = arguments[i];
        result.set(arg, offset);
        offset += arg.length;
    }
    return result;
}
exports.concat = concat;
//# sourceMappingURL=bytes.js.map