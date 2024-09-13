import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "removeFor" function.
 */
export type RemoveForParams = WithOverrides<{
    fidOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "fidOwner";
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
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
export declare const FN_SELECTOR: "0x787bd966";
/**
 * Checks if the `removeFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `removeFor` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRemoveForSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isRemoveForSupported(["0x..."]);
 * ```
 */
export declare function isRemoveForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "removeFor" function.
 * @param options - The options for the removeFor function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRemoveForParams } from "thirdweb/extensions/farcaster";
 * const result = encodeRemoveForParams({
 *  fidOwner: ...,
 *  key: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeRemoveForParams(options: RemoveForParams): `0x${string}`;
/**
 * Encodes the "removeFor" function into a Hex string with its parameters.
 * @param options - The options for the removeFor function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRemoveFor } from "thirdweb/extensions/farcaster";
 * const result = encodeRemoveFor({
 *  fidOwner: ...,
 *  key: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeRemoveFor(options: RemoveForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "removeFor" function on the contract.
 * @param options - The options for the "removeFor" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { removeFor } from "thirdweb/extensions/farcaster";
 *
 * const transaction = removeFor({
 *  contract,
 *  fidOwner: ...,
 *  key: ...,
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
export declare function removeFor(options: BaseTransactionOptions<RemoveForParams | {
    asyncParams: () => Promise<RemoveForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=removeFor.d.ts.map