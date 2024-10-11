import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "airdropNativeToken" function.
 */
export type AirdropNativeTokenParams = WithOverrides<{
    contents: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "_contents";
        components: [
            {
                type: "address";
                name: "recipient";
            },
            {
                type: "uint256";
                name: "amount";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x0d5818f7";
/**
 * Checks if the `airdropNativeToken` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `airdropNativeToken` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isAirdropNativeTokenSupported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isAirdropNativeTokenSupported(["0x..."]);
 * ```
 */
export declare function isAirdropNativeTokenSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "airdropNativeToken" function.
 * @param options - The options for the airdropNativeToken function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropNativeTokenParams } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropNativeTokenParams({
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropNativeTokenParams(options: AirdropNativeTokenParams): `0x${string}`;
/**
 * Encodes the "airdropNativeToken" function into a Hex string with its parameters.
 * @param options - The options for the airdropNativeToken function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropNativeToken } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropNativeToken({
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropNativeToken(options: AirdropNativeTokenParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "airdropNativeToken" function on the contract.
 * @param options - The options for the "airdropNativeToken" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { airdropNativeToken } from "thirdweb/extensions/airdrop";
 *
 * const transaction = airdropNativeToken({
 *  contract,
 *  contents: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function airdropNativeToken(options: BaseTransactionOptions<AirdropNativeTokenParams | {
    asyncParams: () => Promise<AirdropNativeTokenParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=airdropNativeToken.d.ts.map