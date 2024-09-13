import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setPublisherProfileUri" function.
 */
export type SetPublisherProfileUriParams = WithOverrides<{
    publisher: AbiParameterToPrimitiveType<{
        type: "address";
        name: "publisher";
    }>;
    uri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "uri";
    }>;
}>;
export declare const FN_SELECTOR: "0x6e578e54";
/**
 * Checks if the `setPublisherProfileUri` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setPublisherProfileUri` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isSetPublisherProfileUriSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isSetPublisherProfileUriSupported(["0x..."]);
 * ```
 */
export declare function isSetPublisherProfileUriSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setPublisherProfileUri" function.
 * @param options - The options for the setPublisherProfileUri function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetPublisherProfileUriParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetPublisherProfileUriParams({
 *  publisher: ...,
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetPublisherProfileUriParams(options: SetPublisherProfileUriParams): `0x${string}`;
/**
 * Encodes the "setPublisherProfileUri" function into a Hex string with its parameters.
 * @param options - The options for the setPublisherProfileUri function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetPublisherProfileUri } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetPublisherProfileUri({
 *  publisher: ...,
 *  uri: ...,
 * });
 * ```
 */
export declare function encodeSetPublisherProfileUri(options: SetPublisherProfileUriParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setPublisherProfileUri" function on the contract.
 * @param options - The options for the "setPublisherProfileUri" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setPublisherProfileUri } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = setPublisherProfileUri({
 *  contract,
 *  publisher: ...,
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
export declare function setPublisherProfileUri(options: BaseTransactionOptions<SetPublisherProfileUriParams | {
    asyncParams: () => Promise<SetPublisherProfileUriParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setPublisherProfileUri.d.ts.map