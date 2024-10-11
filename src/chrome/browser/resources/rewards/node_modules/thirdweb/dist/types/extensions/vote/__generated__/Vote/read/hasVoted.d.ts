import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "hasVoted" function.
 */
export type HasVotedParams = {
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0x43859632";
/**
 * Checks if the `hasVoted` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `hasVoted` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isHasVotedSupported } from "thirdweb/extensions/vote";
 * const supported = isHasVotedSupported(["0x..."]);
 * ```
 */
export declare function isHasVotedSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "hasVoted" function.
 * @param options - The options for the hasVoted function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeHasVotedParams } from "thirdweb/extensions/vote";
 * const result = encodeHasVotedParams({
 *  proposalId: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeHasVotedParams(options: HasVotedParams): `0x${string}`;
/**
 * Encodes the "hasVoted" function into a Hex string with its parameters.
 * @param options - The options for the hasVoted function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeHasVoted } from "thirdweb/extensions/vote";
 * const result = encodeHasVoted({
 *  proposalId: ...,
 *  account: ...,
 * });
 * ```
 */
export declare function encodeHasVoted(options: HasVotedParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the hasVoted function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeHasVotedResult } from "thirdweb/extensions/vote";
 * const result = decodeHasVotedResultResult("...");
 * ```
 */
export declare function decodeHasVotedResult(result: Hex): boolean;
/**
 * Calls the "hasVoted" function on the contract.
 * @param options - The options for the hasVoted function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { hasVoted } from "thirdweb/extensions/vote";
 *
 * const result = await hasVoted({
 *  contract,
 *  proposalId: ...,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function hasVoted(options: BaseTransactionOptions<HasVotedParams>): Promise<boolean>;
//# sourceMappingURL=hasVoted.d.ts.map