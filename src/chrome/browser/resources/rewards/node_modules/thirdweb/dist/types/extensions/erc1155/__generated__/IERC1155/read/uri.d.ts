import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "uri" function.
 */
export type UriParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
};
export declare const FN_SELECTOR: "0x0e89341c";
/**
 * Checks if the `uri` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `uri` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isUriSupported } from "thirdweb/extensions/erc1155";
 * const supported = isUriSupported(["0x..."]);
 * ```
 */
export declare function isUriSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "uri" function.
 * @param options - The options for the uri function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeUriParams } from "thirdweb/extensions/erc1155";
 * const result = encodeUriParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeUriParams(options: UriParams): `0x${string}`;
/**
 * Encodes the "uri" function into a Hex string with its parameters.
 * @param options - The options for the uri function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeUri } from "thirdweb/extensions/erc1155";
 * const result = encodeUri({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeUri(options: UriParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the uri function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeUriResult } from "thirdweb/extensions/erc1155";
 * const result = decodeUriResultResult("...");
 * ```
 */
export declare function decodeUriResult(result: Hex): string;
/**
 * Calls the "uri" function on the contract.
 * @param options - The options for the uri function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { uri } from "thirdweb/extensions/erc1155";
 *
 * const result = await uri({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function uri(options: BaseTransactionOptions<UriParams>): Promise<string>;
//# sourceMappingURL=uri.d.ts.map