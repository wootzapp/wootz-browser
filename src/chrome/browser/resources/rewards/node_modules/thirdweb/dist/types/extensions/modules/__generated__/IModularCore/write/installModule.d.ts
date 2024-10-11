import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "installModule" function.
 */
export type InstallModuleParams = WithOverrides<{
    moduleContract: AbiParameterToPrimitiveType<{
        type: "address";
        name: "moduleContract";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0x8da798da";
/**
 * Checks if the `installModule` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `installModule` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isInstallModuleSupported } from "thirdweb/extensions/modules";
 *
 * const supported = isInstallModuleSupported(["0x..."]);
 * ```
 */
export declare function isInstallModuleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "installModule" function.
 * @param options - The options for the installModule function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeInstallModuleParams } from "thirdweb/extensions/modules";
 * const result = encodeInstallModuleParams({
 *  moduleContract: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeInstallModuleParams(options: InstallModuleParams): `0x${string}`;
/**
 * Encodes the "installModule" function into a Hex string with its parameters.
 * @param options - The options for the installModule function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeInstallModule } from "thirdweb/extensions/modules";
 * const result = encodeInstallModule({
 *  moduleContract: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeInstallModule(options: InstallModuleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "installModule" function on the contract.
 * @param options - The options for the "installModule" function.
 * @returns A prepared transaction object.
 * @extension MODULES
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { installModule } from "thirdweb/extensions/modules";
 *
 * const transaction = installModule({
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
export declare function installModule(options: BaseTransactionOptions<InstallModuleParams | {
    asyncParams: () => Promise<InstallModuleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=installModule.d.ts.map