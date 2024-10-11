import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "baseURIIndices" function.
 */
export type BaseURIIndicesParams = {
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "index";
    }>;
};
export declare const FN_SELECTOR: "0xd860483f";
/**
 * Checks if the `baseURIIndices` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `baseURIIndices` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isBaseURIIndicesSupported } from "thirdweb/extensions/erc721";
 * const supported = isBaseURIIndicesSupported(["0x..."]);
 * ```
 */
export declare function isBaseURIIndicesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "baseURIIndices" function.
 * @param options - The options for the baseURIIndices function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeBaseURIIndicesParams } from "thirdweb/extensions/erc721";
 * const result = encodeBaseURIIndicesParams({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeBaseURIIndicesParams(options: BaseURIIndicesParams): `0x${string}`;
/**
 * Encodes the "baseURIIndices" function into a Hex string with its parameters.
 * @param options - The options for the baseURIIndices function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeBaseURIIndices } from "thirdweb/extensions/erc721";
 * const result = encodeBaseURIIndices({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeBaseURIIndices(options: BaseURIIndicesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the baseURIIndices function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeBaseURIIndicesResult } from "thirdweb/extensions/erc721";
 * const result = decodeBaseURIIndicesResultResult("...");
 * ```
 */
export declare function decodeBaseURIIndicesResult(result: Hex): bigint;
/**
 * Calls the "baseURIIndices" function on the contract.
 * @param options - The options for the baseURIIndices function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { baseURIIndices } from "thirdweb/extensions/erc721";
 *
 * const result = await baseURIIndices({
 *  contract,
 *  index: ...,
 * });
 *
 * ```
 */
export declare function baseURIIndices(options: BaseTransactionOptions<BaseURIIndicesParams>): Promise<bigint>;
//# sourceMappingURL=baseURIIndices.d.ts.map