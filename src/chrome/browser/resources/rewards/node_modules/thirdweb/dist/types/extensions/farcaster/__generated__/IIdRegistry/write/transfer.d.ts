import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "transfer" function.
 */
export type TransferParams = WithOverrides<{
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
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
export declare const FN_SELECTOR: "0xbe45fd62";
/**
 * Checks if the `transfer` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `transfer` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isTransferSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isTransferSupported(["0x..."]);
 * ```
 */
export declare function isTransferSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "transfer" function.
 * @param options - The options for the transfer function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransferParams } from "thirdweb/extensions/farcaster";
 * const result = encodeTransferParams({
 *  to: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeTransferParams(options: TransferParams): `0x${string}`;
/**
 * Encodes the "transfer" function into a Hex string with its parameters.
 * @param options - The options for the transfer function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeTransfer } from "thirdweb/extensions/farcaster";
 * const result = encodeTransfer({
 *  to: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeTransfer(options: TransferParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "transfer" function on the contract.
 * @param options - The options for the "transfer" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { transfer } from "thirdweb/extensions/farcaster";
 *
 * const transaction = transfer({
 *  contract,
 *  to: ...,
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
export declare function transfer(options: BaseTransactionOptions<TransferParams | {
    asyncParams: () => Promise<TransferParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=transfer.d.ts.map