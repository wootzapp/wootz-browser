import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xa2309ff8";
/**
 * Checks if the `totalMinted` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalMinted` method is supported.
 * @modules ERC721Core
 * @example
 * ```ts
 * import { ERC721Core } from "thirdweb/modules";
 * const supported = ERC721Core.isTotalMintedSupported(["0x..."]);
 * ```
 */
export declare function isTotalMintedSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the totalMinted function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules ERC721Core
 * @example
 * ```ts
 * import { ERC721Core } from "thirdweb/modules";
 * const result = ERC721Core.decodeTotalMintedResultResult("...");
 * ```
 */
export declare function decodeTotalMintedResult(result: Hex): bigint;
/**
 * Calls the "totalMinted" function on the contract.
 * @param options - The options for the totalMinted function.
 * @returns The parsed result of the function call.
 * @modules ERC721Core
 * @example
 * ```ts
 * import { ERC721Core } from "thirdweb/modules";
 *
 * const result = await ERC721Core.totalMinted({
 *  contract,
 * });
 *
 * ```
 */
export declare function totalMinted(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=totalMinted.d.ts.map