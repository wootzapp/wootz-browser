import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xfec7ff72";
/**
 * Checks if the `encodeBytesOnInstall` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `encodeBytesOnInstall` method is supported.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const supported = TransferableERC20.isEncodeBytesOnInstallSupported(["0x..."]);
 * ```
 */
export declare function isEncodeBytesOnInstallSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the encodeBytesOnInstall function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.decodeEncodeBytesOnInstallResultResult("...");
 * ```
 */
export declare function decodeEncodeBytesOnInstallResult(result: Hex): `0x${string}`;
/**
 * Calls the "encodeBytesOnInstall" function on the contract.
 * @param options - The options for the encodeBytesOnInstall function.
 * @returns The parsed result of the function call.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 *
 * const result = await TransferableERC20.encodeBytesOnInstall({
 *  contract,
 * });
 *
 * ```
 */
export declare function encodeBytesOnInstall(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=encodeBytesOnInstall.d.ts.map