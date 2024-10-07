import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "registerFor" function.
 */
export type RegisterForParams = WithOverrides<{
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    recovery: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recovery";
    }>;
    deadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "deadline";
    }>;
    sig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "sig";
    }>;
    extraStorage: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "extraStorage";
    }>;
}>;
export declare const FN_SELECTOR: "0xa0c7529c";
/**
 * Checks if the `registerFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `registerFor` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRegisterForSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isRegisterForSupported(["0x..."]);
 * ```
 */
export declare function isRegisterForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "registerFor" function.
 * @param options - The options for the registerFor function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRegisterForParams } from "thirdweb/extensions/farcaster";
 * const result = encodeRegisterForParams({
 *  to: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 *  extraStorage: ...,
 * });
 * ```
 */
export declare function encodeRegisterForParams(options: RegisterForParams): `0x${string}`;
/**
 * Encodes the "registerFor" function into a Hex string with its parameters.
 * @param options - The options for the registerFor function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRegisterFor } from "thirdweb/extensions/farcaster";
 * const result = encodeRegisterFor({
 *  to: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 *  extraStorage: ...,
 * });
 * ```
 */
export declare function encodeRegisterFor(options: RegisterForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "registerFor" function on the contract.
 * @param options - The options for the "registerFor" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { registerFor } from "thirdweb/extensions/farcaster";
 *
 * const transaction = registerFor({
 *  contract,
 *  to: ...,
 *  recovery: ...,
 *  deadline: ...,
 *  sig: ...,
 *  extraStorage: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function registerFor(options: BaseTransactionOptions<RegisterForParams | {
    asyncParams: () => Promise<RegisterForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=registerFor.d.ts.map