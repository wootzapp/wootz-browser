import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "tokensOfOwnerIn" function.
 */
export type TokensOfOwnerInParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
    start: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "start";
    }>;
    stop: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "stop";
    }>;
};
export declare const FN_SELECTOR: "0x99a2557a";
/**
 * Checks if the `tokensOfOwnerIn` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tokensOfOwnerIn` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isTokensOfOwnerInSupported } from "thirdweb/extensions/erc721";
 * const supported = isTokensOfOwnerInSupported(["0x..."]);
 * ```
 */
export declare function isTokensOfOwnerInSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tokensOfOwnerIn" function.
 * @param options - The options for the tokensOfOwnerIn function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokensOfOwnerInParams } from "thirdweb/extensions/erc721";
 * const result = encodeTokensOfOwnerInParams({
 *  owner: ...,
 *  start: ...,
 *  stop: ...,
 * });
 * ```
 */
export declare function encodeTokensOfOwnerInParams(options: TokensOfOwnerInParams): `0x${string}`;
/**
 * Encodes the "tokensOfOwnerIn" function into a Hex string with its parameters.
 * @param options - The options for the tokensOfOwnerIn function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokensOfOwnerIn } from "thirdweb/extensions/erc721";
 * const result = encodeTokensOfOwnerIn({
 *  owner: ...,
 *  start: ...,
 *  stop: ...,
 * });
 * ```
 */
export declare function encodeTokensOfOwnerIn(options: TokensOfOwnerInParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the tokensOfOwnerIn function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeTokensOfOwnerInResult } from "thirdweb/extensions/erc721";
 * const result = decodeTokensOfOwnerInResultResult("...");
 * ```
 */
export declare function decodeTokensOfOwnerInResult(result: Hex): readonly bigint[];
/**
 * Calls the "tokensOfOwnerIn" function on the contract.
 * @param options - The options for the tokensOfOwnerIn function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { tokensOfOwnerIn } from "thirdweb/extensions/erc721";
 *
 * const result = await tokensOfOwnerIn({
 *  contract,
 *  owner: ...,
 *  start: ...,
 *  stop: ...,
 * });
 *
 * ```
 */
export declare function tokensOfOwnerIn(options: BaseTransactionOptions<TokensOfOwnerInParams>): Promise<readonly bigint[]>;
//# sourceMappingURL=tokensOfOwnerIn.d.ts.map