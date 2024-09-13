import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x27e86d6e";
/**
 * Checks if the `getLastBlockHash` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getLastBlockHash` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetLastBlockHashSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetLastBlockHashSupported(["0x..."]);
 * ```
 */
export declare function isGetLastBlockHashSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getLastBlockHash function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetLastBlockHashResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetLastBlockHashResultResult("...");
 * ```
 */
export declare function decodeGetLastBlockHashResult(result: Hex): `0x${string}`;
/**
 * Calls the "getLastBlockHash" function on the contract.
 * @param options - The options for the getLastBlockHash function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getLastBlockHash } from "thirdweb/extensions/multicall3";
 *
 * const result = await getLastBlockHash({
 *  contract,
 * });
 *
 * ```
 */
export declare function getLastBlockHash(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=getLastBlockHash.d.ts.map