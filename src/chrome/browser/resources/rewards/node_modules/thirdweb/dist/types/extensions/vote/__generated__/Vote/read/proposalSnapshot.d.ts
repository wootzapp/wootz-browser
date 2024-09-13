import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "proposalSnapshot" function.
 */
export type ProposalSnapshotParams = {
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
};
export declare const FN_SELECTOR: "0x2d63f693";
/**
 * Checks if the `proposalSnapshot` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `proposalSnapshot` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isProposalSnapshotSupported } from "thirdweb/extensions/vote";
 * const supported = isProposalSnapshotSupported(["0x..."]);
 * ```
 */
export declare function isProposalSnapshotSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "proposalSnapshot" function.
 * @param options - The options for the proposalSnapshot function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeProposalSnapshotParams } from "thirdweb/extensions/vote";
 * const result = encodeProposalSnapshotParams({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeProposalSnapshotParams(options: ProposalSnapshotParams): `0x${string}`;
/**
 * Encodes the "proposalSnapshot" function into a Hex string with its parameters.
 * @param options - The options for the proposalSnapshot function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeProposalSnapshot } from "thirdweb/extensions/vote";
 * const result = encodeProposalSnapshot({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeProposalSnapshot(options: ProposalSnapshotParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the proposalSnapshot function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeProposalSnapshotResult } from "thirdweb/extensions/vote";
 * const result = decodeProposalSnapshotResultResult("...");
 * ```
 */
export declare function decodeProposalSnapshotResult(result: Hex): bigint;
/**
 * Calls the "proposalSnapshot" function on the contract.
 * @param options - The options for the proposalSnapshot function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { proposalSnapshot } from "thirdweb/extensions/vote";
 *
 * const result = await proposalSnapshot({
 *  contract,
 *  proposalId: ...,
 * });
 *
 * ```
 */
export declare function proposalSnapshot(options: BaseTransactionOptions<ProposalSnapshotParams>): Promise<bigint>;
//# sourceMappingURL=proposalSnapshot.d.ts.map