import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "propose" function.
 */
export type ProposeParams = WithOverrides<{
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
    description: AbiParameterToPrimitiveType<{
        type: "string";
        name: "description";
    }>;
}>;
export declare const FN_SELECTOR: "0x7d5e81e2";
/**
 * Checks if the `propose` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `propose` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isProposeSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isProposeSupported(["0x..."]);
 * ```
 */
export declare function isProposeSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "propose" function.
 * @param options - The options for the propose function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeProposeParams } from "thirdweb/extensions/vote";
 * const result = encodeProposeParams({
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  description: ...,
 * });
 * ```
 */
export declare function encodeProposeParams(options: ProposeParams): `0x${string}`;
/**
 * Encodes the "propose" function into a Hex string with its parameters.
 * @param options - The options for the propose function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodePropose } from "thirdweb/extensions/vote";
 * const result = encodePropose({
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  description: ...,
 * });
 * ```
 */
export declare function encodePropose(options: ProposeParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "propose" function on the contract.
 * @param options - The options for the "propose" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { propose } from "thirdweb/extensions/vote";
 *
 * const transaction = propose({
 *  contract,
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  description: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function propose(options: BaseTransactionOptions<ProposeParams | {
    asyncParams: () => Promise<ProposeParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=propose.d.ts.map