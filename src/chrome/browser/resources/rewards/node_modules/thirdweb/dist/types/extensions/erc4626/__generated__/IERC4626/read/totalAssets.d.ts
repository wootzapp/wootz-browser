import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x01e1d114";
/**
 * Checks if the `totalAssets` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalAssets` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isTotalAssetsSupported } from "thirdweb/extensions/erc4626";
 * const supported = isTotalAssetsSupported(["0x..."]);
 * ```
 */
export declare function isTotalAssetsSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the totalAssets function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodeTotalAssetsResult } from "thirdweb/extensions/erc4626";
 * const result = decodeTotalAssetsResultResult("...");
 * ```
 */
export declare function decodeTotalAssetsResult(result: Hex): bigint;
/**
 * Calls the "totalAssets" function on the contract.
 * @param options - The options for the totalAssets function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { totalAssets } from "thirdweb/extensions/erc4626";
 *
 * const result = await totalAssets({
 *  contract,
 * });
 *
 * ```
 */
export declare function totalAssets(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=totalAssets.d.ts.map