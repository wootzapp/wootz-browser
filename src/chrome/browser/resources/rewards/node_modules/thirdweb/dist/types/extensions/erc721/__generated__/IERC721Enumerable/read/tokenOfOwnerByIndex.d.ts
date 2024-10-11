import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "tokenOfOwnerByIndex" function.
 */
export type TokenOfOwnerByIndexParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_owner";
    }>;
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_index";
    }>;
};
export declare const FN_SELECTOR: "0x2f745c59";
/**
 * Checks if the `tokenOfOwnerByIndex` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tokenOfOwnerByIndex` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isTokenOfOwnerByIndexSupported } from "thirdweb/extensions/erc721";
 * const supported = isTokenOfOwnerByIndexSupported(["0x..."]);
 * ```
 */
export declare function isTokenOfOwnerByIndexSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tokenOfOwnerByIndex" function.
 * @param options - The options for the tokenOfOwnerByIndex function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokenOfOwnerByIndexParams } from "thirdweb/extensions/erc721";
 * const result = encodeTokenOfOwnerByIndexParams({
 *  owner: ...,
 *  index: ...,
 * });
 * ```
 */
export declare function encodeTokenOfOwnerByIndexParams(options: TokenOfOwnerByIndexParams): `0x${string}`;
/**
 * Encodes the "tokenOfOwnerByIndex" function into a Hex string with its parameters.
 * @param options - The options for the tokenOfOwnerByIndex function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokenOfOwnerByIndex } from "thirdweb/extensions/erc721";
 * const result = encodeTokenOfOwnerByIndex({
 *  owner: ...,
 *  index: ...,
 * });
 * ```
 */
export declare function encodeTokenOfOwnerByIndex(options: TokenOfOwnerByIndexParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the tokenOfOwnerByIndex function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeTokenOfOwnerByIndexResult } from "thirdweb/extensions/erc721";
 * const result = decodeTokenOfOwnerByIndexResultResult("...");
 * ```
 */
export declare function decodeTokenOfOwnerByIndexResult(result: Hex): bigint;
/**
 * Calls the "tokenOfOwnerByIndex" function on the contract.
 * @param options - The options for the tokenOfOwnerByIndex function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { tokenOfOwnerByIndex } from "thirdweb/extensions/erc721";
 *
 * const result = await tokenOfOwnerByIndex({
 *  contract,
 *  owner: ...,
 *  index: ...,
 * });
 *
 * ```
 */
export declare function tokenOfOwnerByIndex(options: BaseTransactionOptions<TokenOfOwnerByIndexParams>): Promise<bigint>;
//# sourceMappingURL=tokenOfOwnerByIndex.d.ts.map