import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "tokenByIndex" function.
 */
export type TokenByIndexParams = {
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_index";
    }>;
};
export declare const FN_SELECTOR: "0x4f6ccce7";
/**
 * Checks if the `tokenByIndex` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tokenByIndex` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isTokenByIndexSupported } from "thirdweb/extensions/erc721";
 * const supported = isTokenByIndexSupported(["0x..."]);
 * ```
 */
export declare function isTokenByIndexSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tokenByIndex" function.
 * @param options - The options for the tokenByIndex function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokenByIndexParams } from "thirdweb/extensions/erc721";
 * const result = encodeTokenByIndexParams({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeTokenByIndexParams(options: TokenByIndexParams): `0x${string}`;
/**
 * Encodes the "tokenByIndex" function into a Hex string with its parameters.
 * @param options - The options for the tokenByIndex function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokenByIndex } from "thirdweb/extensions/erc721";
 * const result = encodeTokenByIndex({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeTokenByIndex(options: TokenByIndexParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the tokenByIndex function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeTokenByIndexResult } from "thirdweb/extensions/erc721";
 * const result = decodeTokenByIndexResultResult("...");
 * ```
 */
export declare function decodeTokenByIndexResult(result: Hex): bigint;
/**
 * Calls the "tokenByIndex" function on the contract.
 * @param options - The options for the tokenByIndex function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { tokenByIndex } from "thirdweb/extensions/erc721";
 *
 * const result = await tokenByIndex({
 *  contract,
 *  index: ...,
 * });
 *
 * ```
 */
export declare function tokenByIndex(options: BaseTransactionOptions<TokenByIndexParams>): Promise<bigint>;
//# sourceMappingURL=tokenByIndex.d.ts.map