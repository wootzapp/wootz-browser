import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "recoverFor" function.
 */
export type RecoverForParams = WithOverrides<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    recoveryDeadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "recoveryDeadline";
    }>;
    recoverySig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "recoverySig";
    }>;
    toDeadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "toDeadline";
    }>;
    toSig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "toSig";
    }>;
}>;
export declare const FN_SELECTOR: "0xba656434";
/**
 * Checks if the `recoverFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `recoverFor` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRecoverForSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isRecoverForSupported(["0x..."]);
 * ```
 */
export declare function isRecoverForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "recoverFor" function.
 * @param options - The options for the recoverFor function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRecoverForParams } from "thirdweb/extensions/farcaster";
 * const result = encodeRecoverForParams({
 *  from: ...,
 *  to: ...,
 *  recoveryDeadline: ...,
 *  recoverySig: ...,
 *  toDeadline: ...,
 *  toSig: ...,
 * });
 * ```
 */
export declare function encodeRecoverForParams(options: RecoverForParams): `0x${string}`;
/**
 * Encodes the "recoverFor" function into a Hex string with its parameters.
 * @param options - The options for the recoverFor function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRecoverFor } from "thirdweb/extensions/farcaster";
 * const result = encodeRecoverFor({
 *  from: ...,
 *  to: ...,
 *  recoveryDeadline: ...,
 *  recoverySig: ...,
 *  toDeadline: ...,
 *  toSig: ...,
 * });
 * ```
 */
export declare function encodeRecoverFor(options: RecoverForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "recoverFor" function on the contract.
 * @param options - The options for the "recoverFor" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { recoverFor } from "thirdweb/extensions/farcaster";
 *
 * const transaction = recoverFor({
 *  contract,
 *  from: ...,
 *  to: ...,
 *  recoveryDeadline: ...,
 *  recoverySig: ...,
 *  toDeadline: ...,
 *  toSig: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function recoverFor(options: BaseTransactionOptions<RecoverForParams | {
    asyncParams: () => Promise<RecoverForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=recoverFor.d.ts.map