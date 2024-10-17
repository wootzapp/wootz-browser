import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getTokenId" function.
 */
export type GetTokenIdParams = {
    localName: AbiParameterToPrimitiveType<{
        type: "string";
        name: "localName";
    }>;
};
export declare const FN_SELECTOR: "0x1e7663bc";
/**
 * Checks if the `getTokenId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getTokenId` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetTokenIdSupported } from "thirdweb/extensions/lens";
 * const supported = isGetTokenIdSupported(["0x..."]);
 * ```
 */
export declare function isGetTokenIdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getTokenId" function.
 * @param options - The options for the getTokenId function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetTokenIdParams } from "thirdweb/extensions/lens";
 * const result = encodeGetTokenIdParams({
 *  localName: ...,
 * });
 * ```
 */
export declare function encodeGetTokenIdParams(options: GetTokenIdParams): `0x${string}`;
/**
 * Encodes the "getTokenId" function into a Hex string with its parameters.
 * @param options - The options for the getTokenId function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetTokenId } from "thirdweb/extensions/lens";
 * const result = encodeGetTokenId({
 *  localName: ...,
 * });
 * ```
 */
export declare function encodeGetTokenId(options: GetTokenIdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getTokenId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetTokenIdResult } from "thirdweb/extensions/lens";
 * const result = decodeGetTokenIdResultResult("...");
 * ```
 */
export declare function decodeGetTokenIdResult(result: Hex): bigint;
/**
 * Calls the "getTokenId" function on the contract.
 * @param options - The options for the getTokenId function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getTokenId } from "thirdweb/extensions/lens";
 *
 * const result = await getTokenId({
 *  contract,
 *  localName: ...,
 * });
 *
 * ```
 */
export declare function getTokenId(options: BaseTransactionOptions<GetTokenIdParams>): Promise<bigint>;
//# sourceMappingURL=getTokenId.d.ts.map