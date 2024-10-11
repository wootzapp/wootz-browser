import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "aggregate3" function.
 */
export type Aggregate3Params = WithOverrides<{
    calls: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "calls";
        components: [
            {
                type: "address";
                name: "target";
            },
            {
                type: "bool";
                name: "allowFailure";
            },
            {
                type: "bytes";
                name: "callData";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x82ad56cb";
/**
 * Checks if the `aggregate3` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `aggregate3` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isAggregate3Supported } from "thirdweb/extensions/multicall3";
 *
 * const supported = isAggregate3Supported(["0x..."]);
 * ```
 */
export declare function isAggregate3Supported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "aggregate3" function.
 * @param options - The options for the aggregate3 function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeAggregate3Params } from "thirdweb/extensions/multicall3";
 * const result = encodeAggregate3Params({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeAggregate3Params(options: Aggregate3Params): `0x${string}`;
/**
 * Encodes the "aggregate3" function into a Hex string with its parameters.
 * @param options - The options for the aggregate3 function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeAggregate3 } from "thirdweb/extensions/multicall3";
 * const result = encodeAggregate3({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeAggregate3(options: Aggregate3Params): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "aggregate3" function on the contract.
 * @param options - The options for the "aggregate3" function.
 * @returns A prepared transaction object.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { aggregate3 } from "thirdweb/extensions/multicall3";
 *
 * const transaction = aggregate3({
 *  contract,
 *  calls: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function aggregate3(options: BaseTransactionOptions<Aggregate3Params | {
    asyncParams: () => Promise<Aggregate3Params>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=aggregate3.d.ts.map