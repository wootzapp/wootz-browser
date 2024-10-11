import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "addStake" function.
 */
export type AddStakeParams = WithOverrides<{
    unstakeDelaySec: AbiParameterToPrimitiveType<{
        type: "uint32";
        name: "_unstakeDelaySec";
    }>;
}>;
export declare const FN_SELECTOR: "0x0396cb60";
/**
 * Checks if the `addStake` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `addStake` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isAddStakeSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isAddStakeSupported(["0x..."]);
 * ```
 */
export declare function isAddStakeSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "addStake" function.
 * @param options - The options for the addStake function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeAddStakeParams } from "thirdweb/extensions/erc4337";
 * const result = encodeAddStakeParams({
 *  unstakeDelaySec: ...,
 * });
 * ```
 */
export declare function encodeAddStakeParams(options: AddStakeParams): `0x${string}`;
/**
 * Encodes the "addStake" function into a Hex string with its parameters.
 * @param options - The options for the addStake function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeAddStake } from "thirdweb/extensions/erc4337";
 * const result = encodeAddStake({
 *  unstakeDelaySec: ...,
 * });
 * ```
 */
export declare function encodeAddStake(options: AddStakeParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "addStake" function on the contract.
 * @param options - The options for the "addStake" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { addStake } from "thirdweb/extensions/erc4337";
 *
 * const transaction = addStake({
 *  contract,
 *  unstakeDelaySec: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function addStake(options: BaseTransactionOptions<AddStakeParams | {
    asyncParams: () => Promise<AddStakeParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=addStake.d.ts.map