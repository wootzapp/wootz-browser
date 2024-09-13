import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "nonces" function.
 */
export type NoncesParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0x7ecebe00";
/**
 * Checks if the `nonces` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `nonces` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isNoncesSupported } from "thirdweb/extensions/farcaster";
 * const supported = isNoncesSupported(["0x..."]);
 * ```
 */
export declare function isNoncesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "nonces" function.
 * @param options - The options for the nonces function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeNoncesParams } from "thirdweb/extensions/farcaster";
 * const result = encodeNoncesParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeNoncesParams(options: NoncesParams): `0x${string}`;
/**
 * Encodes the "nonces" function into a Hex string with its parameters.
 * @param options - The options for the nonces function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeNonces } from "thirdweb/extensions/farcaster";
 * const result = encodeNonces({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeNonces(options: NoncesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the nonces function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeNoncesResult } from "thirdweb/extensions/farcaster";
 * const result = decodeNoncesResultResult("...");
 * ```
 */
export declare function decodeNoncesResult(result: Hex): bigint;
/**
 * Calls the "nonces" function on the contract.
 * @param options - The options for the nonces function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { nonces } from "thirdweb/extensions/farcaster";
 *
 * const result = await nonces({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function nonces(options: BaseTransactionOptions<NoncesParams>): Promise<bigint>;
//# sourceMappingURL=nonces.d.ts.map