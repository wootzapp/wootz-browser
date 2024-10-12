import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setSaleConfig" function.
 */
export type SetSaleConfigParams = WithOverrides<{
    primarySaleRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_primarySaleRecipient";
    }>;
}>;
export declare const FN_SELECTOR: "0xd29a3628";
/**
 * Checks if the `setSaleConfig` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setSaleConfig` method is supported.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { ClaimableERC20 } from "thirdweb/modules";
 *
 * const supported = ClaimableERC20.isSetSaleConfigSupported(["0x..."]);
 * ```
 */
export declare function isSetSaleConfigSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setSaleConfig" function.
 * @param options - The options for the setSaleConfig function.
 * @returns The encoded ABI parameters.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { ClaimableERC20 } from "thirdweb/modules";
 * const result = ClaimableERC20.encodeSetSaleConfigParams({
 *  primarySaleRecipient: ...,
 * });
 * ```
 */
export declare function encodeSetSaleConfigParams(options: SetSaleConfigParams): `0x${string}`;
/**
 * Encodes the "setSaleConfig" function into a Hex string with its parameters.
 * @param options - The options for the setSaleConfig function.
 * @returns The encoded hexadecimal string.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { ClaimableERC20 } from "thirdweb/modules";
 * const result = ClaimableERC20.encodeSetSaleConfig({
 *  primarySaleRecipient: ...,
 * });
 * ```
 */
export declare function encodeSetSaleConfig(options: SetSaleConfigParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setSaleConfig" function on the contract.
 * @param options - The options for the "setSaleConfig" function.
 * @returns A prepared transaction object.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { ClaimableERC20 } from "thirdweb/modules";
 *
 * const transaction = ClaimableERC20.setSaleConfig({
 *  contract,
 *  primarySaleRecipient: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setSaleConfig(options: BaseTransactionOptions<SetSaleConfigParams | {
    asyncParams: () => Promise<SetSaleConfigParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setSaleConfig.d.ts.map