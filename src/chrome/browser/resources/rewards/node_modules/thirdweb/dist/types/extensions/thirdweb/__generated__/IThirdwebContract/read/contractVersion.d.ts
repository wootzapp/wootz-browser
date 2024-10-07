import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xa0a8e460";
/**
 * Checks if the `contractVersion` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `contractVersion` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isContractVersionSupported } from "thirdweb/extensions/thirdweb";
 * const supported = isContractVersionSupported(["0x..."]);
 * ```
 */
export declare function isContractVersionSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the contractVersion function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeContractVersionResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeContractVersionResultResult("...");
 * ```
 */
export declare function decodeContractVersionResult(result: Hex): number;
/**
 * Calls the "contractVersion" function on the contract.
 * @param options - The options for the contractVersion function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { contractVersion } from "thirdweb/extensions/thirdweb";
 *
 * const result = await contractVersion({
 *  contract,
 * });
 *
 * ```
 */
export declare function contractVersion(options: BaseTransactionOptions): Promise<number>;
//# sourceMappingURL=contractVersion.d.ts.map