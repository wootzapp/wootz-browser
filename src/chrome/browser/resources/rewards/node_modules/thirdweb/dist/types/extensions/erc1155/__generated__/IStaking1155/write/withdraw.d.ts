import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "withdraw" function.
 */
export type WithdrawParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    amount: AbiParameterToPrimitiveType<{
        type: "uint64";
        name: "amount";
    }>;
}>;
export declare const FN_SELECTOR: "0xc434dcfe";
/**
 * Checks if the `withdraw` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `withdraw` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isWithdrawSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isWithdrawSupported(["0x..."]);
 * ```
 */
export declare function isWithdrawSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "withdraw" function.
 * @param options - The options for the withdraw function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeWithdrawParams } from "thirdweb/extensions/erc1155";
 * const result = encodeWithdrawParams({
 *  tokenId: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeWithdrawParams(options: WithdrawParams): `0x${string}`;
/**
 * Encodes the "withdraw" function into a Hex string with its parameters.
 * @param options - The options for the withdraw function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeWithdraw } from "thirdweb/extensions/erc1155";
 * const result = encodeWithdraw({
 *  tokenId: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeWithdraw(options: WithdrawParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "withdraw" function on the contract.
 * @param options - The options for the "withdraw" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { withdraw } from "thirdweb/extensions/erc1155";
 *
 * const transaction = withdraw({
 *  contract,
 *  tokenId: ...,
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
export declare function withdraw(options: BaseTransactionOptions<WithdrawParams | {
    asyncParams: () => Promise<WithdrawParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=withdraw.d.ts.map