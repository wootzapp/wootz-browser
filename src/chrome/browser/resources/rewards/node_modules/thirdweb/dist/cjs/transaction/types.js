"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isBaseTransactionOptions = isBaseTransactionOptions;
const type_guards_js_1 = require("../utils/type-guards.js");
// type-guard
/**
 * Checks if the given value is of type TxOpts.
 * @param value - The value to check.
 * @returns True if the value is of type TxOpts, false otherwise.
 * @internal
 */
function isBaseTransactionOptions(value) {
    return ((0, type_guards_js_1.isObjectWithKeys)(value, ["__contract"]) &&
        (0, type_guards_js_1.isObjectWithKeys)(value.__contract, ["address", "chain"]) &&
        typeof value.__contract.address === "string");
}
//# sourceMappingURL=types.js.map