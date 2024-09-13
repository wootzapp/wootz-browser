import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "exactOutputSingle" function.
 */
export type ExactOutputSingleParams = WithOverrides<{
    params: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "params";
        components: [
            {
                type: "address";
                name: "tokenIn";
            },
            {
                type: "address";
                name: "tokenOut";
            },
            {
                type: "uint24";
                name: "fee";
            },
            {
                type: "address";
                name: "recipient";
            },
            {
                type: "uint256";
                name: "deadline";
            },
            {
                type: "uint256";
                name: "amountOut";
            },
            {
                type: "uint256";
                name: "amountInMaximum";
            },
            {
                type: "uint160";
                name: "sqrtPriceLimitX96";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0xdb3e2198";
/**
 * Checks if the `exactOutputSingle` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `exactOutputSingle` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isExactOutputSingleSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isExactOutputSingleSupported(["0x..."]);
 * ```
 */
export declare function isExactOutputSingleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "exactOutputSingle" function.
 * @param options - The options for the exactOutputSingle function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactOutputSingleParams } from "thirdweb/extensions/uniswap";
 * const result = encodeExactOutputSingleParams({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactOutputSingleParams(options: ExactOutputSingleParams): `0x${string}`;
/**
 * Encodes the "exactOutputSingle" function into a Hex string with its parameters.
 * @param options - The options for the exactOutputSingle function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactOutputSingle } from "thirdweb/extensions/uniswap";
 * const result = encodeExactOutputSingle({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactOutputSingle(options: ExactOutputSingleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "exactOutputSingle" function on the contract.
 * @param options - The options for the "exactOutputSingle" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { exactOutputSingle } from "thirdweb/extensions/uniswap";
 *
 * const transaction = exactOutputSingle({
 *  contract,
 *  params: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function exactOutputSingle(options: BaseTransactionOptions<ExactOutputSingleParams | {
    asyncParams: () => Promise<ExactOutputSingleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=exactOutputSingle.d.ts.map