import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "remove" function.
 */
export type RemoveParams = WithOverrides<{
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
    }>;
}>;
export declare const FN_SELECTOR: "0x58edef4c";
/**
 * Checks if the `remove` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `remove` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRemoveSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isRemoveSupported(["0x..."]);
 * ```
 */
export declare function isRemoveSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "remove" function.
 * @param options - The options for the remove function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRemoveParams } from "thirdweb/extensions/farcaster";
 * const result = encodeRemoveParams({
 *  key: ...,
 * });
 * ```
 */
export declare function encodeRemoveParams(options: RemoveParams): `0x${string}`;
/**
 * Encodes the "remove" function into a Hex string with its parameters.
 * @param options - The options for the remove function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRemove } from "thirdweb/extensions/farcaster";
 * const result = encodeRemove({
 *  key: ...,
 * });
 * ```
 */
export declare function encodeRemove(options: RemoveParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "remove" function on the contract.
 * @param options - The options for the "remove" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { remove } from "thirdweb/extensions/farcaster";
 *
 * const transaction = remove({
 *  contract,
 *  key: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function remove(options: BaseTransactionOptions<RemoveParams | {
    asyncParams: () => Promise<RemoveParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=remove.d.ts.map