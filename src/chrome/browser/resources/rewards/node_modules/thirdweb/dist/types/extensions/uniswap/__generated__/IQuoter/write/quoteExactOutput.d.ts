import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "quoteExactOutput" function.
 */
export type QuoteExactOutputParams = WithOverrides<{
    path: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "path";
    }>;
    amountOut: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amountOut";
    }>;
}>;
export declare const FN_SELECTOR: "0x2f80bb1d";
/**
 * Checks if the `quoteExactOutput` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `quoteExactOutput` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isQuoteExactOutputSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isQuoteExactOutputSupported(["0x..."]);
 * ```
 */
export declare function isQuoteExactOutputSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "quoteExactOutput" function.
 * @param options - The options for the quoteExactOutput function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactOutputParams } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactOutputParams({
 *  path: ...,
 *  amountOut: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactOutputParams(options: QuoteExactOutputParams): `0x${string}`;
/**
 * Encodes the "quoteExactOutput" function into a Hex string with its parameters.
 * @param options - The options for the quoteExactOutput function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactOutput } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactOutput({
 *  path: ...,
 *  amountOut: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactOutput(options: QuoteExactOutputParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "quoteExactOutput" function on the contract.
 * @param options - The options for the "quoteExactOutput" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { quoteExactOutput } from "thirdweb/extensions/uniswap";
 *
 * const transaction = quoteExactOutput({
 *  contract,
 *  path: ...,
 *  amountOut: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function quoteExactOutput(options: BaseTransactionOptions<QuoteExactOutputParams | {
    asyncParams: () => Promise<QuoteExactOutputParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=quoteExactOutput.d.ts.map