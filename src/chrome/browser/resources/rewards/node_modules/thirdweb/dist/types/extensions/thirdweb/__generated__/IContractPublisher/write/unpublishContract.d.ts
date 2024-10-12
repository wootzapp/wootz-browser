import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "unpublishContract" function.
 */
export type UnpublishContractParams = WithOverrides<{
    publisher: AbiParameterToPrimitiveType<{
        type: "address";
        name: "publisher";
    }>;
    contractId: AbiParameterToPrimitiveType<{
        type: "string";
        name: "contractId";
    }>;
}>;
export declare const FN_SELECTOR: "0x06eb56cc";
/**
 * Checks if the `unpublishContract` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `unpublishContract` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isUnpublishContractSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isUnpublishContractSupported(["0x..."]);
 * ```
 */
export declare function isUnpublishContractSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "unpublishContract" function.
 * @param options - The options for the unpublishContract function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeUnpublishContractParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeUnpublishContractParams({
 *  publisher: ...,
 *  contractId: ...,
 * });
 * ```
 */
export declare function encodeUnpublishContractParams(options: UnpublishContractParams): `0x${string}`;
/**
 * Encodes the "unpublishContract" function into a Hex string with its parameters.
 * @param options - The options for the unpublishContract function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeUnpublishContract } from "thirdweb/extensions/thirdweb";
 * const result = encodeUnpublishContract({
 *  publisher: ...,
 *  contractId: ...,
 * });
 * ```
 */
export declare function encodeUnpublishContract(options: UnpublishContractParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "unpublishContract" function on the contract.
 * @param options - The options for the "unpublishContract" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { unpublishContract } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = unpublishContract({
 *  contract,
 *  publisher: ...,
 *  contractId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function unpublishContract(options: BaseTransactionOptions<UnpublishContractParams | {
    asyncParams: () => Promise<UnpublishContractParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=unpublishContract.d.ts.map