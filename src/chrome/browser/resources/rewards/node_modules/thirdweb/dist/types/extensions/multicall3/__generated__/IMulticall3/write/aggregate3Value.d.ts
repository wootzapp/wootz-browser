import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "aggregate3Value" function.
 */
export type Aggregate3ValueParams = WithOverrides<{
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
                type: "uint256";
                name: "value";
            },
            {
                type: "bytes";
                name: "callData";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x174dea71";
/**
 * Checks if the `aggregate3Value` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `aggregate3Value` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isAggregate3ValueSupported } from "thirdweb/extensions/multicall3";
 *
 * const supported = isAggregate3ValueSupported(["0x..."]);
 * ```
 */
export declare function isAggregate3ValueSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "aggregate3Value" function.
 * @param options - The options for the aggregate3Value function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeAggregate3ValueParams } from "thirdweb/extensions/multicall3";
 * const result = encodeAggregate3ValueParams({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeAggregate3ValueParams(options: Aggregate3ValueParams): `0x${string}`;
/**
 * Encodes the "aggregate3Value" function into a Hex string with its parameters.
 * @param options - The options for the aggregate3Value function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeAggregate3Value } from "thirdweb/extensions/multicall3";
 * const result = encodeAggregate3Value({
 *  calls: ...,
 * });
 * ```
 */
export declare function encodeAggregate3Value(options: Aggregate3ValueParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "aggregate3Value" function on the contract.
 * @param options - The options for the "aggregate3Value" function.
 * @returns A prepared transaction object.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { aggregate3Value } from "thirdweb/extensions/multicall3";
 *
 * const transaction = aggregate3Value({
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
export declare function aggregate3Value(options: BaseTransactionOptions<Aggregate3ValueParams | {
    asyncParams: () => Promise<Aggregate3ValueParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=aggregate3Value.d.ts.map