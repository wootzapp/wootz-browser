import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "idOf" function.
 */
export type IdOfParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
};
export declare const FN_SELECTOR: "0xd94fe832";
/**
 * Checks if the `idOf` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `idOf` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isIdOfSupported } from "thirdweb/extensions/farcaster";
 * const supported = isIdOfSupported(["0x..."]);
 * ```
 */
export declare function isIdOfSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "idOf" function.
 * @param options - The options for the idOf function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeIdOfParams } from "thirdweb/extensions/farcaster";
 * const result = encodeIdOfParams({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeIdOfParams(options: IdOfParams): `0x${string}`;
/**
 * Encodes the "idOf" function into a Hex string with its parameters.
 * @param options - The options for the idOf function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeIdOf } from "thirdweb/extensions/farcaster";
 * const result = encodeIdOf({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeIdOf(options: IdOfParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the idOf function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeIdOfResult } from "thirdweb/extensions/farcaster";
 * const result = decodeIdOfResultResult("...");
 * ```
 */
export declare function decodeIdOfResult(result: Hex): bigint;
/**
 * Calls the "idOf" function on the contract.
 * @param options - The options for the idOf function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { idOf } from "thirdweb/extensions/farcaster";
 *
 * const result = await idOf({
 *  contract,
 *  owner: ...,
 * });
 *
 * ```
 */
export declare function idOf(options: BaseTransactionOptions<IdOfParams>): Promise<bigint>;
//# sourceMappingURL=idOf.d.ts.map