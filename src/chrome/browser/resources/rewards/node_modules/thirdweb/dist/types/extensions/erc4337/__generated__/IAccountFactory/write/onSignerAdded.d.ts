import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "onSignerAdded" function.
 */
export type OnSignerAddedParams = WithOverrides<{
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
    }>;
    creatorAdmin: AbiParameterToPrimitiveType<{
        type: "address";
        name: "creatorAdmin";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0x9ddbb9d8";
/**
 * Checks if the `onSignerAdded` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `onSignerAdded` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isOnSignerAddedSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isOnSignerAddedSupported(["0x..."]);
 * ```
 */
export declare function isOnSignerAddedSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "onSignerAdded" function.
 * @param options - The options for the onSignerAdded function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeOnSignerAddedParams } from "thirdweb/extensions/erc4337";
 * const result = encodeOnSignerAddedParams({
 *  signer: ...,
 *  creatorAdmin: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnSignerAddedParams(options: OnSignerAddedParams): `0x${string}`;
/**
 * Encodes the "onSignerAdded" function into a Hex string with its parameters.
 * @param options - The options for the onSignerAdded function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeOnSignerAdded } from "thirdweb/extensions/erc4337";
 * const result = encodeOnSignerAdded({
 *  signer: ...,
 *  creatorAdmin: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnSignerAdded(options: OnSignerAddedParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "onSignerAdded" function on the contract.
 * @param options - The options for the "onSignerAdded" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { onSignerAdded } from "thirdweb/extensions/erc4337";
 *
 * const transaction = onSignerAdded({
 *  contract,
 *  signer: ...,
 *  creatorAdmin: ...,
 *  data: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function onSignerAdded(options: BaseTransactionOptions<OnSignerAddedParams | {
    asyncParams: () => Promise<OnSignerAddedParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=onSignerAdded.d.ts.map