import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getLocalName" function.
 */
export type GetLocalNameParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
};
export declare const FN_SELECTOR: "0x4985e504";
/**
 * Checks if the `getLocalName` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getLocalName` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetLocalNameSupported } from "thirdweb/extensions/lens";
 * const supported = isGetLocalNameSupported(["0x..."]);
 * ```
 */
export declare function isGetLocalNameSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getLocalName" function.
 * @param options - The options for the getLocalName function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetLocalNameParams } from "thirdweb/extensions/lens";
 * const result = encodeGetLocalNameParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeGetLocalNameParams(options: GetLocalNameParams): `0x${string}`;
/**
 * Encodes the "getLocalName" function into a Hex string with its parameters.
 * @param options - The options for the getLocalName function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetLocalName } from "thirdweb/extensions/lens";
 * const result = encodeGetLocalName({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeGetLocalName(options: GetLocalNameParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getLocalName function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetLocalNameResult } from "thirdweb/extensions/lens";
 * const result = decodeGetLocalNameResultResult("...");
 * ```
 */
export declare function decodeGetLocalNameResult(result: Hex): string;
/**
 * Calls the "getLocalName" function on the contract.
 * @param options - The options for the getLocalName function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getLocalName } from "thirdweb/extensions/lens";
 *
 * const result = await getLocalName({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function getLocalName(options: BaseTransactionOptions<GetLocalNameParams>): Promise<string>;
//# sourceMappingURL=getLocalName.d.ts.map