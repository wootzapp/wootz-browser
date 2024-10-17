import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "freezeBatchBaseURI" function.
 */
export type FreezeBatchBaseURIParams = WithOverrides<{
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_index";
    }>;
}>;
export declare const FN_SELECTOR: "0xa07ced9e";
/**
 * Checks if the `freezeBatchBaseURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `freezeBatchBaseURI` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isFreezeBatchBaseURISupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isFreezeBatchBaseURISupported(["0x..."]);
 * ```
 */
export declare function isFreezeBatchBaseURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "freezeBatchBaseURI" function.
 * @param options - The options for the freezeBatchBaseURI function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeFreezeBatchBaseURIParams } from "thirdweb/extensions/erc1155";
 * const result = encodeFreezeBatchBaseURIParams({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeFreezeBatchBaseURIParams(options: FreezeBatchBaseURIParams): `0x${string}`;
/**
 * Encodes the "freezeBatchBaseURI" function into a Hex string with its parameters.
 * @param options - The options for the freezeBatchBaseURI function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeFreezeBatchBaseURI } from "thirdweb/extensions/erc1155";
 * const result = encodeFreezeBatchBaseURI({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeFreezeBatchBaseURI(options: FreezeBatchBaseURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "freezeBatchBaseURI" function on the contract.
 * @param options - The options for the "freezeBatchBaseURI" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { freezeBatchBaseURI } from "thirdweb/extensions/erc1155";
 *
 * const transaction = freezeBatchBaseURI({
 *  contract,
 *  index: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function freezeBatchBaseURI(options: BaseTransactionOptions<FreezeBatchBaseURIParams | {
    asyncParams: () => Promise<FreezeBatchBaseURIParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=freezeBatchBaseURI.d.ts.map