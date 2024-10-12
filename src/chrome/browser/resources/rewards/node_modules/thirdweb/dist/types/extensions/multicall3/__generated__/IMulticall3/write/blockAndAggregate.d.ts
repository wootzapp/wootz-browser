import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "blockAndAggregate" function.
 */
export type BlockAndAggregateParams = WithOverrides<{
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
export declare const FN_SELECTOR: "0xc3077fa9";
/**
 * Checks if the `blockAndAggregate` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `blockAndAggregate` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isBlockAndAggregateSupported } from "thirdweb/extensions/multicall3";
 *
 * const supported = isBlockAndAggregateSupported(["0x..."]);
 * ```
 */
export declare function isBlockAndAggregateSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "blockAndAggregate" function.
 * @param options - The options for the blockAndAggregate function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeBlockAndAggregateParams } from "thirdweb/extensions/multicall3";
 * const result = encodeBlockAndAggregateParams({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeBlockAndAggregateParams(options: BlockAndAggregateParams): `0x${string}`;
/**
 * Encodes the "blockAndAggregate" function into a Hex string with its parameters.
 * @param options - The options for the blockAndAggregate function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeBlockAndAggregate } from "thirdweb/extensions/multicall3";
 * const result = encodeBlockAndAggregate({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeBlockAndAggregate(options: BlockAndAggregateParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "blockAndAggregate" function on the contract.
 * @param options - The options for the "blockAndAggregate" function.
 * @returns A prepared transaction object.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { blockAndAggregate } from "thirdweb/extensions/multicall3";
 *
 * const transaction = blockAndAggregate({
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
export declare function blockAndAggregate(options: BaseTransactionOptions<BlockAndAggregateParams | {
    asyncParams: () => Promise<BlockAndAggregateParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=blockAndAggregate.d.ts.map