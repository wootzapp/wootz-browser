import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "getSenderAddress" function.
 */
export type GetSenderAddressParams = WithOverrides<{
    initCode: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "initCode";
    }>;
}>;
export declare const FN_SELECTOR: "0x9b249f69";
/**
 * Checks if the `getSenderAddress` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getSenderAddress` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isGetSenderAddressSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isGetSenderAddressSupported(["0x..."]);
 * ```
 */
export declare function isGetSenderAddressSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getSenderAddress" function.
 * @param options - The options for the getSenderAddress function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetSenderAddressParams } from "thirdweb/extensions/erc4337";
 * const result = encodeGetSenderAddressParams({
 *  initCode: ...,
 * });
 * ```
 */
export declare function encodeGetSenderAddressParams(options: GetSenderAddressParams): `0x${string}`;
/**
 * Encodes the "getSenderAddress" function into a Hex string with its parameters.
 * @param options - The options for the getSenderAddress function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetSenderAddress } from "thirdweb/extensions/erc4337";
 * const result = encodeGetSenderAddress({
 *  initCode: ...,
 * });
 * ```
 */
export declare function encodeGetSenderAddress(options: GetSenderAddressParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "getSenderAddress" function on the contract.
 * @param options - The options for the "getSenderAddress" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { getSenderAddress } from "thirdweb/extensions/erc4337";
 *
 * const transaction = getSenderAddress({
 *  contract,
 *  initCode: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function getSenderAddress(options: BaseTransactionOptions<GetSenderAddressParams | {
    asyncParams: () => Promise<GetSenderAddressParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=getSenderAddress.d.ts.map