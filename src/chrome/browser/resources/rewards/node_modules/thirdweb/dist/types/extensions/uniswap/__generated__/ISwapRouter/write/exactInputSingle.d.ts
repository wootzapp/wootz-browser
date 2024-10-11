import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "exactInputSingle" function.
 */
export type ExactInputSingleParams = WithOverrides<{
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
                name: "amountIn";
            },
            {
                type: "uint256";
                name: "amountOutMinimum";
            },
            {
                type: "uint160";
                name: "sqrtPriceLimitX96";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x414bf389";
/**
 * Checks if the `exactInputSingle` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `exactInputSingle` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isExactInputSingleSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isExactInputSingleSupported(["0x..."]);
 * ```
 */
export declare function isExactInputSingleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "exactInputSingle" function.
 * @param options - The options for the exactInputSingle function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactInputSingleParams } from "thirdweb/extensions/uniswap";
 * const result = encodeExactInputSingleParams({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactInputSingleParams(options: ExactInputSingleParams): `0x${string}`;
/**
 * Encodes the "exactInputSingle" function into a Hex string with its parameters.
 * @param options - The options for the exactInputSingle function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactInputSingle } from "thirdweb/extensions/uniswap";
 * const result = encodeExactInputSingle({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactInputSingle(options: ExactInputSingleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "exactInputSingle" function on the contract.
 * @param options - The options for the "exactInputSingle" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { exactInputSingle } from "thirdweb/extensions/uniswap";
 *
 * const transaction = exactInputSingle({
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
export declare function exactInputSingle(options: BaseTransactionOptions<ExactInputSingleParams | {
    asyncParams: () => Promise<ExactInputSingleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=exactInputSingle.d.ts.map