"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.ZERO_ADDRESS = exports.NATIVE_TOKEN_ADDRESS = void 0;
exports.isNativeTokenAddress = isNativeTokenAddress;
/**
 * The address of the native token.
 */
exports.NATIVE_TOKEN_ADDRESS = "0xeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
/**
 * @internal
 */
function isNativeTokenAddress(address) {
    return address.toLowerCase() === exports.NATIVE_TOKEN_ADDRESS;
}
/**
 * The zero address in Ethereum, represented as a hexadecimal string.
 */
exports.ZERO_ADDRESS = "0x0000000000000000000000000000000000000000";
//# sourceMappingURL=addresses.js.map