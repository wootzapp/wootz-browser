import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setMaxTotalSupply" function.
 */
export type SetMaxTotalSupplyParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
    maxTotalSupply: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_maxTotalSupply";
    }>;
}>;
export declare const FN_SELECTOR: "0x87198cf2";
/**
 * Checks if the `setMaxTotalSupply` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setMaxTotalSupply` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isSetMaxTotalSupplySupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isSetMaxTotalSupplySupported(["0x..."]);
 * ```
 */
export declare function isSetMaxTotalSupplySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setMaxTotalSupply" function.
 * @param options - The options for the setMaxTotalSupply function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSetMaxTotalSupplyParams } from "thirdweb/extensions/erc1155";
 * const result = encodeSetMaxTotalSupplyParams({
 *  tokenId: ...,
 *  maxTotalSupply: ...,
 * });
 * ```
 */
export declare function encodeSetMaxTotalSupplyParams(options: SetMaxTotalSupplyParams): `0x${string}`;
/**
 * Encodes the "setMaxTotalSupply" function into a Hex string with its parameters.
 * @param options - The options for the setMaxTotalSupply function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeSetMaxTotalSupply } from "thirdweb/extensions/erc1155";
 * const result = encodeSetMaxTotalSupply({
 *  tokenId: ...,
 *  maxTotalSupply: ...,
 * });
 * ```
 */
export declare function encodeSetMaxTotalSupply(options: SetMaxTotalSupplyParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setMaxTotalSupply" function on the contract.
 * @param options - The options for the "setMaxTotalSupply" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setMaxTotalSupply } from "thirdweb/extensions/erc1155";
 *
 * const transaction = setMaxTotalSupply({
 *  contract,
 *  tokenId: ...,
 *  maxTotalSupply: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setMaxTotalSupply(options: BaseTransactionOptions<SetMaxTotalSupplyParams | {
    asyncParams: () => Promise<SetMaxTotalSupplyParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setMaxTotalSupply.d.ts.map