import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "initialize" function.
 */
export type InitializeParams = WithOverrides<{
    defaultAdmin: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_defaultAdmin";
    }>;
    name: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_name";
    }>;
    symbol: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_symbol";
    }>;
    contractURI: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_contractURI";
    }>;
    trustedForwarders: AbiParameterToPrimitiveType<{
        type: "address[]";
        name: "_trustedForwarders";
    }>;
    primarySaleRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_primarySaleRecipient";
    }>;
    platformFeeRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_platformFeeRecipient";
    }>;
    platformFeeBps: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_platformFeeBps";
    }>;
}>;
export declare const FN_SELECTOR: "0xdfad80a6";
/**
 * Checks if the `initialize` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `initialize` method is supported.
 * @extension PREBUILTS
 * @example
 * ```ts
 * import { isInitializeSupported } from "thirdweb/extensions/prebuilts";
 *
 * const supported = isInitializeSupported(["0x..."]);
 * ```
 */
export declare function isInitializeSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "initialize" function.
 * @param options - The options for the initialize function.
 * @returns The encoded ABI parameters.
 * @extension PREBUILTS
 * @example
 * ```ts
 * import { encodeInitializeParams } from "thirdweb/extensions/prebuilts";
 * const result = encodeInitializeParams({
 *  defaultAdmin: ...,
 *  name: ...,
 *  symbol: ...,
 *  contractURI: ...,
 *  trustedForwarders: ...,
 *  primarySaleRecipient: ...,
 *  platformFeeRecipient: ...,
 *  platformFeeBps: ...,
 * });
 * ```
 */
export declare function encodeInitializeParams(options: InitializeParams): `0x${string}`;
/**
 * Encodes the "initialize" function into a Hex string with its parameters.
 * @param options - The options for the initialize function.
 * @returns The encoded hexadecimal string.
 * @extension PREBUILTS
 * @example
 * ```ts
 * import { encodeInitialize } from "thirdweb/extensions/prebuilts";
 * const result = encodeInitialize({
 *  defaultAdmin: ...,
 *  name: ...,
 *  symbol: ...,
 *  contractURI: ...,
 *  trustedForwarders: ...,
 *  primarySaleRecipient: ...,
 *  platformFeeRecipient: ...,
 *  platformFeeBps: ...,
 * });
 * ```
 */
export declare function encodeInitialize(options: InitializeParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "initialize" function on the contract.
 * @param options - The options for the "initialize" function.
 * @returns A prepared transaction object.
 * @extension PREBUILTS
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { initialize } from "thirdweb/extensions/prebuilts";
 *
 * const transaction = initialize({
 *  contract,
 *  defaultAdmin: ...,
 *  name: ...,
 *  symbol: ...,
 *  contractURI: ...,
 *  trustedForwarders: ...,
 *  primarySaleRecipient: ...,
 *  platformFeeRecipient: ...,
 *  platformFeeBps: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function initialize(options: BaseTransactionOptions<InitializeParams | {
    asyncParams: () => Promise<InitializeParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=initialize.d.ts.map