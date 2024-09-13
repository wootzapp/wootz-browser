import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x18160ddd";
/**
 * Checks if the `totalSupply` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalSupply` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isTotalSupplySupported } from "thirdweb/extensions/erc20";
 * const supported = isTotalSupplySupported(["0x..."]);
 * ```
 */
export declare function isTotalSupplySupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the totalSupply function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeTotalSupplyResult } from "thirdweb/extensions/erc20";
 * const result = decodeTotalSupplyResultResult("...");
 * ```
 */
export declare function decodeTotalSupplyResult(result: Hex): bigint;
/**
 * Calls the "totalSupply" function on the contract.
 * @param options - The options for the totalSupply function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { totalSupply } from "thirdweb/extensions/erc20";
 *
 * const result = await totalSupply({
 *  contract,
 * });
 *
 * ```
 */
export declare function totalSupply(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=totalSupply.d.ts.map