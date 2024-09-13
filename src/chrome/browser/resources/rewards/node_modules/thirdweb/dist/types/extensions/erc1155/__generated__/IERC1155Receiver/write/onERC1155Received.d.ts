import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "onERC1155Received" function.
 */
export type OnERC1155ReceivedParams = WithOverrides<{
    operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "operator";
    }>;
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    id: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "id";
    }>;
    value: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "value";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0xf23a6e61";
/**
 * Checks if the `onERC1155Received` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `onERC1155Received` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isOnERC1155ReceivedSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isOnERC1155ReceivedSupported(["0x..."]);
 * ```
 */
export declare function isOnERC1155ReceivedSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "onERC1155Received" function.
 * @param options - The options for the onERC1155Received function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeOnERC1155ReceivedParams } from "thirdweb/extensions/erc1155";
 * const result = encodeOnERC1155ReceivedParams({
 *  operator: ...,
 *  from: ...,
 *  id: ...,
 *  value: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnERC1155ReceivedParams(options: OnERC1155ReceivedParams): `0x${string}`;
/**
 * Encodes the "onERC1155Received" function into a Hex string with its parameters.
 * @param options - The options for the onERC1155Received function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeOnERC1155Received } from "thirdweb/extensions/erc1155";
 * const result = encodeOnERC1155Received({
 *  operator: ...,
 *  from: ...,
 *  id: ...,
 *  value: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnERC1155Received(options: OnERC1155ReceivedParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "onERC1155Received" function on the contract.
 * @param options - The options for the "onERC1155Received" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { onERC1155Received } from "thirdweb/extensions/erc1155";
 *
 * const transaction = onERC1155Received({
 *  contract,
 *  operator: ...,
 *  from: ...,
 *  id: ...,
 *  value: ...,
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
export declare function onERC1155Received(options: BaseTransactionOptions<OnERC1155ReceivedParams | {
    asyncParams: () => Promise<OnERC1155ReceivedParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=onERC1155Received.d.ts.map