import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "quoteExactInputSingle" function.
 */
export type QuoteExactInputSingleParams = WithOverrides<{
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
    amountIn: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amountIn";
    }>;
    sqrtPriceLimitX96: AbiParameterToPrimitiveType<{
        type: "uint160";
        name: "sqrtPriceLimitX96";
    }>;
}>;
export declare const FN_SELECTOR: "0xf7729d43";
/**
 * Checks if the `quoteExactInputSingle` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `quoteExactInputSingle` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isQuoteExactInputSingleSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isQuoteExactInputSingleSupported(["0x..."]);
 * ```
 */
export declare function isQuoteExactInputSingleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "quoteExactInputSingle" function.
 * @param options - The options for the quoteExactInputSingle function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactInputSingleParams } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactInputSingleParams({
 *  tokenIn: ...,
 *  tokenOut: ...,
 *  fee: ...,
 *  amountIn: ...,
 *  sqrtPriceLimitX96: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactInputSingleParams(options: QuoteExactInputSingleParams): `0x${string}`;
/**
 * Encodes the "quoteExactInputSingle" function into a Hex string with its parameters.
 * @param options - The options for the quoteExactInputSingle function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeQuoteExactInputSingle } from "thirdweb/extensions/uniswap";
 * const result = encodeQuoteExactInputSingle({
 *  tokenIn: ...,
 *  tokenOut: ...,
 *  fee: ...,
 *  amountIn: ...,
 *  sqrtPriceLimitX96: ...,
 * });
 * ```
 */
export declare function encodeQuoteExactInputSingle(options: QuoteExactInputSingleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "quoteExactInputSingle" function on the contract.
 * @param options - The options for the "quoteExactInputSingle" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { quoteExactInputSingle } from "thirdweb/extensions/uniswap";
 *
 * const transaction = quoteExactInputSingle({
 *  contract,
 *  tokenIn: ...,
 *  tokenOut: ...,
 *  fee: ...,
 *  amountIn: ...,
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
export declare function quoteExactInputSingle(options: BaseTransactionOptions<QuoteExactInputSingleParams | {
    asyncParams: () => Promise<QuoteExactInputSingleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=quoteExactInputSingle.d.ts.map