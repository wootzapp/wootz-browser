import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "withdrawRewardTokens" function.
 */
export type WithdrawRewardTokensParams = WithOverrides<{
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_amount";
    }>;
}>;
export declare const FN_SELECTOR: "0xcb43b2dd";
/**
 * Checks if the `withdrawRewardTokens` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `withdrawRewardTokens` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isWithdrawRewardTokensSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isWithdrawRewardTokensSupported(["0x..."]);
 * ```
 */
export declare function isWithdrawRewardTokensSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "withdrawRewardTokens" function.
 * @param options - The options for the withdrawRewardTokens function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeWithdrawRewardTokensParams } from "thirdweb/extensions/erc1155";
 * const result = encodeWithdrawRewardTokensParams({
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeWithdrawRewardTokensParams(options: WithdrawRewardTokensParams): `0x${string}`;
/**
 * Encodes the "withdrawRewardTokens" function into a Hex string with its parameters.
 * @param options - The options for the withdrawRewardTokens function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeWithdrawRewardTokens } from "thirdweb/extensions/erc1155";
 * const result = encodeWithdrawRewardTokens({
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeWithdrawRewardTokens(options: WithdrawRewardTokensParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "withdrawRewardTokens" function on the contract.
 * @param options - The options for the "withdrawRewardTokens" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { withdrawRewardTokens } from "thirdweb/extensions/erc1155";
 *
 * const transaction = withdrawRewardTokens({
 *  contract,
 *  amount: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function withdrawRewardTokens(options: BaseTransactionOptions<WithdrawRewardTokensParams | {
    asyncParams: () => Promise<WithdrawRewardTokensParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=withdrawRewardTokens.d.ts.map