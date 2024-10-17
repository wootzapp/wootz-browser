import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xeb08ab28";
/**
 * Checks if the `idCounter` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `idCounter` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isIdCounterSupported } from "thirdweb/extensions/farcaster";
 * const supported = isIdCounterSupported(["0x..."]);
 * ```
 */
export declare function isIdCounterSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the idCounter function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeIdCounterResult } from "thirdweb/extensions/farcaster";
 * const result = decodeIdCounterResultResult("...");
 * ```
 */
export declare function decodeIdCounterResult(result: Hex): bigint;
/**
 * Calls the "idCounter" function on the contract.
 * @param options - The options for the idCounter function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { idCounter } from "thirdweb/extensions/farcaster";
 *
 * const result = await idCounter({
 *  contract,
 * });
 *
 * ```
 */
export declare function idCounter(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=idCounter.d.ts.map