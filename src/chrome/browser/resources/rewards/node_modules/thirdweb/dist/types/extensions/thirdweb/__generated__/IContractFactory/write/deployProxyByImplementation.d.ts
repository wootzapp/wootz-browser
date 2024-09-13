import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "deployProxyByImplementation" function.
 */
export type DeployProxyByImplementationParams = WithOverrides<{
    implementation: AbiParameterToPrimitiveType<{
        type: "address";
        name: "implementation";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
    salt: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "salt";
    }>;
}>;
export declare const FN_SELECTOR: "0x11b804ab";
/**
 * Checks if the `deployProxyByImplementation` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `deployProxyByImplementation` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isDeployProxyByImplementationSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isDeployProxyByImplementationSupported(["0x..."]);
 * ```
 */
export declare function isDeployProxyByImplementationSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "deployProxyByImplementation" function.
 * @param options - The options for the deployProxyByImplementation function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeDeployProxyByImplementationParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeDeployProxyByImplementationParams({
 *  implementation: ...,
 *  data: ...,
 *  salt: ...,
 * });
 * ```
 */
export declare function encodeDeployProxyByImplementationParams(options: DeployProxyByImplementationParams): `0x${string}`;
/**
 * Encodes the "deployProxyByImplementation" function into a Hex string with its parameters.
 * @param options - The options for the deployProxyByImplementation function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeDeployProxyByImplementation } from "thirdweb/extensions/thirdweb";
 * const result = encodeDeployProxyByImplementation({
 *  implementation: ...,
 *  data: ...,
 *  salt: ...,
 * });
 * ```
 */
export declare function encodeDeployProxyByImplementation(options: DeployProxyByImplementationParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "deployProxyByImplementation" function on the contract.
 * @param options - The options for the "deployProxyByImplementation" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { deployProxyByImplementation } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = deployProxyByImplementation({
 *  contract,
 *  implementation: ...,
 *  data: ...,
 *  salt: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function deployProxyByImplementation(options: BaseTransactionOptions<DeployProxyByImplementationParams | {
    asyncParams: () => Promise<DeployProxyByImplementationParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=deployProxyByImplementation.d.ts.map