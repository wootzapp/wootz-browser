import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "count" function.
 */
export type CountParams = {
    deployer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_deployer";
    }>;
};
export declare const FN_SELECTOR: "0x05d85eda";
/**
 * Checks if the `count` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `count` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isCountSupported } from "thirdweb/extensions/thirdweb";
 * const supported = isCountSupported(["0x..."]);
 * ```
 */
export declare function isCountSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "count" function.
 * @param options - The options for the count function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeCountParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeCountParams({
 *  deployer: ...,
 * });
 * ```
 */
export declare function encodeCountParams(options: CountParams): `0x${string}`;
/**
 * Encodes the "count" function into a Hex string with its parameters.
 * @param options - The options for the count function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeCount } from "thirdweb/extensions/thirdweb";
 * const result = encodeCount({
 *  deployer: ...,
 * });
 * ```
 */
export declare function encodeCount(options: CountParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the count function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeCountResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeCountResultResult("...");
 * ```
 */
export declare function decodeCountResult(result: Hex): bigint;
/**
 * Calls the "count" function on the contract.
 * @param options - The options for the count function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { count } from "thirdweb/extensions/thirdweb";
 *
 * const result = await count({
 *  contract,
 *  deployer: ...,
 * });
 *
 * ```
 */
export declare function count(options: BaseTransactionOptions<CountParams>): Promise<bigint>;
//# sourceMappingURL=count.d.ts.map