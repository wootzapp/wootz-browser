import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x75794a3c";
/**
 * Checks if the `nextTokenId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `nextTokenId` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isNextTokenIdSupported } from "thirdweb/extensions/erc1155";
 * const supported = isNextTokenIdSupported(["0x..."]);
 * ```
 */
export declare function isNextTokenIdSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the nextTokenId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeNextTokenIdResult } from "thirdweb/extensions/erc1155";
 * const result = decodeNextTokenIdResultResult("...");
 * ```
 */
export declare function decodeNextTokenIdResult(result: Hex): bigint;
/**
 * Calls the "nextTokenId" function on the contract.
 * @param options - The options for the nextTokenId function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { nextTokenId } from "thirdweb/extensions/erc1155";
 *
 * const result = await nextTokenId({
 *  contract,
 * });
 *
 * ```
 */
export declare function nextTokenId(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=nextTokenId.d.ts.map