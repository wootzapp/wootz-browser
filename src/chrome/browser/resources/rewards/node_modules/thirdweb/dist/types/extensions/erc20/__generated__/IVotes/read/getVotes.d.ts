import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getVotes" function.
 */
export type GetVotesParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0x9ab24eb0";
/**
 * Checks if the `getVotes` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getVotes` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isGetVotesSupported } from "thirdweb/extensions/erc20";
 * const supported = isGetVotesSupported(["0x..."]);
 * ```
 */
export declare function isGetVotesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getVotes" function.
 * @param options - The options for the getVotes function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeGetVotesParams } from "thirdweb/extensions/erc20";
 * const result = encodeGetVotesParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeGetVotesParams(options: GetVotesParams): `0x${string}`;
/**
 * Encodes the "getVotes" function into a Hex string with its parameters.
 * @param options - The options for the getVotes function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeGetVotes } from "thirdweb/extensions/erc20";
 * const result = encodeGetVotes({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeGetVotes(options: GetVotesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getVotes function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeGetVotesResult } from "thirdweb/extensions/erc20";
 * const result = decodeGetVotesResultResult("...");
 * ```
 */
export declare function decodeGetVotesResult(result: Hex): bigint;
/**
 * Calls the "getVotes" function on the contract.
 * @param options - The options for the getVotes function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { getVotes } from "thirdweb/extensions/erc20";
 *
 * const result = await getVotes({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function getVotes(options: BaseTransactionOptions<GetVotesParams>): Promise<bigint>;
//# sourceMappingURL=getVotes.d.ts.map