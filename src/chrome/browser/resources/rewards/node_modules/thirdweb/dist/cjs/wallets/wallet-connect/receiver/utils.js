"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.validateAccountAddress = validateAccountAddress;
exports.parseEip155ChainId = parseEip155ChainId;
const address_js_1 = require("../../../utils/address.js");
/**
 * @internal
 */
function validateAccountAddress(account, address) {
    if ((0, address_js_1.checksumAddress)(account.address) !== (0, address_js_1.checksumAddress)(address)) {
        throw new Error(`Failed to validate account address (${account.address}), differs from ${address}`);
    }
}
/**
 * @internal
 */
function parseEip155ChainId(chainId) {
    const chainIdParts = chainId.split(":");
    const chainIdAsNumber = Number.parseInt(chainIdParts[1] ?? "0");
    if (chainIdParts.length !== 2 ||
        chainIdParts[0] !== "eip155" ||
        chainIdAsNumber === 0 ||
        !chainIdAsNumber) {
        throw new Error(`Invalid chainId ${chainId}, should have the format 'eip155:1'`);
    }
    return chainIdAsNumber;
}
//# sourceMappingURL=utils.js.map