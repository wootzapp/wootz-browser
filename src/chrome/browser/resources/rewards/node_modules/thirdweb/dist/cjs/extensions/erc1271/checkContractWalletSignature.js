"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.checkContractWalletSignature = checkContractWalletSignature;
const viem_1 = require("viem");
const hex_js_1 = require("../../utils/encoding/hex.js");
const isValidSignature_js_1 = require("./__generated__/isValidSignature/read/isValidSignature.js");
const MAGIC_VALUE = "0x1626ba7e";
/**
 * Checks if a contract wallet signature is valid.
 * @deprecated Use `verifySignature` instead
 * @param options - The options for the checkContractWalletSignature function.
 * @param options.contract - The contract to check the signature against.
 * @param options.message - The message to check the signature against.
 * @param options.signature - The signature to check.
 * @extension ERC1271
 * @example
 * ```ts
 * import { checkContractWalletSignature } from "thirdweb/extensions/erc1271";
 * const isValid = await checkContractWalletSignature({
 *  contract: myContract,
 *  message: "hello world",
 *  signature: "0x...",
 * });
 * ```
 * @returns A promise that resolves with a boolean indicating if the signature is valid.
 */
async function checkContractWalletSignature(options) {
    if (!(0, hex_js_1.isHex)(options.signature)) {
        throw new Error("The signature must be a valid hex string.");
    }
    const result = await (0, isValidSignature_js_1.isValidSignature)({
        contract: options.contract,
        hash: (0, viem_1.hashMessage)(options.message),
        signature: options.signature,
    });
    return result === MAGIC_VALUE;
}
//# sourceMappingURL=checkContractWalletSignature.js.map