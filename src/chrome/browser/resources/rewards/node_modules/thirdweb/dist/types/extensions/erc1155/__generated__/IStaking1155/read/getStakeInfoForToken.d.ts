import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getStakeInfoForToken" function.
 */
export type GetStakeInfoForTokenParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    staker: AbiParameterToPrimitiveType<{
        type: "address";
        name: "staker";
    }>;
};
export declare const FN_SELECTOR: "0x168fb5c5";
/**
 * Checks if the `getStakeInfoForToken` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getStakeInfoForToken` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isGetStakeInfoForTokenSupported } from "thirdweb/extensions/erc1155";
 * const supported = isGetStakeInfoForTokenSupported(["0x..."]);
 * ```
 */
export declare function isGetStakeInfoForTokenSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getStakeInfoForToken" function.
 * @param options - The options for the getStakeInfoForToken function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeGetStakeInfoForTokenParams } from "thirdweb/extensions/erc1155";
 * const result = encodeGetStakeInfoForTokenParams({
 *  tokenId: ...,
 *  staker: ...,
 * });
 * ```
 */
export declare function encodeGetStakeInfoForTokenParams(options: GetStakeInfoForTokenParams): `0x${string}`;
/**
 * Encodes the "getStakeInfoForToken" function into a Hex string with its parameters.
 * @param options - The options for the getStakeInfoForToken function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeGetStakeInfoForToken } from "thirdweb/extensions/erc1155";
 * const result = encodeGetStakeInfoForToken({
 *  tokenId: ...,
 *  staker: ...,
 * });
 * ```
 */
export declare function encodeGetStakeInfoForToken(options: GetStakeInfoForTokenParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getStakeInfoForToken function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeGetStakeInfoForTokenResult } from "thirdweb/extensions/erc1155";
 * const result = decodeGetStakeInfoForTokenResultResult("...");
 * ```
 */
export declare function decodeGetStakeInfoForTokenResult(result: Hex): readonly [bigint, bigint];
/**
 * Calls the "getStakeInfoForToken" function on the contract.
 * @param options - The options for the getStakeInfoForToken function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getStakeInfoForToken } from "thirdweb/extensions/erc1155";
 *
 * const result = await getStakeInfoForToken({
 *  contract,
 *  tokenId: ...,
 *  staker: ...,
 * });
 *
 * ```
 */
export declare function getStakeInfoForToken(options: BaseTransactionOptions<GetStakeInfoForTokenParams>): Promise<readonly [bigint, bigint]>;
//# sourceMappingURL=getStakeInfoForToken.d.ts.map