import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "stake" function.
 */
export type StakeParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    amount: AbiParameterToPrimitiveType<{
        type: "uint64";
        name: "amount";
    }>;
}>;
export declare const FN_SELECTOR: "0x952e68cf";
/**
 * Checks if the `stake` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `stake` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isStakeSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isStakeSupported(["0x..."]);
 * ```
 */
export declare function isStakeSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "stake" function.
 * @param options - The options for the stake function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeStakeParams } from "thirdweb/extensions/erc1155";
 * const result = encodeStakeParams({
 *  tokenId: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeStakeParams(options: StakeParams): `0x${string}`;
/**
 * Encodes the "stake" function into a Hex string with its parameters.
 * @param options - The options for the stake function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeStake } from "thirdweb/extensions/erc1155";
 * const result = encodeStake({
 *  tokenId: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeStake(options: StakeParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "stake" function on the contract.
 * @param options - The options for the "stake" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { stake } from "thirdweb/extensions/erc1155";
 *
 * const transaction = stake({
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
export declare function stake(options: BaseTransactionOptions<StakeParams | {
    asyncParams: () => Promise<StakeParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=stake.d.ts.map