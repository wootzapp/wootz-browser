import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "relay" function.
 */
export type RelayParams = WithOverrides<{
    target: AbiParameterToPrimitiveType<{
        type: "address";
        name: "target";
    }>;
    value: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "value";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0xc28bc2fa";
/**
 * Checks if the `relay` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `relay` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isRelaySupported } from "thirdweb/extensions/vote";
 *
 * const supported = isRelaySupported(["0x..."]);
 * ```
 */
export declare function isRelaySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "relay" function.
 * @param options - The options for the relay function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeRelayParams } from "thirdweb/extensions/vote";
 * const result = encodeRelayParams({
 *  target: ...,
 *  value: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeRelayParams(options: RelayParams): `0x${string}`;
/**
 * Encodes the "relay" function into a Hex string with its parameters.
 * @param options - The options for the relay function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeRelay } from "thirdweb/extensions/vote";
 * const result = encodeRelay({
 *  target: ...,
 *  value: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeRelay(options: RelayParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "relay" function on the contract.
 * @param options - The options for the "relay" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { relay } from "thirdweb/extensions/vote";
 *
 * const transaction = relay({
 *  contract,
 *  target: ...,
 *  value: ...,
 *  data: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function relay(options: BaseTransactionOptions<RelayParams | {
    asyncParams: () => Promise<RelayParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=relay.d.ts.map