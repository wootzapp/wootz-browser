import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x2751c4fd";
/**
 * Checks if the `rentedUnits` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `rentedUnits` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRentedUnitsSupported } from "thirdweb/extensions/farcaster";
 * const supported = isRentedUnitsSupported(["0x..."]);
 * ```
 */
export declare function isRentedUnitsSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the rentedUnits function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeRentedUnitsResult } from "thirdweb/extensions/farcaster";
 * const result = decodeRentedUnitsResultResult("...");
 * ```
 */
export declare function decodeRentedUnitsResult(result: Hex): bigint;
/**
 * Calls the "rentedUnits" function on the contract.
 * @param options - The options for the rentedUnits function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { rentedUnits } from "thirdweb/extensions/farcaster";
 *
 * const result = await rentedUnits({
 *  contract,
 * });
 *
 * ```
 */
export declare function rentedUnits(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=rentedUnits.d.ts.map