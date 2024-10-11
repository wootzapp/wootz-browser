import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xfc3c2a73";
/**
 * Checks if the `getAllSharedMetadata` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getAllSharedMetadata` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isGetAllSharedMetadataSupported } from "thirdweb/extensions/erc721";
 * const supported = isGetAllSharedMetadataSupported(["0x..."]);
 * ```
 */
export declare function isGetAllSharedMetadataSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getAllSharedMetadata function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeGetAllSharedMetadataResult } from "thirdweb/extensions/erc721";
 * const result = decodeGetAllSharedMetadataResultResult("...");
 * ```
 */
export declare function decodeGetAllSharedMetadataResult(result: Hex): readonly {
    id: `0x${string}`;
    metadata: {
        name: string;
        description: string;
        imageURI: string;
        animationURI: string;
    };
}[];
/**
 * Calls the "getAllSharedMetadata" function on the contract.
 * @param options - The options for the getAllSharedMetadata function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { getAllSharedMetadata } from "thirdweb/extensions/erc721";
 *
 * const result = await getAllSharedMetadata({
 *  contract,
 * });
 *
 * ```
 */
export declare function getAllSharedMetadata(options: BaseTransactionOptions): Promise<readonly {
    id: `0x${string}`;
    metadata: {
        name: string;
        description: string;
        imageURI: string;
        animationURI: string;
    };
}[]>;
//# sourceMappingURL=getAllSharedMetadata.d.ts.map