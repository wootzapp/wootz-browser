import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "enableFeeAmount" function.
 */
export type EnableFeeAmountParams = WithOverrides<{
    fee: AbiParameterToPrimitiveType<{
        type: "uint24";
        name: "fee";
    }>;
    tickSpacing: AbiParameterToPrimitiveType<{
        type: "int24";
        name: "tickSpacing";
    }>;
}>;
export declare const FN_SELECTOR: "0x8a7c195f";
/**
 * Checks if the `enableFeeAmount` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `enableFeeAmount` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isEnableFeeAmountSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isEnableFeeAmountSupported(["0x..."]);
 * ```
 */
export declare function isEnableFeeAmountSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "enableFeeAmount" function.
 * @param options - The options for the enableFeeAmount function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeEnableFeeAmountParams } from "thirdweb/extensions/uniswap";
 * const result = encodeEnableFeeAmountParams({
 *  fee: ...,
 *  tickSpacing: ...,
 * });
 * ```
 */
export declare function encodeEnableFeeAmountParams(options: EnableFeeAmountParams): `0x${string}`;
/**
 * Encodes the "enableFeeAmount" function into a Hex string with its parameters.
 * @param options - The options for the enableFeeAmount function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeEnableFeeAmount } from "thirdweb/extensions/uniswap";
 * const result = encodeEnableFeeAmount({
 *  fee: ...,
 *  tickSpacing: ...,
 * });
 * ```
 */
export declare function encodeEnableFeeAmount(options: EnableFeeAmountParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "enableFeeAmount" function on the contract.
 * @param options - The options for the "enableFeeAmount" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { enableFeeAmount } from "thirdweb/extensions/uniswap";
 *
 * const transaction = enableFeeAmount({
 *  contract,
 *  fee: ...,
 *  tickSpacing: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function enableFeeAmount(options: BaseTransactionOptions<EnableFeeAmountParams | {
    asyncParams: () => Promise<EnableFeeAmountParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=enableFeeAmount.d.ts.map