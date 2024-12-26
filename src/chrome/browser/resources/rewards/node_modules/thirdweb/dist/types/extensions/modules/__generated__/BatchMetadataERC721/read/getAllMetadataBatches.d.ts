import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xe6c23512";
/**
 * Checks if the `getAllMetadataBatches` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getAllMetadataBatches` method is supported.
 * @modules BatchMetadataERC721
 * @example
 * ```ts
 * import { BatchMetadataERC721 } from "thirdweb/modules";
 * const supported = BatchMetadataERC721.isGetAllMetadataBatchesSupported(["0x..."]);
 * ```
 */
export declare function isGetAllMetadataBatchesSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getAllMetadataBatches function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules BatchMetadataERC721
 * @example
 * ```ts
 * import { BatchMetadataERC721 } from "thirdweb/modules";
 * const result = BatchMetadataERC721.decodeGetAllMetadataBatchesResultResult("...");
 * ```
 */
export declare function decodeGetAllMetadataBatchesResult(result: Hex): readonly {
    startTokenIdInclusive: bigint;
    endTokenIdInclusive: bigint;
    baseURI: string;
}[];
/**
 * Calls the "getAllMetadataBatches" function on the contract.
 * @param options - The options for the getAllMetadataBatches function.
 * @returns The parsed result of the function call.
 * @modules BatchMetadataERC721
 * @example
 * ```ts
 * import { BatchMetadataERC721 } from "thirdweb/modules";
 *
 * const result = await BatchMetadataERC721.getAllMetadataBatches({
 *  contract,
 * });
 *
 * ```
 */
export declare function getAllMetadataBatches(options: BaseTransactionOptions): Promise<readonly {
    startTokenIdInclusive: bigint;
    endTokenIdInclusive: bigint;
    baseURI: string;
}[]>;
//# sourceMappingURL=getAllMetadataBatches.d.ts.map