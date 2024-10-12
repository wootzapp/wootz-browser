import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getBlockHash" function.
 */
export type GetBlockHashParams = {
    blockNumber: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "blockNumber";
    }>;
};
export declare const FN_SELECTOR: "0xee82ac5e";
/**
 * Checks if the `getBlockHash` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getBlockHash` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetBlockHashSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetBlockHashSupported(["0x..."]);
 * ```
 */
export declare function isGetBlockHashSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getBlockHash" function.
 * @param options - The options for the getBlockHash function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeGetBlockHashParams } from "thirdweb/extensions/multicall3";
 * const result = encodeGetBlockHashParams({
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeGetBlockHashParams(options: GetBlockHashParams): `0x${string}`;
/**
 * Encodes the "getBlockHash" function into a Hex string with its parameters.
 * @param options - The options for the getBlockHash function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeGetBlockHash } from "thirdweb/extensions/multicall3";
 * const result = encodeGetBlockHash({
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeGetBlockHash(options: GetBlockHashParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getBlockHash function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetBlockHashResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetBlockHashResultResult("...");
 * ```
 */
export declare function decodeGetBlockHashResult(result: Hex): `0x${string}`;
/**
 * Calls the "getBlockHash" function on the contract.
 * @param options - The options for the getBlockHash function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getBlockHash } from "thirdweb/extensions/multicall3";
 *
 * const result = await getBlockHash({
 *  contract,
 *  blockNumber: ...,
 * });
 *
 * ```
 */
export declare function getBlockHash(options: BaseTransactionOptions<GetBlockHashParams>): Promise<`0x${string}`>;
//# sourceMappingURL=getBlockHash.d.ts.map