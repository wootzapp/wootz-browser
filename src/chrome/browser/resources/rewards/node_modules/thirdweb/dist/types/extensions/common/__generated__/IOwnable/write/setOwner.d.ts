import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setOwner" function.
 */
export type SetOwnerParams = WithOverrides<{
    newOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_newOwner";
    }>;
}>;
export declare const FN_SELECTOR: "0x13af4035";
/**
 * Checks if the `setOwner` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setOwner` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isSetOwnerSupported } from "thirdweb/extensions/common";
 *
 * const supported = isSetOwnerSupported(["0x..."]);
 * ```
 */
export declare function isSetOwnerSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setOwner" function.
 * @param options - The options for the setOwner function.
 * @returns The encoded ABI parameters.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetOwnerParams } from "thirdweb/extensions/common";
 * const result = encodeSetOwnerParams({
 *  newOwner: ...,
 * });
 * ```
 */
export declare function encodeSetOwnerParams(options: SetOwnerParams): `0x${string}`;
/**
 * Encodes the "setOwner" function into a Hex string with its parameters.
 * @param options - The options for the setOwner function.
 * @returns The encoded hexadecimal string.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetOwner } from "thirdweb/extensions/common";
 * const result = encodeSetOwner({
 *  newOwner: ...,
 * });
 * ```
 */
export declare function encodeSetOwner(options: SetOwnerParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setOwner" function on the contract.
 * @param options - The options for the "setOwner" function.
 * @returns A prepared transaction object.
 * @extension COMMON
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setOwner } from "thirdweb/extensions/common";
 *
 * const transaction = setOwner({
 *  contract,
 *  newOwner: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setOwner(options: BaseTransactionOptions<SetOwnerParams | {
    asyncParams: () => Promise<SetOwnerParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setOwner.d.ts.map