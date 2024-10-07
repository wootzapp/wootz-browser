import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getContentURI" function.
 */
export type GetContentURIParams = {
    profileId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "profileId";
    }>;
    pubId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "pubId";
    }>;
};
export declare const FN_SELECTOR: "0xb5a31496";
/**
 * Checks if the `getContentURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getContentURI` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetContentURISupported } from "thirdweb/extensions/lens";
 * const supported = isGetContentURISupported(["0x..."]);
 * ```
 */
export declare function isGetContentURISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getContentURI" function.
 * @param options - The options for the getContentURI function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetContentURIParams } from "thirdweb/extensions/lens";
 * const result = encodeGetContentURIParams({
 *  profileId: ...,
 *  pubId: ...,
 * });
 * ```
 */
export declare function encodeGetContentURIParams(options: GetContentURIParams): `0x${string}`;
/**
 * Encodes the "getContentURI" function into a Hex string with its parameters.
 * @param options - The options for the getContentURI function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetContentURI } from "thirdweb/extensions/lens";
 * const result = encodeGetContentURI({
 *  profileId: ...,
 *  pubId: ...,
 * });
 * ```
 */
export declare function encodeGetContentURI(options: GetContentURIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getContentURI function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetContentURIResult } from "thirdweb/extensions/lens";
 * const result = decodeGetContentURIResultResult("...");
 * ```
 */
export declare function decodeGetContentURIResult(result: Hex): string;
/**
 * Calls the "getContentURI" function on the contract.
 * @param options - The options for the getContentURI function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getContentURI } from "thirdweb/extensions/lens";
 *
 * const result = await getContentURI({
 *  contract,
 *  profileId: ...,
 *  pubId: ...,
 * });
 *
 * ```
 */
export declare function getContentURI(options: BaseTransactionOptions<GetContentURIParams>): Promise<string>;
//# sourceMappingURL=getContentURI.d.ts.map