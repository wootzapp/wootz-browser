import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getScore" function.
 */
export type GetScoreParams = {
    tokenOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_tokenOwner";
    }>;
};
export declare const FN_SELECTOR: "0xd47875d0";
/**
 * Checks if the `getScore` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getScore` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isGetScoreSupported } from "thirdweb/extensions/thirdweb";
 * const supported = isGetScoreSupported(["0x..."]);
 * ```
 */
export declare function isGetScoreSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getScore" function.
 * @param options - The options for the getScore function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetScoreParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetScoreParams({
 *  tokenOwner: ...,
 * });
 * ```
 */
export declare function encodeGetScoreParams(options: GetScoreParams): `0x${string}`;
/**
 * Encodes the "getScore" function into a Hex string with its parameters.
 * @param options - The options for the getScore function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetScore } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetScore({
 *  tokenOwner: ...,
 * });
 * ```
 */
export declare function encodeGetScore(options: GetScoreParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getScore function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeGetScoreResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeGetScoreResultResult("...");
 * ```
 */
export declare function decodeGetScoreResult(result: Hex): bigint;
/**
 * Calls the "getScore" function on the contract.
 * @param options - The options for the getScore function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getScore } from "thirdweb/extensions/thirdweb";
 *
 * const result = await getScore({
 *  contract,
 *  tokenOwner: ...,
 * });
 *
 * ```
 */
export declare function getScore(options: BaseTransactionOptions<GetScoreParams>): Promise<bigint>;
//# sourceMappingURL=getScore.d.ts.map