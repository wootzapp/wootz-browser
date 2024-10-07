import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "quoteExactInput" function.
 */
export type QuoteExactInputParams = WithOverrides<{
    path: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "path";
    }>;
    amountIn: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amountIn";
    }>;
}>;
export declare const FN_SELECTOR: "0xcdca1753";
/**
 * Checks if the `quoteExactInput` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `quoteExactInput` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isQuoteExactInputSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isQuoteExactInputSupported(["0x..."]);
 * ```
 */
export declare function isQuoteExactInputSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "quoteExactInput" function.
 * @param options - The options for the quoteExactInput function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactInputParams } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactInputParams({
 *  path: ...,
 *  amountIn: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactInputParams(options: QuoteExactInputParams): `0x${string}`;
/**
 * Encodes the "quoteExactInput" function into a Hex string with its parameters.
 * @param options - The options for the quoteExactInput function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactInput } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactInput({
 *  path: ...,
 *  amountIn: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactInput(options: QuoteExactInputParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "quoteExactInput" function on the contract.
 * @param options - The options for the "quoteExactInput" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { quoteExactInput } from "thirdweb/extensions/uniswap";
 *
 * const transaction = quoteExactInput({
 *  contract,
 *  path: ...,
 *  amountIn: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function quoteExactInput(options: BaseTransactionOptions<QuoteExactInputParams | {
    asyncParams: () => Promise<QuoteExactInputParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=quoteExactInput.d.ts.map