import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "feeAmountTickSpacing" function.
 */
export type FeeAmountTickSpacingParams = {
    fee: AbiParameterToPrimitiveType<{
        type: "uint24";
        name: "fee";
    }>;
};
export declare const FN_SELECTOR: "0x22afcccb";
/**
 * Checks if the `feeAmountTickSpacing` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `feeAmountTickSpacing` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isFeeAmountTickSpacingSupported } from "thirdweb/extensions/uniswap";
 * const supported = isFeeAmountTickSpacingSupported(["0x..."]);
 * ```
 */
export declare function isFeeAmountTickSpacingSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "feeAmountTickSpacing" function.
 * @param options - The options for the feeAmountTickSpacing function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeFeeAmountTickSpacingParams } from "thirdweb/extensions/uniswap";
 * const result = encodeFeeAmountTickSpacingParams({
 *  fee: ...,
 * });
 * ```
 */
export declare function encodeFeeAmountTickSpacingParams(options: FeeAmountTickSpacingParams): `0x${string}`;
/**
 * Encodes the "feeAmountTickSpacing" function into a Hex string with its parameters.
 * @param options - The options for the feeAmountTickSpacing function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeFeeAmountTickSpacing } from "thirdweb/extensions/uniswap";
 * const result = encodeFeeAmountTickSpacing({
 *  fee: ...,
 * });
 * ```
 */
export declare function encodeFeeAmountTickSpacing(options: FeeAmountTickSpacingParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the feeAmountTickSpacing function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { decodeFeeAmountTickSpacingResult } from "thirdweb/extensions/uniswap";
 * const result = decodeFeeAmountTickSpacingResultResult("...");
 * ```
 */
export declare function decodeFeeAmountTickSpacingResult(result: Hex): number;
/**
 * Calls the "feeAmountTickSpacing" function on the contract.
 * @param options - The options for the feeAmountTickSpacing function.
 * @returns The parsed result of the function call.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { feeAmountTickSpacing } from "thirdweb/extensions/uniswap";
 *
 * const result = await feeAmountTickSpacing({
 *  contract,
 *  fee: ...,
 * });
 *
 * ```
 */
export declare function feeAmountTickSpacing(options: BaseTransactionOptions<FeeAmountTickSpacingParams>): Promise<number>;
//# sourceMappingURL=feeAmountTickSpacing.d.ts.map