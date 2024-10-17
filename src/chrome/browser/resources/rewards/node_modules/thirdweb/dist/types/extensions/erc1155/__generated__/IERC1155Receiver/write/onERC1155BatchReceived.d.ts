import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "onERC1155BatchReceived" function.
 */
export type OnERC1155BatchReceivedParams = WithOverrides<{
    operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "operator";
    }>;
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    ids: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "ids";
    }>;
    values: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "values";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0xbc197c81";
/**
 * Checks if the `onERC1155BatchReceived` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `onERC1155BatchReceived` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isOnERC1155BatchReceivedSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isOnERC1155BatchReceivedSupported(["0x..."]);
 * ```
 */
export declare function isOnERC1155BatchReceivedSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "onERC1155BatchReceived" function.
 * @param options - The options for the onERC1155BatchReceived function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeOnERC1155BatchReceivedParams } from "thirdweb/extensions/erc1155";
 * const result = encodeOnERC1155BatchReceivedParams({
 *  operator: ...,
 *  from: ...,
 *  ids: ...,
 *  values: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnERC1155BatchReceivedParams(options: OnERC1155BatchReceivedParams): `0x${string}`;
/**
 * Encodes the "onERC1155BatchReceived" function into a Hex string with its parameters.
 * @param options - The options for the onERC1155BatchReceived function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeOnERC1155BatchReceived } from "thirdweb/extensions/erc1155";
 * const result = encodeOnERC1155BatchReceived({
 *  operator: ...,
 *  from: ...,
 *  ids: ...,
 *  values: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeOnERC1155BatchReceived(options: OnERC1155BatchReceivedParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "onERC1155BatchReceived" function on the contract.
 * @param options - The options for the "onERC1155BatchReceived" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { onERC1155BatchReceived } from "thirdweb/extensions/erc1155";
 *
 * const transaction = onERC1155BatchReceived({
 *  contract,
 *  operator: ...,
 *  from: ...,
 *  ids: ...,
 *  values: ...,
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
export declare function onERC1155BatchReceived(options: BaseTransactionOptions<OnERC1155BatchReceivedParams | {
    asyncParams: () => Promise<OnERC1155BatchReceivedParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=onERC1155BatchReceived.d.ts.map