import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "released" function.
 */
export type ReleasedParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0x9852595c";
/**
 * Checks if the `released` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `released` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isReleasedSupported } from "thirdweb/extensions/split";
 * const supported = isReleasedSupported(["0x..."]);
 * ```
 */
export declare function isReleasedSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "released" function.
 * @param options - The options for the released function.
 * @returns The encoded ABI parameters.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeReleasedParams } from "thirdweb/extensions/split";
 * const result = encodeReleasedParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeReleasedParams(options: ReleasedParams): `0x${string}`;
/**
 * Encodes the "released" function into a Hex string with its parameters.
 * @param options - The options for the released function.
 * @returns The encoded hexadecimal string.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeReleased } from "thirdweb/extensions/split";
 * const result = encodeReleased({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeReleased(options: ReleasedParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the released function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension SPLIT
 * @example
 * ```ts
 * import { decodeReleasedResult } from "thirdweb/extensions/split";
 * const result = decodeReleasedResultResult("...");
 * ```
 */
export declare function decodeReleasedResult(result: Hex): bigint;
/**
 * Calls the "released" function on the contract.
 * @param options - The options for the released function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { released } from "thirdweb/extensions/split";
 *
 * const result = await released({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function released(options: BaseTransactionOptions<ReleasedParams>): Promise<bigint>;
//# sourceMappingURL=released.d.ts.map