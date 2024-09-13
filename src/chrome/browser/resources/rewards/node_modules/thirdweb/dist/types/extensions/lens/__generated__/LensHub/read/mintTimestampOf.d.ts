import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "mintTimestampOf" function.
 */
export type MintTimestampOfParams = {
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
};
export declare const FN_SELECTOR: "0x50ddf35c";
/**
 * Checks if the `mintTimestampOf` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `mintTimestampOf` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isMintTimestampOfSupported } from "thirdweb/extensions/lens";
 * const supported = isMintTimestampOfSupported(["0x..."]);
 * ```
 */
export declare function isMintTimestampOfSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "mintTimestampOf" function.
 * @param options - The options for the mintTimestampOf function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeMintTimestampOfParams } from "thirdweb/extensions/lens";
 * const result = encodeMintTimestampOfParams({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeMintTimestampOfParams(options: MintTimestampOfParams): `0x${string}`;
/**
 * Encodes the "mintTimestampOf" function into a Hex string with its parameters.
 * @param options - The options for the mintTimestampOf function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeMintTimestampOf } from "thirdweb/extensions/lens";
 * const result = encodeMintTimestampOf({
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeMintTimestampOf(options: MintTimestampOfParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the mintTimestampOf function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeMintTimestampOfResult } from "thirdweb/extensions/lens";
 * const result = decodeMintTimestampOfResultResult("...");
 * ```
 */
export declare function decodeMintTimestampOfResult(result: Hex): bigint;
/**
 * Calls the "mintTimestampOf" function on the contract.
 * @param options - The options for the mintTimestampOf function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { mintTimestampOf } from "thirdweb/extensions/lens";
 *
 * const result = await mintTimestampOf({
 *  contract,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function mintTimestampOf(options: BaseTransactionOptions<MintTimestampOfParams>): Promise<bigint>;
//# sourceMappingURL=mintTimestampOf.d.ts.map