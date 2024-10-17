import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setRulesEngineOverride" function.
 */
export type SetRulesEngineOverrideParams = WithOverrides<{
    rulesEngineAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_rulesEngineAddress";
    }>;
}>;
export declare const FN_SELECTOR: "0x0eb0adb6";
/**
 * Checks if the `setRulesEngineOverride` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setRulesEngineOverride` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isSetRulesEngineOverrideSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isSetRulesEngineOverrideSupported(["0x..."]);
 * ```
 */
export declare function isSetRulesEngineOverrideSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setRulesEngineOverride" function.
 * @param options - The options for the setRulesEngineOverride function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetRulesEngineOverrideParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetRulesEngineOverrideParams({
 *  rulesEngineAddress: ...,
 * });
 * ```
 */
export declare function encodeSetRulesEngineOverrideParams(options: SetRulesEngineOverrideParams): `0x${string}`;
/**
 * Encodes the "setRulesEngineOverride" function into a Hex string with its parameters.
 * @param options - The options for the setRulesEngineOverride function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeSetRulesEngineOverride } from "thirdweb/extensions/thirdweb";
 * const result = encodeSetRulesEngineOverride({
 *  rulesEngineAddress: ...,
 * });
 * ```
 */
export declare function encodeSetRulesEngineOverride(options: SetRulesEngineOverrideParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setRulesEngineOverride" function on the contract.
 * @param options - The options for the "setRulesEngineOverride" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setRulesEngineOverride } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = setRulesEngineOverride({
 *  contract,
 *  rulesEngineAddress: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setRulesEngineOverride(options: BaseTransactionOptions<SetRulesEngineOverrideParams | {
    asyncParams: () => Promise<SetRulesEngineOverrideParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setRulesEngineOverride.d.ts.map