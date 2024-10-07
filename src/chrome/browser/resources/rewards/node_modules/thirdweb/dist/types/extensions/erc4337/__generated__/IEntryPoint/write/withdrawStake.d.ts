import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "withdrawStake" function.
 */
export type WithdrawStakeParams = WithOverrides<{
    withdrawAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "withdrawAddress";
    }>;
}>;
export declare const FN_SELECTOR: "0xc23a5cea";
/**
 * Checks if the `withdrawStake` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `withdrawStake` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isWithdrawStakeSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isWithdrawStakeSupported(["0x..."]);
 * ```
 */
export declare function isWithdrawStakeSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "withdrawStake" function.
 * @param options - The options for the withdrawStake function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeWithdrawStakeParams } from "thirdweb/extensions/erc4337";
 * const result = encodeWithdrawStakeParams({
 *  withdrawAddress: ...,
 * });
 * ```
 */
export declare function encodeWithdrawStakeParams(options: WithdrawStakeParams): `0x${string}`;
/**
 * Encodes the "withdrawStake" function into a Hex string with its parameters.
 * @param options - The options for the withdrawStake function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeWithdrawStake } from "thirdweb/extensions/erc4337";
 * const result = encodeWithdrawStake({
 *  withdrawAddress: ...,
 * });
 * ```
 */
export declare function encodeWithdrawStake(options: WithdrawStakeParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "withdrawStake" function on the contract.
 * @param options - The options for the "withdrawStake" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { withdrawStake } from "thirdweb/extensions/erc4337";
 *
 * const transaction = withdrawStake({
 *  contract,
 *  withdrawAddress: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function withdrawStake(options: BaseTransactionOptions<WithdrawStakeParams | {
    asyncParams: () => Promise<WithdrawStakeParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=withdrawStake.d.ts.map