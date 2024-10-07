import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "aggregate" function.
 */
export type AggregateParams = WithOverrides<{
    calls: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "calls";
        components: [
            {
                type: "address";
                name: "target";
            },
            {
                type: "bytes";
                name: "callData";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x252dba42";
/**
 * Checks if the `aggregate` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `aggregate` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isAggregateSupported } from "thirdweb/extensions/multicall3";
 *
 * const supported = isAggregateSupported(["0x..."]);
 * ```
 */
export declare function isAggregateSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "aggregate" function.
 * @param options - The options for the aggregate function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeAggregateParams } from "thirdweb/extensions/multicall3";
 * const result = encodeAggregateParams({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeAggregateParams(options: AggregateParams): `0x${string}`;
/**
 * Encodes the "aggregate" function into a Hex string with its parameters.
 * @param options - The options for the aggregate function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeAggregate } from "thirdweb/extensions/multicall3";
 * const result = encodeAggregate({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeAggregate(options: AggregateParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "aggregate" function on the contract.
 * @param options - The options for the "aggregate" function.
 * @returns A prepared transaction object.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { aggregate } from "thirdweb/extensions/multicall3";
 *
 * const transaction = aggregate({
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
export declare function aggregate(options: BaseTransactionOptions<AggregateParams | {
    asyncParams: () => Promise<AggregateParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=aggregate.d.ts.map