import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getPastVotes" function.
 */
export type GetPastVotesParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
    blockNumber: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "blockNumber";
    }>;
};
export declare const FN_SELECTOR: "0x3a46b1a8";
/**
 * Checks if the `getPastVotes` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getPastVotes` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isGetPastVotesSupported } from "thirdweb/extensions/erc20";
 * const supported = isGetPastVotesSupported(["0x..."]);
 * ```
 */
export declare function isGetPastVotesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getPastVotes" function.
 * @param options - The options for the getPastVotes function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeGetPastVotesParams } from "thirdweb/extensions/erc20";
 * const result = encodeGetPastVotesParams({
 *  account: ...,
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeGetPastVotesParams(options: GetPastVotesParams): `0x${string}`;
/**
 * Encodes the "getPastVotes" function into a Hex string with its parameters.
 * @param options - The options for the getPastVotes function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeGetPastVotes } from "thirdweb/extensions/erc20";
 * const result = encodeGetPastVotes({
 *  account: ...,
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeGetPastVotes(options: GetPastVotesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getPastVotes function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeGetPastVotesResult } from "thirdweb/extensions/erc20";
 * const result = decodeGetPastVotesResultResult("...");
 * ```
 */
export declare function decodeGetPastVotesResult(result: Hex): bigint;
/**
 * Calls the "getPastVotes" function on the contract.
 * @param options - The options for the getPastVotes function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { getPastVotes } from "thirdweb/extensions/erc20";
 *
 * const result = await getPastVotes({
 *  contract,
 *  account: ...,
 *  blockNumber: ...,
 * });
 *
 * ```
 */
export declare function getPastVotes(options: BaseTransactionOptions<GetPastVotesParams>): Promise<bigint>;
//# sourceMappingURL=getPastVotes.d.ts.map