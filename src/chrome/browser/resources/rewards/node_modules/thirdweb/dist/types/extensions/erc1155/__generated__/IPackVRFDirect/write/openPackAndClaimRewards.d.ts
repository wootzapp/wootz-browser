import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "openPackAndClaimRewards" function.
 */
export type OpenPackAndClaimRewardsParams = WithOverrides<{
    packId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_packId";
    }>;
    amountToOpen: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_amountToOpen";
    }>;
    callBackGasLimit: AbiParameterToPrimitiveType<{
        type: "uint32";
        name: "_callBackGasLimit";
    }>;
}>;
export declare const FN_SELECTOR: "0xac296b3f";
/**
 * Checks if the `openPackAndClaimRewards` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `openPackAndClaimRewards` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isOpenPackAndClaimRewardsSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isOpenPackAndClaimRewardsSupported(["0x..."]);
 * ```
 */
export declare function isOpenPackAndClaimRewardsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "openPackAndClaimRewards" function.
 * @param options - The options for the openPackAndClaimRewards function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeOpenPackAndClaimRewardsParams } from "thirdweb/extensions/erc1155";
 * const result = encodeOpenPackAndClaimRewardsParams({
 *  packId: ...,
 *  amountToOpen: ...,
 *  callBackGasLimit: ...,
 * });
 * ```
 */
export declare function encodeOpenPackAndClaimRewardsParams(options: OpenPackAndClaimRewardsParams): `0x${string}`;
/**
 * Encodes the "openPackAndClaimRewards" function into a Hex string with its parameters.
 * @param options - The options for the openPackAndClaimRewards function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeOpenPackAndClaimRewards } from "thirdweb/extensions/erc1155";
 * const result = encodeOpenPackAndClaimRewards({
 *  packId: ...,
 *  amountToOpen: ...,
 *  callBackGasLimit: ...,
 * });
 * ```
 */
export declare function encodeOpenPackAndClaimRewards(options: OpenPackAndClaimRewardsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "openPackAndClaimRewards" function on the contract.
 * @param options - The options for the "openPackAndClaimRewards" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { openPackAndClaimRewards } from "thirdweb/extensions/erc1155";
 *
 * const transaction = openPackAndClaimRewards({
 *  contract,
 *  packId: ...,
 *  amountToOpen: ...,
 *  callBackGasLimit: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function openPackAndClaimRewards(options: BaseTransactionOptions<OpenPackAndClaimRewardsParams | {
    asyncParams: () => Promise<OpenPackAndClaimRewardsParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=openPackAndClaimRewards.d.ts.map