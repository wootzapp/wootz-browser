import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "balanceOf" function.
 */
export type BalanceOfParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_owner";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
};
export declare const FN_SELECTOR: "0x00fdd58e";
/**
 * Checks if the `balanceOf` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `balanceOf` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isBalanceOfSupported } from "thirdweb/extensions/erc1155";
 * const supported = isBalanceOfSupported(["0x..."]);
 * ```
 */
export declare function isBalanceOfSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "balanceOf" function.
 * @param options - The options for the balanceOf function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBalanceOfParams } from "thirdweb/extensions/erc1155";
 * const result = encodeBalanceOfParams({
 *  owner: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeBalanceOfParams(options: BalanceOfParams): `0x${string}`;
/**
 * Encodes the "balanceOf" function into a Hex string with its parameters.
 * @param options - The options for the balanceOf function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBalanceOf } from "thirdweb/extensions/erc1155";
 * const result = encodeBalanceOf({
 *  owner: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeBalanceOf(options: BalanceOfParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the balanceOf function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeBalanceOfResult } from "thirdweb/extensions/erc1155";
 * const result = decodeBalanceOfResultResult("...");
 * ```
 */
export declare function decodeBalanceOfResult(result: Hex): bigint;
/**
 * Calls the "balanceOf" function on the contract.
 * @param options - The options for the balanceOf function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { balanceOf } from "thirdweb/extensions/erc1155";
 *
 * const result = await balanceOf({
 *  contract,
 *  owner: ...,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function balanceOf(options: BaseTransactionOptions<BalanceOfParams>): Promise<bigint>;
//# sourceMappingURL=balanceOf.d.ts.map