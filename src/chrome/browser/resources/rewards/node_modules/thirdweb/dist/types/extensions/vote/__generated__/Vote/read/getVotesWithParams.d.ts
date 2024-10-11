import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getVotesWithParams" function.
 */
export type GetVotesWithParamsParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
    blockNumber: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "blockNumber";
    }>;
    params: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "params";
    }>;
};
export declare const FN_SELECTOR: "0x9a802a6d";
/**
 * Checks if the `getVotesWithParams` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getVotesWithParams` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isGetVotesWithParamsSupported } from "thirdweb/extensions/vote";
 * const supported = isGetVotesWithParamsSupported(["0x..."]);
 * ```
 */
export declare function isGetVotesWithParamsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getVotesWithParams" function.
 * @param options - The options for the getVotesWithParams function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeGetVotesWithParamsParams } from "thirdweb/extensions/vote";
 * const result = encodeGetVotesWithParamsParams({
 *  account: ...,
 *  blockNumber: ...,
 *  params: ...,
 * });
 * ```
 */
export declare function encodeGetVotesWithParamsParams(options: GetVotesWithParamsParams): `0x${string}`;
/**
 * Encodes the "getVotesWithParams" function into a Hex string with its parameters.
 * @param options - The options for the getVotesWithParams function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeGetVotesWithParams } from "thirdweb/extensions/vote";
 * const result = encodeGetVotesWithParams({
 *  account: ...,
 *  blockNumber: ...,
 *  params: ...,
 * });
 * ```
 */
export declare function encodeGetVotesWithParams(options: GetVotesWithParamsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getVotesWithParams function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeGetVotesWithParamsResult } from "thirdweb/extensions/vote";
 * const result = decodeGetVotesWithParamsResultResult("...");
 * ```
 */
export declare function decodeGetVotesWithParamsResult(result: Hex): bigint;
/**
 * Calls the "getVotesWithParams" function on the contract.
 * @param options - The options for the getVotesWithParams function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { getVotesWithParams } from "thirdweb/extensions/vote";
 *
 * const result = await getVotesWithParams({
 *  contract,
 *  account: ...,
 *  blockNumber: ...,
 *  params: ...,
 * });
 *
 * ```
 */
export declare function getVotesWithParams(options: BaseTransactionOptions<GetVotesWithParamsParams>): Promise<bigint>;
//# sourceMappingURL=getVotesWithParams.d.ts.map