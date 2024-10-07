import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xacd083f8";
/**
 * Checks if the `nextTokenIdToClaim` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `nextTokenIdToClaim` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isNextTokenIdToClaimSupported } from "thirdweb/extensions/erc721";
 * const supported = isNextTokenIdToClaimSupported(["0x..."]);
 * ```
 */
export declare function isNextTokenIdToClaimSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the nextTokenIdToClaim function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeNextTokenIdToClaimResult } from "thirdweb/extensions/erc721";
 * const result = decodeNextTokenIdToClaimResultResult("...");
 * ```
 */
export declare function decodeNextTokenIdToClaimResult(result: Hex): bigint;
/**
 * Calls the "nextTokenIdToClaim" function on the contract.
 * @param options - The options for the nextTokenIdToClaim function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { nextTokenIdToClaim } from "thirdweb/extensions/erc721";
 *
 * const result = await nextTokenIdToClaim({
 *  contract,
 * });
 *
 * ```
 */
export declare function nextTokenIdToClaim(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=nextTokenIdToClaim.d.ts.map