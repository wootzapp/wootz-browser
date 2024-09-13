import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "quoteExactOutputSingle" function.
 */
export type QuoteExactOutputSingleParams = WithOverrides<{
    tokenIn: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenIn";
    }>;
    tokenOut: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenOut";
    }>;
    fee: AbiParameterToPrimitiveType<{
        type: "uint24";
        name: "fee";
    }>;
    amountOut: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amountOut";
    }>;
    sqrtPriceLimitX96: AbiParameterToPrimitiveType<{
        type: "uint160";
        name: "sqrtPriceLimitX96";
    }>;
}>;
export declare const FN_SELECTOR: "0x30d07f21";
/**
 * Checks if the `quoteExactOutputSingle` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `quoteExactOutputSingle` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isQuoteExactOutputSingleSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isQuoteExactOutputSingleSupported(["0x..."]);
 * ```
 */
export declare function isQuoteExactOutputSingleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "quoteExactOutputSingle" function.
 * @param options - The options for the quoteExactOutputSingle function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactOutputSingleParams } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactOutputSingleParams({
 *  tokenIn: ...,
 *  tokenOut: ...,
 *  fee: ...,
 *  amountOut: ...,
 *  sqrtPriceLimitX96: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactOutputSingleParams(options: QuoteExactOutputSingleParams): `0x${string}`;
/**
 * Encodes the "quoteExactOutputSingle" function into a Hex string with its parameters.
 * @param options - The options for the quoteExactOutputSingle function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactOutputSingle } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactOutputSingle({
 *  tokenIn: ...,
 *  tokenOut: ...,
 *  fee: ...,
 *  amountOut: ...,
 *  sqrtPriceLimitX96: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactOutputSingle(options: QuoteExactOutputSingleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "quoteExactOutputSingle" function on the contract.
 * @param options - The options for the "quoteExactOutputSingle" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { quoteExactOutputSingle } from "thirdweb/extensions/uniswap";
 *
 * const transaction = quoteExactOutputSingle({
 *  contract,
 *  tokenIn: ...,
 *  tokenOut: ...,
 *  fee: ...,
 *  amountOut: ...,
 *  sqrtPriceLimitX96: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function quoteExactOutputSingle(options: BaseTransactionOptions<QuoteExactOutputSingleParams | {
    asyncParams: () => Promise<QuoteExactOutputSingleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=quoteExactOutputSingle.d.ts.map