import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "exactOutput" function.
 */
export type ExactOutputParams = WithOverrides<{
    params: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "params";
        components: [
            {
                type: "bytes";
                name: "path";
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
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0xf28c0498";
/**
 * Checks if the `exactOutput` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `exactOutput` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isExactOutputSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isExactOutputSupported(["0x..."]);
 * ```
 */
export declare function isExactOutputSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "exactOutput" function.
 * @param options - The options for the exactOutput function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactOutputParams } from "thirdweb/extensions/uniswap";
 * const result = encodeExactOutputParams({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactOutputParams(options: ExactOutputParams): `0x${string}`;
/**
 * Encodes the "exactOutput" function into a Hex string with its parameters.
 * @param options - The options for the exactOutput function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactOutput } from "thirdweb/extensions/uniswap";
 * const result = encodeExactOutput({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactOutput(options: ExactOutputParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "exactOutput" function on the contract.
 * @param options - The options for the "exactOutput" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { exactOutput } from "thirdweb/extensions/uniswap";
 *
 * const transaction = exactOutput({
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
export declare function exactOutput(options: BaseTransactionOptions<ExactOutputParams | {
    asyncParams: () => Promise<ExactOutputParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=exactOutput.d.ts.map