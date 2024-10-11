import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x06517a29";
/**
 * Checks if the `maxUnits` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `maxUnits` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isMaxUnitsSupported } from "thirdweb/extensions/farcaster";
 * const supported = isMaxUnitsSupported(["0x..."]);
 * ```
 */
export declare function isMaxUnitsSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the maxUnits function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeMaxUnitsResult } from "thirdweb/extensions/farcaster";
 * const result = decodeMaxUnitsResultResult("...");
 * ```
 */
export declare function decodeMaxUnitsResult(result: Hex): bigint;
/**
 * Calls the "maxUnits" function on the contract.
 * @param options - The options for the maxUnits function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { maxUnits } from "thirdweb/extensions/farcaster";
 *
 * const result = await maxUnits({
 *  contract,
 * });
 *
 * ```
 */
export declare function maxUnits(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=maxUnits.d.ts.map