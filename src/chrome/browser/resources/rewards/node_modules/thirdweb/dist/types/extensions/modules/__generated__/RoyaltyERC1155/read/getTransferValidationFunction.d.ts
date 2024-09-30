import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x0d705df6";
/**
 * Checks if the `getTransferValidationFunction` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getTransferValidationFunction` method is supported.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 * const supported = RoyaltyERC1155.isGetTransferValidationFunctionSupported(["0x..."]);
 * ```
 */
export declare function isGetTransferValidationFunctionSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getTransferValidationFunction function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 * const result = RoyaltyERC1155.decodeGetTransferValidationFunctionResultResult("...");
 * ```
 */
export declare function decodeGetTransferValidationFunctionResult(result: Hex): readonly [`0x${string}`, boolean];
/**
 * Calls the "getTransferValidationFunction" function on the contract.
 * @param options - The options for the getTransferValidationFunction function.
 * @returns The parsed result of the function call.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 *
 * const result = await RoyaltyERC1155.getTransferValidationFunction({
 *  contract,
 * });
 *
 * ```
 */
export declare function getTransferValidationFunction(options: BaseTransactionOptions): Promise<readonly [`0x${string}`, boolean]>;
//# sourceMappingURL=getTransferValidationFunction.d.ts.map