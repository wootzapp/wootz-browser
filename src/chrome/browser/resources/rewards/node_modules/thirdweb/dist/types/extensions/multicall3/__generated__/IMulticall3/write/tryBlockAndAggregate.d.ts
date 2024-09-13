import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "tryBlockAndAggregate" function.
 */
export type TryBlockAndAggregateParams = WithOverrides<{
    requireSuccess: AbiParameterToPrimitiveType<{
        type: "bool";
        name: "requireSuccess";
    }>;
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
export declare const FN_SELECTOR: "0x399542e9";
/**
 * Checks if the `tryBlockAndAggregate` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tryBlockAndAggregate` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isTryBlockAndAggregateSupported } from "thirdweb/extensions/multicall3";
 *
 * const supported = isTryBlockAndAggregateSupported(["0x..."]);
 * ```
 */
export declare function isTryBlockAndAggregateSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tryBlockAndAggregate" function.
 * @param options - The options for the tryBlockAndAggregate function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeTryBlockAndAggregateParams } from "thirdweb/extensions/multicall3";
 * const result = encodeTryBlockAndAggregateParams({
 *  requireSuccess: ...,
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeTryBlockAndAggregateParams(options: TryBlockAndAggregateParams): `0x${string}`;
/**
 * Encodes the "tryBlockAndAggregate" function into a Hex string with its parameters.
 * @param options - The options for the tryBlockAndAggregate function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeTryBlockAndAggregate } from "thirdweb/extensions/multicall3";
 * const result = encodeTryBlockAndAggregate({
 *  requireSuccess: ...,
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeTryBlockAndAggregate(options: TryBlockAndAggregateParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "tryBlockAndAggregate" function on the contract.
 * @param options - The options for the "tryBlockAndAggregate" function.
 * @returns A prepared transaction object.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { tryBlockAndAggregate } from "thirdweb/extensions/multicall3";
 *
 * const transaction = tryBlockAndAggregate({
 *  contract,
 *  requireSuccess: ...,
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
export declare function tryBlockAndAggregate(options: BaseTransactionOptions<TryBlockAndAggregateParams | {
    asyncParams: () => Promise<TryBlockAndAggregateParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=tryBlockAndAggregate.d.ts.map