import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xe6798baa";
/**
 * Checks if the `startTokenId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `startTokenId` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isStartTokenIdSupported } from "thirdweb/extensions/erc721";
 * const supported = isStartTokenIdSupported(["0x..."]);
 * ```
 */
export declare function isStartTokenIdSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the startTokenId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeStartTokenIdResult } from "thirdweb/extensions/erc721";
 * const result = decodeStartTokenIdResultResult("...");
 * ```
 */
export declare function decodeStartTokenIdResult(result: Hex): bigint;
/**
 * Calls the "startTokenId" function on the contract.
 * @param options - The options for the startTokenId function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { startTokenId } from "thirdweb/extensions/erc721";
 *
 * const result = await startTokenId({
 *  contract,
 * });
 *
 * ```
 */
export declare function startTokenId(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=startTokenId.d.ts.map