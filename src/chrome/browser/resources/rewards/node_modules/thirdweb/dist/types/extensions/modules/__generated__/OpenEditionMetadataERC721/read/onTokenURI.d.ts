import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "onTokenURI" function.
 */
export type OnTokenURIParams = {
    id: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_id";
    }>;
};
export declare const FN_SELECTOR: "0xcfc0cb96";
/**
 * Checks if the `onTokenURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `onTokenURI` method is supported.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 * const supported = OpenEditionMetadataERC721.isOnTokenURISupported(["0x..."]);
 * ```
 */
export declare function isOnTokenURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "onTokenURI" function.
 * @param options - The options for the onTokenURI function.
 * @returns The encoded ABI parameters.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 * const result = OpenEditionMetadataERC721.encodeOnTokenURIParams({
 *  id: ...,
 * });
 * ```
 */
export declare function encodeOnTokenURIParams(options: OnTokenURIParams): `0x${string}`;
/**
 * Encodes the "onTokenURI" function into a Hex string with its parameters.
 * @param options - The options for the onTokenURI function.
 * @returns The encoded hexadecimal string.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 * const result = OpenEditionMetadataERC721.encodeOnTokenURI({
 *  id: ...,
 * });
 * ```
 */
export declare function encodeOnTokenURI(options: OnTokenURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the onTokenURI function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 * const result = OpenEditionMetadataERC721.decodeOnTokenURIResultResult("...");
 * ```
 */
export declare function decodeOnTokenURIResult(result: Hex): string;
/**
 * Calls the "onTokenURI" function on the contract.
 * @param options - The options for the onTokenURI function.
 * @returns The parsed result of the function call.
 * @modules OpenEditionMetadataERC721
 * @example
 * ```ts
 * import { OpenEditionMetadataERC721 } from "thirdweb/modules";
 *
 * const result = await OpenEditionMetadataERC721.onTokenURI({
 *  contract,
 *  id: ...,
 * });
 *
 * ```
 */
export declare function onTokenURI(options: BaseTransactionOptions<OnTokenURIParams>): Promise<string>;
//# sourceMappingURL=onTokenURI.d.ts.map