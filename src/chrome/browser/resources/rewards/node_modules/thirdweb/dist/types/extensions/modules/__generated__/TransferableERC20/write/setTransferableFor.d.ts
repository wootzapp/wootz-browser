import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setTransferableFor" function.
 */
export type SetTransferableForParams = WithOverrides<{
    target: AbiParameterToPrimitiveType<{
        type: "address";
        name: "target";
    }>;
    enableTransfer: AbiParameterToPrimitiveType<{
        type: "bool";
        name: "enableTransfer";
    }>;
}>;
export declare const FN_SELECTOR: "0x4c297cbd";
/**
 * Checks if the `setTransferableFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setTransferableFor` method is supported.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 *
 * const supported = TransferableERC20.isSetTransferableForSupported(["0x..."]);
 * ```
 */
export declare function isSetTransferableForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setTransferableFor" function.
 * @param options - The options for the setTransferableFor function.
 * @returns The encoded ABI parameters.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.encodeSetTransferableForParams({
 *  target: ...,
 *  enableTransfer: ...,
 * });
 * ```
 */
export declare function encodeSetTransferableForParams(options: SetTransferableForParams): `0x${string}`;
/**
 * Encodes the "setTransferableFor" function into a Hex string with its parameters.
 * @param options - The options for the setTransferableFor function.
 * @returns The encoded hexadecimal string.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.encodeSetTransferableFor({
 *  target: ...,
 *  enableTransfer: ...,
 * });
 * ```
 */
export declare function encodeSetTransferableFor(options: SetTransferableForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setTransferableFor" function on the contract.
 * @param options - The options for the "setTransferableFor" function.
 * @returns A prepared transaction object.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { TransferableERC20 } from "thirdweb/modules";
 *
 * const transaction = TransferableERC20.setTransferableFor({
 *  contract,
 *  target: ...,
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
export declare function setTransferableFor(options: BaseTransactionOptions<SetTransferableForParams | {
    asyncParams: () => Promise<SetTransferableForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setTransferableFor.d.ts.map