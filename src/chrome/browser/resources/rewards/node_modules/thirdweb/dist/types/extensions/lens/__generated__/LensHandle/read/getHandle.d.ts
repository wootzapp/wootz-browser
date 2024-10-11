import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getHandle" function.
 */
export type GetHandleParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
};
export declare const FN_SELECTOR: "0xec81d194";
/**
 * Checks if the `getHandle` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getHandle` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetHandleSupported } from "thirdweb/extensions/lens";
 * const supported = isGetHandleSupported(["0x..."]);
 * ```
 */
export declare function isGetHandleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getHandle" function.
 * @param options - The options for the getHandle function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetHandleParams } from "thirdweb/extensions/lens";
 * const result = encodeGetHandleParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeGetHandleParams(options: GetHandleParams): `0x${string}`;
/**
 * Encodes the "getHandle" function into a Hex string with its parameters.
 * @param options - The options for the getHandle function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetHandle } from "thirdweb/extensions/lens";
 * const result = encodeGetHandle({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeGetHandle(options: GetHandleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getHandle function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetHandleResult } from "thirdweb/extensions/lens";
 * const result = decodeGetHandleResultResult("...");
 * ```
 */
export declare function decodeGetHandleResult(result: Hex): string;
/**
 * Calls the "getHandle" function on the contract.
 * @param options - The options for the getHandle function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getHandle } from "thirdweb/extensions/lens";
 *
 * const result = await getHandle({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function getHandle(options: BaseTransactionOptions<GetHandleParams>): Promise<string>;
//# sourceMappingURL=getHandle.d.ts.map