import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "previewRedeem" function.
 */
export type PreviewRedeemParams = {
    shares: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "shares";
    }>;
};
export declare const FN_SELECTOR: "0x4cdad506";
/**
 * Checks if the `previewRedeem` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `previewRedeem` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isPreviewRedeemSupported } from "thirdweb/extensions/erc4626";
 * const supported = isPreviewRedeemSupported(["0x..."]);
 * ```
 */
export declare function isPreviewRedeemSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "previewRedeem" function.
 * @param options - The options for the previewRedeem function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewRedeemParams } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewRedeemParams({
 *  shares: ...,
 * });
 * ```
 */
export declare function encodePreviewRedeemParams(options: PreviewRedeemParams): `0x${string}`;
/**
 * Encodes the "previewRedeem" function into a Hex string with its parameters.
 * @param options - The options for the previewRedeem function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewRedeem } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewRedeem({
 *  shares: ...,
 * });
 * ```
 */
export declare function encodePreviewRedeem(options: PreviewRedeemParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the previewRedeem function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodePreviewRedeemResult } from "thirdweb/extensions/erc4626";
 * const result = decodePreviewRedeemResultResult("...");
 * ```
 */
export declare function decodePreviewRedeemResult(result: Hex): bigint;
/**
 * Calls the "previewRedeem" function on the contract.
 * @param options - The options for the previewRedeem function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { previewRedeem } from "thirdweb/extensions/erc4626";
 *
 * const result = await previewRedeem({
 *  contract,
 *  shares: ...,
 * });
 *
 * ```
 */
export declare function previewRedeem(options: BaseTransactionOptions<PreviewRedeemParams>): Promise<bigint>;
//# sourceMappingURL=previewRedeem.d.ts.map