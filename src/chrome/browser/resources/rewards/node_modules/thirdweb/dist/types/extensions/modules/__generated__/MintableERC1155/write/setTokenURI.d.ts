import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setTokenURI" function.
 */
export type SetTokenURIParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
    tokenURI: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_tokenURI";
    }>;
}>;
export declare const FN_SELECTOR: "0x162094c4";
/**
 * Checks if the `setTokenURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setTokenURI` method is supported.
 * @modules MintableERC1155
 * @example
 * ```ts
 * import { MintableERC1155 } from "thirdweb/modules";
 *
 * const supported = MintableERC1155.isSetTokenURISupported(["0x..."]);
 * ```
 */
export declare function isSetTokenURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setTokenURI" function.
 * @param options - The options for the setTokenURI function.
 * @returns The encoded ABI parameters.
 * @modules MintableERC1155
 * @example
 * ```ts
 * import { MintableERC1155 } from "thirdweb/modules";
 * const result = MintableERC1155.encodeSetTokenURIParams({
 *  tokenId: ...,
 *  tokenURI: ...,
 * });
 * ```
 */
export declare function encodeSetTokenURIParams(options: SetTokenURIParams): `0x${string}`;
/**
 * Encodes the "setTokenURI" function into a Hex string with its parameters.
 * @param options - The options for the setTokenURI function.
 * @returns The encoded hexadecimal string.
 * @modules MintableERC1155
 * @example
 * ```ts
 * import { MintableERC1155 } from "thirdweb/modules";
 * const result = MintableERC1155.encodeSetTokenURI({
 *  tokenId: ...,
 *  tokenURI: ...,
 * });
 * ```
 */
export declare function encodeSetTokenURI(options: SetTokenURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setTokenURI" function on the contract.
 * @param options - The options for the "setTokenURI" function.
 * @returns A prepared transaction object.
 * @modules MintableERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { MintableERC1155 } from "thirdweb/modules";
 *
 * const transaction = MintableERC1155.setTokenURI({
 *  contract,
 *  tokenId: ...,
 *  tokenURI: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setTokenURI(options: BaseTransactionOptions<SetTokenURIParams | {
    asyncParams: () => Promise<SetTokenURIParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setTokenURI.d.ts.map