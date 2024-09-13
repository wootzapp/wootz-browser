import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getBatchIdAtIndex" function.
 */
export type GetBatchIdAtIndexParams = {
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_index";
    }>;
};
export declare const FN_SELECTOR: "0x2419f51b";
/**
 * Checks if the `getBatchIdAtIndex` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getBatchIdAtIndex` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isGetBatchIdAtIndexSupported } from "thirdweb/extensions/erc721";
 * const supported = isGetBatchIdAtIndexSupported(["0x..."]);
 * ```
 */
export declare function isGetBatchIdAtIndexSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getBatchIdAtIndex" function.
 * @param options - The options for the getBatchIdAtIndex function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeGetBatchIdAtIndexParams } from "thirdweb/extensions/erc721";
 * const result = encodeGetBatchIdAtIndexParams({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeGetBatchIdAtIndexParams(options: GetBatchIdAtIndexParams): `0x${string}`;
/**
 * Encodes the "getBatchIdAtIndex" function into a Hex string with its parameters.
 * @param options - The options for the getBatchIdAtIndex function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeGetBatchIdAtIndex } from "thirdweb/extensions/erc721";
 * const result = encodeGetBatchIdAtIndex({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeGetBatchIdAtIndex(options: GetBatchIdAtIndexParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getBatchIdAtIndex function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeGetBatchIdAtIndexResult } from "thirdweb/extensions/erc721";
 * const result = decodeGetBatchIdAtIndexResultResult("...");
 * ```
 */
export declare function decodeGetBatchIdAtIndexResult(result: Hex): bigint;
/**
 * Calls the "getBatchIdAtIndex" function on the contract.
 * @param options - The options for the getBatchIdAtIndex function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { getBatchIdAtIndex } from "thirdweb/extensions/erc721";
 *
 * const result = await getBatchIdAtIndex({
 *  contract,
 *  index: ...,
 * });
 *
 * ```
 */
export declare function getBatchIdAtIndex(options: BaseTransactionOptions<GetBatchIdAtIndexParams>): Promise<bigint>;
//# sourceMappingURL=getBatchIdAtIndex.d.ts.map