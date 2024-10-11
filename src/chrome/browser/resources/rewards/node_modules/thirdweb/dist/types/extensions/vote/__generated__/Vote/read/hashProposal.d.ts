import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "hashProposal" function.
 */
export type HashProposalParams = {
    targets: AbiParameterToPrimitiveType<{
        type: "address[]";
        name: "targets";
    }>;
    values: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "values";
    }>;
    calldatas: AbiParameterToPrimitiveType<{
        type: "bytes[]";
        name: "calldatas";
    }>;
    descriptionHash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "descriptionHash";
    }>;
};
export declare const FN_SELECTOR: "0xc59057e4";
/**
 * Checks if the `hashProposal` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `hashProposal` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isHashProposalSupported } from "thirdweb/extensions/vote";
 * const supported = isHashProposalSupported(["0x..."]);
 * ```
 */
export declare function isHashProposalSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "hashProposal" function.
 * @param options - The options for the hashProposal function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeHashProposalParams } from "thirdweb/extensions/vote";
 * const result = encodeHashProposalParams({
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  descriptionHash: ...,
 * });
 * ```
 */
export declare function encodeHashProposalParams(options: HashProposalParams): `0x${string}`;
/**
 * Encodes the "hashProposal" function into a Hex string with its parameters.
 * @param options - The options for the hashProposal function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeHashProposal } from "thirdweb/extensions/vote";
 * const result = encodeHashProposal({
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  descriptionHash: ...,
 * });
 * ```
 */
export declare function encodeHashProposal(options: HashProposalParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the hashProposal function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeHashProposalResult } from "thirdweb/extensions/vote";
 * const result = decodeHashProposalResultResult("...");
 * ```
 */
export declare function decodeHashProposalResult(result: Hex): bigint;
/**
 * Calls the "hashProposal" function on the contract.
 * @param options - The options for the hashProposal function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { hashProposal } from "thirdweb/extensions/vote";
 *
 * const result = await hashProposal({
 *  contract,
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  descriptionHash: ...,
 * });
 *
 * ```
 */
export declare function hashProposal(options: BaseTransactionOptions<HashProposalParams>): Promise<bigint>;
//# sourceMappingURL=hashProposal.d.ts.map