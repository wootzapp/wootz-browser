import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "depositRewardTokens" function.
 */
export type DepositRewardTokensParams = WithOverrides<{
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_amount";
    }>;
}>;
export declare const FN_SELECTOR: "0x16c621e0";
/**
 * Checks if the `depositRewardTokens` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `depositRewardTokens` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isDepositRewardTokensSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isDepositRewardTokensSupported(["0x..."]);
 * ```
 */
export declare function isDepositRewardTokensSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "depositRewardTokens" function.
 * @param options - The options for the depositRewardTokens function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeDepositRewardTokensParams } from "thirdweb/extensions/erc20";
 * const result = encodeDepositRewardTokensParams({
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeDepositRewardTokensParams(options: DepositRewardTokensParams): `0x${string}`;
/**
 * Encodes the "depositRewardTokens" function into a Hex string with its parameters.
 * @param options - The options for the depositRewardTokens function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeDepositRewardTokens } from "thirdweb/extensions/erc20";
 * const result = encodeDepositRewardTokens({
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeDepositRewardTokens(options: DepositRewardTokensParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "depositRewardTokens" function on the contract.
 * @param options - The options for the "depositRewardTokens" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { depositRewardTokens } from "thirdweb/extensions/erc20";
 *
 * const transaction = depositRewardTokens({
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
export declare function depositRewardTokens(options: BaseTransactionOptions<DepositRewardTokensParams | {
    asyncParams: () => Promise<DepositRewardTokensParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=depositRewardTokens.d.ts.map