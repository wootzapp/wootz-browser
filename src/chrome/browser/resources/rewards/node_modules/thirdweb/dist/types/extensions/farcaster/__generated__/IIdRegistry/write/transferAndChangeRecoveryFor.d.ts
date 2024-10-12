import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "transferAndChangeRecoveryFor" function.
 */
export type TransferAndChangeRecoveryForParams = WithOverrides<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    recovery: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recovery";
    }>;
    fromDeadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fromDeadline";
    }>;
    fromSig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "fromSig";
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
export declare const FN_SELECTOR: "0x4c5cbb34";
/**
 * Checks if the `transferAndChangeRecoveryFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `transferAndChangeRecoveryFor` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isTransferAndChangeRecoveryForSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isTransferAndChangeRecoveryForSupported(["0x..."]);
 * ```
 */
export declare function isTransferAndChangeRecoveryForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "transferAndChangeRecoveryFor" function.
 * @param options - The options for the transferAndChangeRecoveryFor function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransferAndChangeRecoveryForParams } from "thirdweb/extensions/farcaster";
 * const result = encodeTransferAndChangeRecoveryForParams({
 *  from: ...,
 *  to: ...,
 *  recovery: ...,
 *  fromDeadline: ...,
 *  fromSig: ...,
 *  toDeadline: ...,
 *  toSig: ...,
 * });
 * ```
 */
export declare function encodeTransferAndChangeRecoveryForParams(options: TransferAndChangeRecoveryForParams): `0x${string}`;
/**
 * Encodes the "transferAndChangeRecoveryFor" function into a Hex string with its parameters.
 * @param options - The options for the transferAndChangeRecoveryFor function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransferAndChangeRecoveryFor } from "thirdweb/extensions/farcaster";
 * const result = encodeTransferAndChangeRecoveryFor({
 *  from: ...,
 *  to: ...,
 *  recovery: ...,
 *  fromDeadline: ...,
 *  fromSig: ...,
 *  toDeadline: ...,
 *  toSig: ...,
 * });
 * ```
 */
export declare function encodeTransferAndChangeRecoveryFor(options: TransferAndChangeRecoveryForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "transferAndChangeRecoveryFor" function on the contract.
 * @param options - The options for the "transferAndChangeRecoveryFor" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { transferAndChangeRecoveryFor } from "thirdweb/extensions/farcaster";
 *
 * const transaction = transferAndChangeRecoveryFor({
 *  contract,
 *  from: ...,
 *  to: ...,
 *  recovery: ...,
 *  fromDeadline: ...,
 *  fromSig: ...,
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
export declare function transferAndChangeRecoveryFor(options: BaseTransactionOptions<TransferAndChangeRecoveryForParams | {
    asyncParams: () => Promise<TransferAndChangeRecoveryForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=transferAndChangeRecoveryFor.d.ts.map