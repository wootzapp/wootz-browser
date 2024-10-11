import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "redeem" function.
 */
export type RedeemParams = WithOverrides<{
    shares: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "shares";
    }>;
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
    }>;
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
}>;
export declare const FN_SELECTOR: "0xba087652";
/**
 * Checks if the `redeem` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `redeem` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isRedeemSupported } from "thirdweb/extensions/erc4626";
 *
 * const supported = isRedeemSupported(["0x..."]);
 * ```
 */
export declare function isRedeemSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "redeem" function.
 * @param options - The options for the redeem function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeRedeemParams } from "thirdweb/extensions/erc4626";
 * const result = encodeRedeemParams({
 *  shares: ...,
 *  receiver: ...,
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeRedeemParams(options: RedeemParams): `0x${string}`;
/**
 * Encodes the "redeem" function into a Hex string with its parameters.
 * @param options - The options for the redeem function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeRedeem } from "thirdweb/extensions/erc4626";
 * const result = encodeRedeem({
 *  shares: ...,
 *  receiver: ...,
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeRedeem(options: RedeemParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "redeem" function on the contract.
 * @param options - The options for the "redeem" function.
 * @returns A prepared transaction object.
 * @extension ERC4626
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { redeem } from "thirdweb/extensions/erc4626";
 *
 * const transaction = redeem({
 *  contract,
 *  shares: ...,
 *  receiver: ...,
 *  owner: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function redeem(options: BaseTransactionOptions<RedeemParams | {
    asyncParams: () => Promise<RedeemParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=redeem.d.ts.map