import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x00dbe109";
/**
 * Checks if the `payeeCount` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `payeeCount` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isPayeeCountSupported } from "thirdweb/extensions/split";
 * const supported = isPayeeCountSupported(["0x..."]);
 * ```
 */
export declare function isPayeeCountSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the payeeCount function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension SPLIT
 * @example
 * ```ts
 * import { decodePayeeCountResult } from "thirdweb/extensions/split";
 * const result = decodePayeeCountResultResult("...");
 * ```
 */
export declare function decodePayeeCountResult(result: Hex): bigint;
/**
 * Calls the "payeeCount" function on the contract.
 * @param options - The options for the payeeCount function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { payeeCount } from "thirdweb/extensions/split";
 *
 * const result = await payeeCount({
 *  contract,
 * });
 *
 * ```
 */
export declare function payeeCount(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=payeeCount.d.ts.map