import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "previewMint" function.
 */
export type PreviewMintParams = {
    shares: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "shares";
    }>;
};
export declare const FN_SELECTOR: "0xb3d7f6b9";
/**
 * Checks if the `previewMint` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `previewMint` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isPreviewMintSupported } from "thirdweb/extensions/erc4626";
 * const supported = isPreviewMintSupported(["0x..."]);
 * ```
 */
export declare function isPreviewMintSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "previewMint" function.
 * @param options - The options for the previewMint function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewMintParams } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewMintParams({
 *  shares: ...,
 * });
 * ```
 */
export declare function encodePreviewMintParams(options: PreviewMintParams): `0x${string}`;
/**
 * Encodes the "previewMint" function into a Hex string with its parameters.
 * @param options - The options for the previewMint function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewMint } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewMint({
 *  shares: ...,
 * });
 * ```
 */
export declare function encodePreviewMint(options: PreviewMintParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the previewMint function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodePreviewMintResult } from "thirdweb/extensions/erc4626";
 * const result = decodePreviewMintResultResult("...");
 * ```
 */
export declare function decodePreviewMintResult(result: Hex): bigint;
/**
 * Calls the "previewMint" function on the contract.
 * @param options - The options for the previewMint function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { previewMint } from "thirdweb/extensions/erc4626";
 *
 * const result = await previewMint({
 *  contract,
 *  shares: ...,
 * });
 *
 * ```
 */
export declare function previewMint(options: BaseTransactionOptions<PreviewMintParams>): Promise<bigint>;
//# sourceMappingURL=previewMint.d.ts.map