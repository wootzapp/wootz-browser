import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "tokensOfOwner" function.
 */
export type TokensOfOwnerParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
};
export declare const FN_SELECTOR: "0x8462151c";
/**
 * Checks if the `tokensOfOwner` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tokensOfOwner` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isTokensOfOwnerSupported } from "thirdweb/extensions/erc721";
 * const supported = isTokensOfOwnerSupported(["0x..."]);
 * ```
 */
export declare function isTokensOfOwnerSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tokensOfOwner" function.
 * @param options - The options for the tokensOfOwner function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokensOfOwnerParams } from "thirdweb/extensions/erc721";
 * const result = encodeTokensOfOwnerParams({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeTokensOfOwnerParams(options: TokensOfOwnerParams): `0x${string}`;
/**
 * Encodes the "tokensOfOwner" function into a Hex string with its parameters.
 * @param options - The options for the tokensOfOwner function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokensOfOwner } from "thirdweb/extensions/erc721";
 * const result = encodeTokensOfOwner({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeTokensOfOwner(options: TokensOfOwnerParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the tokensOfOwner function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeTokensOfOwnerResult } from "thirdweb/extensions/erc721";
 * const result = decodeTokensOfOwnerResultResult("...");
 * ```
 */
export declare function decodeTokensOfOwnerResult(result: Hex): readonly bigint[];
/**
 * Calls the "tokensOfOwner" function on the contract.
 * @param options - The options for the tokensOfOwner function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { tokensOfOwner } from "thirdweb/extensions/erc721";
 *
 * const result = await tokensOfOwner({
 *  contract,
 *  owner: ...,
 * });
 *
 * ```
 */
export declare function tokensOfOwner(options: BaseTransactionOptions<TokensOfOwnerParams>): Promise<readonly bigint[]>;
//# sourceMappingURL=tokensOfOwner.d.ts.map