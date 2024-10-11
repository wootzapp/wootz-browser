import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "previewWithdraw" function.
 */
export type PreviewWithdrawParams = {
    assets: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "assets";
    }>;
};
export declare const FN_SELECTOR: "0x0a28a477";
/**
 * Checks if the `previewWithdraw` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `previewWithdraw` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isPreviewWithdrawSupported } from "thirdweb/extensions/erc4626";
 * const supported = isPreviewWithdrawSupported(["0x..."]);
 * ```
 */
export declare function isPreviewWithdrawSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "previewWithdraw" function.
 * @param options - The options for the previewWithdraw function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewWithdrawParams } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewWithdrawParams({
 *  assets: ...,
 * });
 * ```
 */
export declare function encodePreviewWithdrawParams(options: PreviewWithdrawParams): `0x${string}`;
/**
 * Encodes the "previewWithdraw" function into a Hex string with its parameters.
 * @param options - The options for the previewWithdraw function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewWithdraw } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewWithdraw({
 *  assets: ...,
 * });
 * ```
 */
export declare function encodePreviewWithdraw(options: PreviewWithdrawParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the previewWithdraw function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodePreviewWithdrawResult } from "thirdweb/extensions/erc4626";
 * const result = decodePreviewWithdrawResultResult("...");
 * ```
 */
export declare function decodePreviewWithdrawResult(result: Hex): bigint;
/**
 * Calls the "previewWithdraw" function on the contract.
 * @param options - The options for the previewWithdraw function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { previewWithdraw } from "thirdweb/extensions/erc4626";
 *
 * const result = await previewWithdraw({
 *  contract,
 *  assets: ...,
 * });
 *
 * ```
 */
export declare function previewWithdraw(options: BaseTransactionOptions<PreviewWithdrawParams>): Promise<bigint>;
//# sourceMappingURL=previewWithdraw.d.ts.map