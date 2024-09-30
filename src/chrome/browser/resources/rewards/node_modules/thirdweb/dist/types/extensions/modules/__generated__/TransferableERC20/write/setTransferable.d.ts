import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setTransferable" function.
 */
export type SetTransferableParams = WithOverrides<{
    enableTransfer: AbiParameterToPrimitiveType<{
        type: "bool";
        name: "enableTransfer";
    }>;
}>;
export declare const FN_SELECTOR: "0x9cd23707";
/**
 * Checks if the `setTransferable` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setTransferable` method is supported.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 *
 * const supported = TransferableERC20.isSetTransferableSupported(["0x..."]);
 * ```
 */
export declare function isSetTransferableSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setTransferable" function.
 * @param options - The options for the setTransferable function.
 * @returns The encoded ABI parameters.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.encodeSetTransferableParams({
 *  enableTransfer: ...,
 * });
 * ```
 */
export declare function encodeSetTransferableParams(options: SetTransferableParams): `0x${string}`;
/**
 * Encodes the "setTransferable" function into a Hex string with its parameters.
 * @param options - The options for the setTransferable function.
 * @returns The encoded hexadecimal string.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.encodeSetTransferable({
 *  enableTransfer: ...,
 * });
 * ```
 */
export declare function encodeSetTransferable(options: SetTransferableParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setTransferable" function on the contract.
 * @param options - The options for the "setTransferable" function.
 * @returns A prepared transaction object.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { TransferableERC20 } from "thirdweb/modules";
 *
 * const transaction = TransferableERC20.setTransferable({
 *  contract,
 *  enableTransfer: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setTransferable(options: BaseTransactionOptions<SetTransferableParams | {
    asyncParams: () => Promise<SetTransferableParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setTransferable.d.ts.map