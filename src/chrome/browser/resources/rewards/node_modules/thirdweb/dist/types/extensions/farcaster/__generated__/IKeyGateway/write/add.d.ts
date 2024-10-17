import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "add" function.
 */
export type AddParams = WithOverrides<{
    keyType: AbiParameterToPrimitiveType<{
        type: "uint32";
        name: "keyType";
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
    }>;
    metadataType: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "metadataType";
    }>;
    metadata: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "metadata";
    }>;
}>;
export declare const FN_SELECTOR: "0x22b1a414";
/**
 * Checks if the `add` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `add` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isAddSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isAddSupported(["0x..."]);
 * ```
 */
export declare function isAddSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "add" function.
 * @param options - The options for the add function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeAddParams } from "thirdweb/extensions/farcaster";
 * const result = encodeAddParams({
 *  keyType: ...,
 *  key: ...,
 *  metadataType: ...,
 *  metadata: ...,
 * });
 * ```
 */
export declare function encodeAddParams(options: AddParams): `0x${string}`;
/**
 * Encodes the "add" function into a Hex string with its parameters.
 * @param options - The options for the add function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeAdd } from "thirdweb/extensions/farcaster";
 * const result = encodeAdd({
 *  keyType: ...,
 *  key: ...,
 *  metadataType: ...,
 *  metadata: ...,
 * });
 * ```
 */
export declare function encodeAdd(options: AddParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "add" function on the contract.
 * @param options - The options for the "add" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { add } from "thirdweb/extensions/farcaster";
 *
 * const transaction = add({
 *  contract,
 *  keyType: ...,
 *  key: ...,
 *  metadataType: ...,
 *  metadata: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function add(options: BaseTransactionOptions<AddParams | {
    asyncParams: () => Promise<AddParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=add.d.ts.map