import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "execute" function.
 */
export type ExecuteParams = WithOverrides<{
    targets: AbiParameterToPrimitiveType<{
        type: "address[]";
        name: "targets";
    }>;
    values: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "values";
    }>;
    calldatas: AbiParameterToPrimitiveType<{
        type: "bytes[]";
        name: "calldatas";
    }>;
    descriptionHash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "descriptionHash";
    }>;
}>;
export declare const FN_SELECTOR: "0x2656227d";
/**
 * Checks if the `execute` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `execute` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isExecuteSupported } from "thirdweb/extensions/vote";
 *
 * const supported = isExecuteSupported(["0x..."]);
 * ```
 */
export declare function isExecuteSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "execute" function.
 * @param options - The options for the execute function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeExecuteParams } from "thirdweb/extensions/vote";
 * const result = encodeExecuteParams({
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  descriptionHash: ...,
 * });
 * ```
 */
export declare function encodeExecuteParams(options: ExecuteParams): `0x${string}`;
/**
 * Encodes the "execute" function into a Hex string with its parameters.
 * @param options - The options for the execute function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeExecute } from "thirdweb/extensions/vote";
 * const result = encodeExecute({
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  descriptionHash: ...,
 * });
 * ```
 */
export declare function encodeExecute(options: ExecuteParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "execute" function on the contract.
 * @param options - The options for the "execute" function.
 * @returns A prepared transaction object.
 * @extension VOTE
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { execute } from "thirdweb/extensions/vote";
 *
 * const transaction = execute({
 *  contract,
 *  targets: ...,
 *  values: ...,
 *  calldatas: ...,
 *  descriptionHash: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function execute(options: BaseTransactionOptions<ExecuteParams | {
    asyncParams: () => Promise<ExecuteParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=execute.d.ts.map