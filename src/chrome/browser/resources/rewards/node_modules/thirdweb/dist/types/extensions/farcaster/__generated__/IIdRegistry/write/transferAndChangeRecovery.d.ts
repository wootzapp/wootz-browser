import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "transferAndChangeRecovery" function.
 */
export type TransferAndChangeRecoveryParams = WithOverrides<{
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    recovery: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recovery";
    }>;
    deadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "deadline";
    }>;
    sig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "sig";
    }>;
}>;
export declare const FN_SELECTOR: "0x3ab8465d";
/**
 * Checks if the `transferAndChangeRecovery` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `transferAndChangeRecovery` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isTransferAndChangeRecoverySupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isTransferAndChangeRecoverySupported(["0x..."]);
 * ```
 */
export declare function isTransferAndChangeRecoverySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "transferAndChangeRecovery" function.
 * @param options - The options for the transferAndChangeRecovery function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransferAndChangeRecoveryParams } from "thirdweb/extensions/farcaster";
 * const result = encodeTransferAndChangeRecoveryParams({
 *  to: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeTransferAndChangeRecoveryParams(options: TransferAndChangeRecoveryParams): `0x${string}`;
/**
 * Encodes the "transferAndChangeRecovery" function into a Hex string with its parameters.
 * @param options - The options for the transferAndChangeRecovery function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransferAndChangeRecovery } from "thirdweb/extensions/farcaster";
 * const result = encodeTransferAndChangeRecovery({
 *  to: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeTransferAndChangeRecovery(options: TransferAndChangeRecoveryParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "transferAndChangeRecovery" function on the contract.
 * @param options - The options for the "transferAndChangeRecovery" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { transferAndChangeRecovery } from "thirdweb/extensions/farcaster";
 *
 * const transaction = transferAndChangeRecovery({
 *  contract,
 *  to: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function transferAndChangeRecovery(options: BaseTransactionOptions<TransferAndChangeRecoveryParams | {
    asyncParams: () => Promise<TransferAndChangeRecoveryParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=transferAndChangeRecovery.d.ts.map