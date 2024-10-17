"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.tenYearsFromNow = tenYearsFromNow;
exports.dateToSeconds = dateToSeconds;
const bigint_js_1 = require("./bigint.js");
/**
 * @internal
 */
function tenYearsFromNow() {
    return new Date(Date.now() + 1000 * 60 * 60 * 24 * 365 * 10); // 10 years
}
/**
 * @internal
 */
function dateToSeconds(date) {
    return (0, bigint_js_1.toBigInt)(Math.floor(date.getTime() / 1000));
}
//# sourceMappingURL=date.js.map