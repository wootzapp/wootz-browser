import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getMetadataUri" function.
 */
export type GetMetadataUriParams = {
    chainId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_chainId";
    }>;
    deployment: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_deployment";
    }>;
};
export declare const FN_SELECTOR: "0xf4c2012d";
/**
 * Checks if the `getMetadataUri` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getMetadataUri` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isGetMetadataUriSupported } from "thirdweb/extensions/thirdweb";
 * const supported = isGetMetadataUriSupported(["0x..."]);
 * ```
 */
export declare function isGetMetadataUriSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getMetadataUri" function.
 * @param options - The options for the getMetadataUri function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetMetadataUriParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetMetadataUriParams({
 *  chainId: ...,
 *  deployment: ...,
 * });
 * ```
 */
export declare function encodeGetMetadataUriParams(options: GetMetadataUriParams): `0x${string}`;
/**
 * Encodes the "getMetadataUri" function into a Hex string with its parameters.
 * @param options - The options for the getMetadataUri function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetMetadataUri } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetMetadataUri({
 *  chainId: ...,
 *  deployment: ...,
 * });
 * ```
 */
export declare function encodeGetMetadataUri(options: GetMetadataUriParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getMetadataUri function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeGetMetadataUriResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeGetMetadataUriResultResult("...");
 * ```
 */
export declare function decodeGetMetadataUriResult(result: Hex): string;
/**
 * Calls the "getMetadataUri" function on the contract.
 * @param options - The options for the getMetadataUri function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getMetadataUri } from "thirdweb/extensions/thirdweb";
 *
 * const result = await getMetadataUri({
 *  contract,
 *  chainId: ...,
 *  deployment: ...,
 * });
 *
 * ```
 */
export declare function getMetadataUri(options: BaseTransactionOptions<GetMetadataUriParams>): Promise<string>;
//# sourceMappingURL=getMetadataUri.d.ts.map