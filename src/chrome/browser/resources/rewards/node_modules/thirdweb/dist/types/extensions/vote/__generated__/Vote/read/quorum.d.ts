import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "quorum" function.
 */
export type QuorumParams = {
    blockNumber: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "blockNumber";
    }>;
};
export declare const FN_SELECTOR: "0xf8ce560a";
/**
 * Checks if the `quorum` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `quorum` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isQuorumSupported } from "thirdweb/extensions/vote";
 * const supported = isQuorumSupported(["0x..."]);
 * ```
 */
export declare function isQuorumSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "quorum" function.
 * @param options - The options for the quorum function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeQuorumParams } from "thirdweb/extensions/vote";
 * const result = encodeQuorumParams({
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeQuorumParams(options: QuorumParams): `0x${string}`;
/**
 * Encodes the "quorum" function into a Hex string with its parameters.
 * @param options - The options for the quorum function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeQuorum } from "thirdweb/extensions/vote";
 * const result = encodeQuorum({
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeQuorum(options: QuorumParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the quorum function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeQuorumResult } from "thirdweb/extensions/vote";
 * const result = decodeQuorumResultResult("...");
 * ```
 */
export declare function decodeQuorumResult(result: Hex): bigint;
/**
 * Calls the "quorum" function on the contract.
 * @param options - The options for the quorum function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { quorum } from "thirdweb/extensions/vote";
 *
 * const result = await quorum({
 *  contract,
 *  blockNumber: ...,
 * });
 *
 * ```
 */
export declare function quorum(options: BaseTransactionOptions<QuorumParams>): Promise<bigint>;
//# sourceMappingURL=quorum.d.ts.map