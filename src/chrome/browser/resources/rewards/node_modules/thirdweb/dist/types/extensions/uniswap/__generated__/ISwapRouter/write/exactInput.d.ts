import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "exactInput" function.
 */
export type ExactInputParams = WithOverrides<{
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
                name: "amountIn";
            },
            {
                type: "uint256";
                name: "amountOutMinimum";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0xc04b8d59";
/**
 * Checks if the `exactInput` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `exactInput` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isExactInputSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isExactInputSupported(["0x..."]);
 * ```
 */
export declare function isExactInputSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "exactInput" function.
 * @param options - The options for the exactInput function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactInputParams } from "thirdweb/extensions/uniswap";
 * const result = encodeExactInputParams({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactInputParams(options: ExactInputParams): `0x${string}`;
/**
 * Encodes the "exactInput" function into a Hex string with its parameters.
 * @param options - The options for the exactInput function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeExactInput } from "thirdweb/extensions/uniswap";
 * const result = encodeExactInput({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeExactInput(options: ExactInputParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "exactInput" function on the contract.
 * @param options - The options for the "exactInput" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { exactInput } from "thirdweb/extensions/uniswap";
 *
 * const transaction = exactInput({
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
export declare function exactInput(options: BaseTransactionOptions<ExactInputParams | {
    asyncParams: () => Promise<ExactInputParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=exactInput.d.ts.map