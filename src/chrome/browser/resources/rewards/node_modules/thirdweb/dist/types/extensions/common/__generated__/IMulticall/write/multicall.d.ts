import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "multicall" function.
 */
export type MulticallParams = WithOverrides<{
    data: AbiParameterToPrimitiveType<{
        type: "bytes[]";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0xac9650d8";
/**
 * Checks if the `multicall` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `multicall` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isMulticallSupported } from "thirdweb/extensions/common";
 *
 * const supported = isMulticallSupported(["0x..."]);
 * ```
 */
export declare function isMulticallSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "multicall" function.
 * @param options - The options for the multicall function.
 * @returns The encoded ABI parameters.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeMulticallParams } from "thirdweb/extensions/common";
 * const result = encodeMulticallParams({
 *  data: ...,
 * });
 * ```
 */
export declare function encodeMulticallParams(options: MulticallParams): `0x${string}`;
/**
 * Encodes the "multicall" function into a Hex string with its parameters.
 * @param options - The options for the multicall function.
 * @returns The encoded hexadecimal string.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeMulticall } from "thirdweb/extensions/common";
 * const result = encodeMulticall({
 *  data: ...,
 * });
 * ```
 */
export declare function encodeMulticall(options: MulticallParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "multicall" function on the contract.
 * @param options - The options for the "multicall" function.
 * @returns A prepared transaction object.
 * @extension COMMON
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { multicall } from "thirdweb/extensions/common";
 *
 * const transaction = multicall({
 *  contract,
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
export declare function multicall(options: BaseTransactionOptions<MulticallParams | {
    asyncParams: () => Promise<MulticallParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=multicall.d.ts.map