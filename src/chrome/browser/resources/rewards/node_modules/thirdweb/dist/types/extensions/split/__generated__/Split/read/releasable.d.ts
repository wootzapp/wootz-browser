import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "releasable" function.
 */
export type ReleasableParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0xa3f8eace";
/**
 * Checks if the `releasable` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `releasable` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isReleasableSupported } from "thirdweb/extensions/split";
 * const supported = isReleasableSupported(["0x..."]);
 * ```
 */
export declare function isReleasableSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "releasable" function.
 * @param options - The options for the releasable function.
 * @returns The encoded ABI parameters.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeReleasableParams } from "thirdweb/extensions/split";
 * const result = encodeReleasableParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeReleasableParams(options: ReleasableParams): `0x${string}`;
/**
 * Encodes the "releasable" function into a Hex string with its parameters.
 * @param options - The options for the releasable function.
 * @returns The encoded hexadecimal string.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeReleasable } from "thirdweb/extensions/split";
 * const result = encodeReleasable({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeReleasable(options: ReleasableParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the releasable function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension SPLIT
 * @example
 * ```ts
 * import { decodeReleasableResult } from "thirdweb/extensions/split";
 * const result = decodeReleasableResultResult("...");
 * ```
 */
export declare function decodeReleasableResult(result: Hex): bigint;
/**
 * Calls the "releasable" function on the contract.
 * @param options - The options for the releasable function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { releasable } from "thirdweb/extensions/split";
 *
 * const result = await releasable({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function releasable(options: BaseTransactionOptions<ReleasableParams>): Promise<bigint>;
//# sourceMappingURL=releasable.d.ts.map