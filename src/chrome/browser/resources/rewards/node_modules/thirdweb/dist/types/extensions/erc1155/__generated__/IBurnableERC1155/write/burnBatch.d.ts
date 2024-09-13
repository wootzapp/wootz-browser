import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "burnBatch" function.
 */
export type BurnBatchParams = WithOverrides<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
    ids: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "ids";
    }>;
    values: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "values";
    }>;
}>;
export declare const FN_SELECTOR: "0x6b20c454";
/**
 * Checks if the `burnBatch` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `burnBatch` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isBurnBatchSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isBurnBatchSupported(["0x..."]);
 * ```
 */
export declare function isBurnBatchSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "burnBatch" function.
 * @param options - The options for the burnBatch function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBurnBatchParams } from "thirdweb/extensions/erc1155";
 * const result = encodeBurnBatchParams({
 *  account: ...,
 *  ids: ...,
 *  values: ...,
 * });
 * ```
 */
export declare function encodeBurnBatchParams(options: BurnBatchParams): `0x${string}`;
/**
 * Encodes the "burnBatch" function into a Hex string with its parameters.
 * @param options - The options for the burnBatch function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBurnBatch } from "thirdweb/extensions/erc1155";
 * const result = encodeBurnBatch({
 *  account: ...,
 *  ids: ...,
 *  values: ...,
 * });
 * ```
 */
export declare function encodeBurnBatch(options: BurnBatchParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "burnBatch" function on the contract.
 * @param options - The options for the "burnBatch" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { burnBatch } from "thirdweb/extensions/erc1155";
 *
 * const transaction = burnBatch({
 *  contract,
 *  account: ...,
 *  ids: ...,
 *  values: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function burnBatch(options: BaseTransactionOptions<BurnBatchParams | {
    asyncParams: () => Promise<BurnBatchParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=burnBatch.d.ts.map