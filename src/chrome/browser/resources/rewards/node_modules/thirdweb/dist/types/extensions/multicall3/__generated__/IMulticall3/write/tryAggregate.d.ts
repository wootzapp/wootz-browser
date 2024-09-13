import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "tryAggregate" function.
 */
export type TryAggregateParams = WithOverrides<{
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
export declare const FN_SELECTOR: "0xbce38bd7";
/**
 * Checks if the `tryAggregate` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tryAggregate` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isTryAggregateSupported } from "thirdweb/extensions/multicall3";
 *
 * const supported = isTryAggregateSupported(["0x..."]);
 * ```
 */
export declare function isTryAggregateSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tryAggregate" function.
 * @param options - The options for the tryAggregate function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeTryAggregateParams } from "thirdweb/extensions/multicall3";
 * const result = encodeTryAggregateParams({
 *  requireSuccess: ...,
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeTryAggregateParams(options: TryAggregateParams): `0x${string}`;
/**
 * Encodes the "tryAggregate" function into a Hex string with its parameters.
 * @param options - The options for the tryAggregate function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeTryAggregate } from "thirdweb/extensions/multicall3";
 * const result = encodeTryAggregate({
 *  requireSuccess: ...,
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeTryAggregate(options: TryAggregateParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "tryAggregate" function on the contract.
 * @param options - The options for the "tryAggregate" function.
 * @returns A prepared transaction object.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { tryAggregate } from "thirdweb/extensions/multicall3";
 *
 * const transaction = tryAggregate({
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
export declare function tryAggregate(options: BaseTransactionOptions<TryAggregateParams | {
    asyncParams: () => Promise<TryAggregateParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=tryAggregate.d.ts.map