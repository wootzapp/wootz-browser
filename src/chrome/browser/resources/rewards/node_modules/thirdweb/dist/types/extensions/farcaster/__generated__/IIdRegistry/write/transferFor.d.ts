import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "transferFor" function.
 */
export type TransferForParams = WithOverrides<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
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
export declare const FN_SELECTOR: "0x16f72842";
/**
 * Checks if the `transferFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `transferFor` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isTransferForSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isTransferForSupported(["0x..."]);
 * ```
 */
export declare function isTransferForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "transferFor" function.
 * @param options - The options for the transferFor function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransferForParams } from "thirdweb/extensions/farcaster";
 * const result = encodeTransferForParams({
 *  from: ...,
 *  to: ...,
 *  fromDeadline: ...,
 *  fromSig: ...,
 *  toDeadline: ...,
 *  toSig: ...,
 * });
 * ```
 */
export declare function encodeTransferForParams(options: TransferForParams): `0x${string}`;
/**
 * Encodes the "transferFor" function into a Hex string with its parameters.
 * @param options - The options for the transferFor function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransferFor } from "thirdweb/extensions/farcaster";
 * const result = encodeTransferFor({
 *  from: ...,
 *  to: ...,
 *  fromDeadline: ...,
 *  fromSig: ...,
 *  toDeadline: ...,
 *  toSig: ...,
 * });
 * ```
 */
export declare function encodeTransferFor(options: TransferForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "transferFor" function on the contract.
 * @param options - The options for the "transferFor" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { transferFor } from "thirdweb/extensions/farcaster";
 *
 * const transaction = transferFor({
 *  contract,
 *  from: ...,
 *  to: ...,
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
export declare function transferFor(options: BaseTransactionOptions<TransferForParams | {
    asyncParams: () => Promise<TransferForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=transferFor.d.ts.map