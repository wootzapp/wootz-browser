import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xb280f703";
/**
 * Checks if the `sharedMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `sharedMetadata` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isSharedMetadataSupported } from "thirdweb/extensions/erc721";
 * const supported = isSharedMetadataSupported(["0x..."]);
 * ```
 */
export declare function isSharedMetadataSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the sharedMetadata function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeSharedMetadataResult } from "thirdweb/extensions/erc721";
 * const result = decodeSharedMetadataResultResult("...");
 * ```
 */
export declare function decodeSharedMetadataResult(result: Hex): readonly [string, string, string, string];
/**
 * Calls the "sharedMetadata" function on the contract.
 * @param options - The options for the sharedMetadata function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { sharedMetadata } from "thirdweb/extensions/erc721";
 *
 * const result = await sharedMetadata({
 *  contract,
 * });
 *
 * ```
 */
export declare function sharedMetadata(options: BaseTransactionOptions): Promise<readonly [string, string, string, string]>;
//# sourceMappingURL=sharedMetadata.d.ts.map