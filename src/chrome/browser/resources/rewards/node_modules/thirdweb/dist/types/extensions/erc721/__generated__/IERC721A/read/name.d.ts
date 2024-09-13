import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x06fdde03";
/**
 * Checks if the `name` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `name` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isNameSupported } from "thirdweb/extensions/erc721";
 * const supported = isNameSupported(["0x..."]);
 * ```
 */
export declare function isNameSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the name function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeNameResult } from "thirdweb/extensions/erc721";
 * const result = decodeNameResultResult("...");
 * ```
 */
export declare function decodeNameResult(result: Hex): string;
/**
 * Calls the "name" function on the contract.
 * @param options - The options for the name function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { name } from "thirdweb/extensions/erc721";
 *
 * const result = await name({
 *  contract,
 * });
 *
 * ```
 */
export declare function name(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=name.d.ts.map