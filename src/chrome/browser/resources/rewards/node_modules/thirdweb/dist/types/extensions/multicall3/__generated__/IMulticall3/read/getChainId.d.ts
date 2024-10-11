import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x3408e470";
/**
 * Checks if the `getChainId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getChainId` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetChainIdSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetChainIdSupported(["0x..."]);
 * ```
 */
export declare function isGetChainIdSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getChainId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetChainIdResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetChainIdResultResult("...");
 * ```
 */
export declare function decodeGetChainIdResult(result: Hex): bigint;
/**
 * Calls the "getChainId" function on the contract.
 * @param options - The options for the getChainId function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getChainId } from "thirdweb/extensions/multicall3";
 *
 * const result = await getChainId({
 *  contract,
 * });
 *
 * ```
 */
export declare function getChainId(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getChainId.d.ts.map