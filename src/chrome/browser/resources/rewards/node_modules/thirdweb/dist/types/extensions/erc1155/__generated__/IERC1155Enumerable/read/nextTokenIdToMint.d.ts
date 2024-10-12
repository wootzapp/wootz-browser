import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x3b1475a7";
/**
 * Checks if the `nextTokenIdToMint` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `nextTokenIdToMint` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isNextTokenIdToMintSupported } from "thirdweb/extensions/erc1155";
 * const supported = isNextTokenIdToMintSupported(["0x..."]);
 * ```
 */
export declare function isNextTokenIdToMintSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the nextTokenIdToMint function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeNextTokenIdToMintResult } from "thirdweb/extensions/erc1155";
 * const result = decodeNextTokenIdToMintResultResult("...");
 * ```
 */
export declare function decodeNextTokenIdToMintResult(result: Hex): bigint;
/**
 * Calls the "nextTokenIdToMint" function on the contract.
 * @param options - The options for the nextTokenIdToMint function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { nextTokenIdToMint } from "thirdweb/extensions/erc1155";
 *
 * const result = await nextTokenIdToMint({
 *  contract,
 * });
 *
 * ```
 */
export declare function nextTokenIdToMint(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=nextTokenIdToMint.d.ts.map