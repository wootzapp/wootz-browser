import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setContractURI" function.
 */
export type SetContractURIParams = WithOverrides<{
    uri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_uri";
    }>;
}>;
export declare const FN_SELECTOR: "0x938e3d7b";
/**
 * Checks if the `setContractURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setContractURI` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isSetContractURISupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isSetContractURISupported(["0x..."]);
 * ```
 */
export declare function isSetContractURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setContractURI" function.
 * @param options - The options for the setContractURI function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetContractURIParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetContractURIParams({
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetContractURIParams(options: SetContractURIParams): `0x${string}`;
/**
 * Encodes the "setContractURI" function into a Hex string with its parameters.
 * @param options - The options for the setContractURI function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetContractURI } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetContractURI({
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetContractURI(options: SetContractURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setContractURI" function on the contract.
 * @param options - The options for the "setContractURI" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setContractURI } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = setContractURI({
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
export declare function setContractURI(options: BaseTransactionOptions<SetContractURIParams | {
    asyncParams: () => Promise<SetContractURIParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setContractURI.d.ts.map