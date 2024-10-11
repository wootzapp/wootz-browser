import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getProfileIdAllowedToRecover" function.
 */
export type GetProfileIdAllowedToRecoverParams = {
    followTokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "followTokenId";
    }>;
};
export declare const FN_SELECTOR: "0x2af1544f";
/**
 * Checks if the `getProfileIdAllowedToRecover` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getProfileIdAllowedToRecover` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isGetProfileIdAllowedToRecoverSupported } from "thirdweb/extensions/lens";
 * const supported = isGetProfileIdAllowedToRecoverSupported(["0x..."]);
 * ```
 */
export declare function isGetProfileIdAllowedToRecoverSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getProfileIdAllowedToRecover" function.
 * @param options - The options for the getProfileIdAllowedToRecover function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetProfileIdAllowedToRecoverParams } from "thirdweb/extensions/lens";
 * const result = encodeGetProfileIdAllowedToRecoverParams({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetProfileIdAllowedToRecoverParams(options: GetProfileIdAllowedToRecoverParams): `0x${string}`;
/**
 * Encodes the "getProfileIdAllowedToRecover" function into a Hex string with its parameters.
 * @param options - The options for the getProfileIdAllowedToRecover function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeGetProfileIdAllowedToRecover } from "thirdweb/extensions/lens";
 * const result = encodeGetProfileIdAllowedToRecover({
 *  followTokenId: ...,
 * });
 * ```
 */
export declare function encodeGetProfileIdAllowedToRecover(options: GetProfileIdAllowedToRecoverParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getProfileIdAllowedToRecover function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeGetProfileIdAllowedToRecoverResult } from "thirdweb/extensions/lens";
 * const result = decodeGetProfileIdAllowedToRecoverResultResult("...");
 * ```
 */
export declare function decodeGetProfileIdAllowedToRecoverResult(result: Hex): bigint;
/**
 * Calls the "getProfileIdAllowedToRecover" function on the contract.
 * @param options - The options for the getProfileIdAllowedToRecover function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { getProfileIdAllowedToRecover } from "thirdweb/extensions/lens";
 *
 * const result = await getProfileIdAllowedToRecover({
 *  contract,
 *  followTokenId: ...,
 * });
 *
 * ```
 */
export declare function getProfileIdAllowedToRecover(options: BaseTransactionOptions<GetProfileIdAllowedToRecoverParams>): Promise<bigint>;
//# sourceMappingURL=getProfileIdAllowedToRecover.d.ts.map