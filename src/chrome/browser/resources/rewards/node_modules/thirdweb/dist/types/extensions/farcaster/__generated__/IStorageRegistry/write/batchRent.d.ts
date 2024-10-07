import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "batchRent" function.
 */
export type BatchRentParams = WithOverrides<{
    fids: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "fids";
    }>;
    units: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "units";
    }>;
}>;
export declare const FN_SELECTOR: "0xa82c356e";
/**
 * Checks if the `batchRent` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `batchRent` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isBatchRentSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isBatchRentSupported(["0x..."]);
 * ```
 */
export declare function isBatchRentSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "batchRent" function.
 * @param options - The options for the batchRent function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeBatchRentParams } from "thirdweb/extensions/farcaster";
 * const result = encodeBatchRentParams({
 *  fids: ...,
 *  units: ...,
 * });
 * ```
 */
export declare function encodeBatchRentParams(options: BatchRentParams): `0x${string}`;
/**
 * Encodes the "batchRent" function into a Hex string with its parameters.
 * @param options - The options for the batchRent function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeBatchRent } from "thirdweb/extensions/farcaster";
 * const result = encodeBatchRent({
 *  fids: ...,
 *  units: ...,
 * });
 * ```
 */
export declare function encodeBatchRent(options: BatchRentParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "batchRent" function on the contract.
 * @param options - The options for the "batchRent" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { batchRent } from "thirdweb/extensions/farcaster";
 *
 * const transaction = batchRent({
 *  contract,
 *  fids: ...,
 *  units: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function batchRent(options: BaseTransactionOptions<BatchRentParams | {
    asyncParams: () => Promise<BatchRentParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=batchRent.d.ts.map