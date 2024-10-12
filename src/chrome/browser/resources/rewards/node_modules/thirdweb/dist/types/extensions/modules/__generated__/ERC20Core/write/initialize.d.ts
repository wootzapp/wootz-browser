import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "initialize" function.
 */
export type InitializeParams = WithOverrides<{
    name: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_name";
    }>;
    symbol: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_symbol";
    }>;
    contractURI: AbiParameterToPrimitiveType<{
        type: "string";
        name: "_contractURI";
    }>;
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_owner";
    }>;
    modules: AbiParameterToPrimitiveType<{
        type: "address[]";
        name: "_modules";
    }>;
    moduleInstallData: AbiParameterToPrimitiveType<{
        type: "bytes[]";
        name: "_moduleInstallData";
    }>;
}>;
export declare const FN_SELECTOR: "0x62835ade";
/**
 * Checks if the `initialize` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `initialize` method is supported.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { ERC20Core } from "thirdweb/modules";
 *
 * const supported = ERC20Core.isInitializeSupported(["0x..."]);
 * ```
 */
export declare function isInitializeSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "initialize" function.
 * @param options - The options for the initialize function.
 * @returns The encoded ABI parameters.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { ERC20Core } from "thirdweb/modules";
 * const result = ERC20Core.encodeInitializeParams({
 *  name: ...,
 *  symbol: ...,
 *  contractURI: ...,
 *  owner: ...,
 *  modules: ...,
 *  moduleInstallData: ...,
 * });
 * ```
 */
export declare function encodeInitializeParams(options: InitializeParams): `0x${string}`;
/**
 * Encodes the "initialize" function into a Hex string with its parameters.
 * @param options - The options for the initialize function.
 * @returns The encoded hexadecimal string.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { ERC20Core } from "thirdweb/modules";
 * const result = ERC20Core.encodeInitialize({
 *  name: ...,
 *  symbol: ...,
 *  contractURI: ...,
 *  owner: ...,
 *  modules: ...,
 *  moduleInstallData: ...,
 * });
 * ```
 */
export declare function encodeInitialize(options: InitializeParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "initialize" function on the contract.
 * @param options - The options for the "initialize" function.
 * @returns A prepared transaction object.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { ERC20Core } from "thirdweb/modules";
 *
 * const transaction = ERC20Core.initialize({
 *  contract,
 *  name: ...,
 *  symbol: ...,
 *  contractURI: ...,
 *  owner: ...,
 *  modules: ...,
 *  moduleInstallData: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function initialize(options: BaseTransactionOptions<InitializeParams | {
    asyncParams: () => Promise<InitializeParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=initialize.d.ts.map