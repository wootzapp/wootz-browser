import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "delegateBySig" function.
 */
export type DelegateBySigParams = WithOverrides<{
    delegatee: AbiParameterToPrimitiveType<{
        type: "address";
        name: "delegatee";
    }>;
    nonce: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "nonce";
    }>;
    expiry: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "expiry";
    }>;
    v: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "v";
    }>;
    r: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "r";
    }>;
    s: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "s";
    }>;
}>;
export declare const FN_SELECTOR: "0xc3cda520";
/**
 * Checks if the `delegateBySig` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `delegateBySig` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isDelegateBySigSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isDelegateBySigSupported(["0x..."]);
 * ```
 */
export declare function isDelegateBySigSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "delegateBySig" function.
 * @param options - The options for the delegateBySig function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeDelegateBySigParams } from "thirdweb/extensions/erc20";
 * const result = encodeDelegateBySigParams({
 *  delegatee: ...,
 *  nonce: ...,
 *  expiry: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodeDelegateBySigParams(options: DelegateBySigParams): `0x${string}`;
/**
 * Encodes the "delegateBySig" function into a Hex string with its parameters.
 * @param options - The options for the delegateBySig function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeDelegateBySig } from "thirdweb/extensions/erc20";
 * const result = encodeDelegateBySig({
 *  delegatee: ...,
 *  nonce: ...,
 *  expiry: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodeDelegateBySig(options: DelegateBySigParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "delegateBySig" function on the contract.
 * @param options - The options for the "delegateBySig" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { delegateBySig } from "thirdweb/extensions/erc20";
 *
 * const transaction = delegateBySig({
 *  contract,
 *  delegatee: ...,
 *  nonce: ...,
 *  expiry: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function delegateBySig(options: BaseTransactionOptions<DelegateBySigParams | {
    asyncParams: () => Promise<DelegateBySigParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=delegateBySig.d.ts.map