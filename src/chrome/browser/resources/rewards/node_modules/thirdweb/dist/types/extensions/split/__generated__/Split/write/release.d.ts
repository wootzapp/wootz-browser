import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "release" function.
 */
export type ReleaseParams = WithOverrides<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
}>;
export declare const FN_SELECTOR: "0x19165587";
/**
 * Checks if the `release` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `release` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isReleaseSupported } from "thirdweb/extensions/split";
 *
 * const supported = isReleaseSupported(["0x..."]);
 * ```
 */
export declare function isReleaseSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "release" function.
 * @param options - The options for the release function.
 * @returns The encoded ABI parameters.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeReleaseParams } from "thirdweb/extensions/split";
 * const result = encodeReleaseParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeReleaseParams(options: ReleaseParams): `0x${string}`;
/**
 * Encodes the "release" function into a Hex string with its parameters.
 * @param options - The options for the release function.
 * @returns The encoded hexadecimal string.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeRelease } from "thirdweb/extensions/split";
 * const result = encodeRelease({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeRelease(options: ReleaseParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "release" function on the contract.
 * @param options - The options for the "release" function.
 * @returns A prepared transaction object.
 * @extension SPLIT
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { release } from "thirdweb/extensions/split";
 *
 * const transaction = release({
 *  contract,
 *  account: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function release(options: BaseTransactionOptions<ReleaseParams | {
    asyncParams: () => Promise<ReleaseParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=release.d.ts.map