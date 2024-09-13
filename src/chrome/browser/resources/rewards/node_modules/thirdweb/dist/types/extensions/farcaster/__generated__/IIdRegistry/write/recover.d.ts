import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "recover" function.
 */
export type RecoverParams = WithOverrides<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    deadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "deadline";
    }>;
    sig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "sig";
    }>;
}>;
export declare const FN_SELECTOR: "0x2a42ede3";
/**
 * Checks if the `recover` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `recover` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRecoverSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isRecoverSupported(["0x..."]);
 * ```
 */
export declare function isRecoverSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "recover" function.
 * @param options - The options for the recover function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRecoverParams } from "thirdweb/extensions/farcaster";
 * const result = encodeRecoverParams({
 *  from: ...,
 *  to: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeRecoverParams(options: RecoverParams): `0x${string}`;
/**
 * Encodes the "recover" function into a Hex string with its parameters.
 * @param options - The options for the recover function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRecover } from "thirdweb/extensions/farcaster";
 * const result = encodeRecover({
 *  from: ...,
 *  to: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeRecover(options: RecoverParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "recover" function on the contract.
 * @param options - The options for the "recover" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { recover } from "thirdweb/extensions/farcaster";
 *
 * const transaction = recover({
 *  contract,
 *  from: ...,
 *  to: ...,
 *  deadline: ...,
 *  sig: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function recover(options: BaseTransactionOptions<RecoverParams | {
    asyncParams: () => Promise<RecoverParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=recover.d.ts.map