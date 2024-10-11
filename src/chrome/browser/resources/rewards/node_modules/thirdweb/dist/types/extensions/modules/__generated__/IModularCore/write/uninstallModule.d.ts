import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "uninstallModule" function.
 */
export type UninstallModuleParams = WithOverrides<{
    moduleContract: AbiParameterToPrimitiveType<{
        type: "address";
        name: "moduleContract";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0x70c109cd";
/**
 * Checks if the `uninstallModule` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `uninstallModule` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isUninstallModuleSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isUninstallModuleSupported(["0x..."]);
 * ```
 */
export declare function isUninstallModuleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "uninstallModule" function.
 * @param options - The options for the uninstallModule function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeUninstallModuleParams } from "thirdweb/extensions/modules";
 * const result = encodeUninstallModuleParams({
 *  moduleContract: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeUninstallModuleParams(options: UninstallModuleParams): `0x${string}`;
/**
 * Encodes the "uninstallModule" function into a Hex string with its parameters.
 * @param options - The options for the uninstallModule function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeUninstallModule } from "thirdweb/extensions/modules";
 * const result = encodeUninstallModule({
 *  moduleContract: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeUninstallModule(options: UninstallModuleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "uninstallModule" function on the contract.
 * @param options - The options for the "uninstallModule" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { uninstallModule } from "thirdweb/extensions/modules";
 *
 * const transaction = uninstallModule({
 *  contract,
 *  moduleContract: ...,
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
export declare function uninstallModule(options: BaseTransactionOptions<UninstallModuleParams | {
    asyncParams: () => Promise<UninstallModuleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=uninstallModule.d.ts.map