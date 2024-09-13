import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setRoyaltyInfoForToken" function.
 */
export type SetRoyaltyInfoForTokenParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    recipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recipient";
    }>;
    bps: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "bps";
    }>;
}>;
export declare const FN_SELECTOR: "0x9bcf7a15";
/**
 * Checks if the `setRoyaltyInfoForToken` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setRoyaltyInfoForToken` method is supported.
 * @extension COMMON
 * @example
 * ```ts
 * import { isSetRoyaltyInfoForTokenSupported } from "thirdweb/extensions/common";
 *
 * const supported = isSetRoyaltyInfoForTokenSupported(["0x..."]);
 * ```
 */
export declare function isSetRoyaltyInfoForTokenSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setRoyaltyInfoForToken" function.
 * @param options - The options for the setRoyaltyInfoForToken function.
 * @returns The encoded ABI parameters.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetRoyaltyInfoForTokenParams } from "thirdweb/extensions/common";
 * const result = encodeSetRoyaltyInfoForTokenParams({
 *  tokenId: ...,
 *  recipient: ...,
 *  bps: ...,
 * });
 * ```
 */
export declare function encodeSetRoyaltyInfoForTokenParams(options: SetRoyaltyInfoForTokenParams): `0x${string}`;
/**
 * Encodes the "setRoyaltyInfoForToken" function into a Hex string with its parameters.
 * @param options - The options for the setRoyaltyInfoForToken function.
 * @returns The encoded hexadecimal string.
 * @extension COMMON
 * @example
 * ```ts
 * import { encodeSetRoyaltyInfoForToken } from "thirdweb/extensions/common";
 * const result = encodeSetRoyaltyInfoForToken({
 *  tokenId: ...,
 *  recipient: ...,
 *  bps: ...,
 * });
 * ```
 */
export declare function encodeSetRoyaltyInfoForToken(options: SetRoyaltyInfoForTokenParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setRoyaltyInfoForToken" function on the contract.
 * @param options - The options for the "setRoyaltyInfoForToken" function.
 * @returns A prepared transaction object.
 * @extension COMMON
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setRoyaltyInfoForToken } from "thirdweb/extensions/common";
 *
 * const transaction = setRoyaltyInfoForToken({
 *  contract,
 *  tokenId: ...,
 *  recipient: ...,
 *  bps: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setRoyaltyInfoForToken(options: BaseTransactionOptions<SetRoyaltyInfoForTokenParams | {
    asyncParams: () => Promise<SetRoyaltyInfoForTokenParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setRoyaltyInfoForToken.d.ts.map