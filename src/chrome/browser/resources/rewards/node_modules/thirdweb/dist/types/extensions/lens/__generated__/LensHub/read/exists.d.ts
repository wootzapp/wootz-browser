import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "exists" function.
 */
export type ExistsParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
};
export declare const FN_SELECTOR: "0x4f558e79";
/**
 * Checks if the `exists` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `exists` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isExistsSupported } from "thirdweb/extensions/lens";
 * const supported = isExistsSupported(["0x..."]);
 * ```
 */
export declare function isExistsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "exists" function.
 * @param options - The options for the exists function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeExistsParams } from "thirdweb/extensions/lens";
 * const result = encodeExistsParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeExistsParams(options: ExistsParams): `0x${string}`;
/**
 * Encodes the "exists" function into a Hex string with its parameters.
 * @param options - The options for the exists function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeExists } from "thirdweb/extensions/lens";
 * const result = encodeExists({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeExists(options: ExistsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the exists function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeExistsResult } from "thirdweb/extensions/lens";
 * const result = decodeExistsResultResult("...");
 * ```
 */
export declare function decodeExistsResult(result: Hex): boolean;
/**
 * Calls the "exists" function on the contract.
 * @param options - The options for the exists function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { exists } from "thirdweb/extensions/lens";
 *
 * const result = await exists({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function exists(options: BaseTransactionOptions<ExistsParams>): Promise<boolean>;
//# sourceMappingURL=exists.d.ts.map