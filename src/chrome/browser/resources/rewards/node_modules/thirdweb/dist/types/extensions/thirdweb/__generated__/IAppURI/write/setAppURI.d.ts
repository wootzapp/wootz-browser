import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setAppURI" function.
 */
export type SetAppURIParams = WithOverrides<{
    uri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_uri";
    }>;
}>;
export declare const FN_SELECTOR: "0xfea18082";
/**
 * Checks if the `setAppURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setAppURI` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isSetAppURISupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isSetAppURISupported(["0x..."]);
 * ```
 */
export declare function isSetAppURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setAppURI" function.
 * @param options - The options for the setAppURI function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetAppURIParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetAppURIParams({
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetAppURIParams(options: SetAppURIParams): `0x${string}`;
/**
 * Encodes the "setAppURI" function into a Hex string with its parameters.
 * @param options - The options for the setAppURI function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetAppURI } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetAppURI({
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetAppURI(options: SetAppURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setAppURI" function on the contract.
 * @param options - The options for the "setAppURI" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setAppURI } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = setAppURI({
 *  contract,
 *  uri: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setAppURI(options: BaseTransactionOptions<SetAppURIParams | {
    asyncParams: () => Promise<SetAppURIParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setAppURI.d.ts.map