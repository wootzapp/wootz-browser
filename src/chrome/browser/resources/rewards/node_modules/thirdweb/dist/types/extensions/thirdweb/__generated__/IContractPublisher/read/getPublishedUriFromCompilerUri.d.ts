import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getPublishedUriFromCompilerUri" function.
 */
export type GetPublishedUriFromCompilerUriParams = {
    compilerMetadataUri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "compilerMetadataUri";
    }>;
};
export declare const FN_SELECTOR: "0x819e992f";
/**
 * Checks if the `getPublishedUriFromCompilerUri` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getPublishedUriFromCompilerUri` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isGetPublishedUriFromCompilerUriSupported } from "thirdweb/extensions/thirdweb";
 * const supported = isGetPublishedUriFromCompilerUriSupported(["0x..."]);
 * ```
 */
export declare function isGetPublishedUriFromCompilerUriSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getPublishedUriFromCompilerUri" function.
 * @param options - The options for the getPublishedUriFromCompilerUri function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetPublishedUriFromCompilerUriParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetPublishedUriFromCompilerUriParams({
 *  compilerMetadataUri: ...,
 * });
 * ```
 */
export declare function encodeGetPublishedUriFromCompilerUriParams(options: GetPublishedUriFromCompilerUriParams): `0x${string}`;
/**
 * Encodes the "getPublishedUriFromCompilerUri" function into a Hex string with its parameters.
 * @param options - The options for the getPublishedUriFromCompilerUri function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetPublishedUriFromCompilerUri } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetPublishedUriFromCompilerUri({
 *  compilerMetadataUri: ...,
 * });
 * ```
 */
export declare function encodeGetPublishedUriFromCompilerUri(options: GetPublishedUriFromCompilerUriParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getPublishedUriFromCompilerUri function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeGetPublishedUriFromCompilerUriResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeGetPublishedUriFromCompilerUriResultResult("...");
 * ```
 */
export declare function decodeGetPublishedUriFromCompilerUriResult(result: Hex): readonly string[];
/**
 * Calls the "getPublishedUriFromCompilerUri" function on the contract.
 * @param options - The options for the getPublishedUriFromCompilerUri function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getPublishedUriFromCompilerUri } from "thirdweb/extensions/thirdweb";
 *
 * const result = await getPublishedUriFromCompilerUri({
 *  contract,
 *  compilerMetadataUri: ...,
 * });
 *
 * ```
 */
export declare function getPublishedUriFromCompilerUri(options: BaseTransactionOptions<GetPublishedUriFromCompilerUriParams>): Promise<readonly string[]>;
//# sourceMappingURL=getPublishedUriFromCompilerUri.d.ts.map