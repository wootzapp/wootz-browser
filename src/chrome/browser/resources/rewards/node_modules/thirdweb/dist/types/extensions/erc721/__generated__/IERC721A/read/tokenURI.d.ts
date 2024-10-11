import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "tokenURI" function.
 */
export type TokenURIParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
};
export declare const FN_SELECTOR: "0xc87b56dd";
/**
 * Checks if the `tokenURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tokenURI` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isTokenURISupported } from "thirdweb/extensions/erc721";
 * const supported = isTokenURISupported(["0x..."]);
 * ```
 */
export declare function isTokenURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tokenURI" function.
 * @param options - The options for the tokenURI function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokenURIParams } from "thirdweb/extensions/erc721";
 * const result = encodeTokenURIParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeTokenURIParams(options: TokenURIParams): `0x${string}`;
/**
 * Encodes the "tokenURI" function into a Hex string with its parameters.
 * @param options - The options for the tokenURI function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeTokenURI } from "thirdweb/extensions/erc721";
 * const result = encodeTokenURI({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeTokenURI(options: TokenURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the tokenURI function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeTokenURIResult } from "thirdweb/extensions/erc721";
 * const result = decodeTokenURIResultResult("...");
 * ```
 */
export declare function decodeTokenURIResult(result: Hex): string;
/**
 * Calls the "tokenURI" function on the contract.
 * @param options - The options for the tokenURI function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { tokenURI } from "thirdweb/extensions/erc721";
 *
 * const result = await tokenURI({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function tokenURI(options: BaseTransactionOptions<TokenURIParams>): Promise<string>;
//# sourceMappingURL=tokenURI.d.ts.map