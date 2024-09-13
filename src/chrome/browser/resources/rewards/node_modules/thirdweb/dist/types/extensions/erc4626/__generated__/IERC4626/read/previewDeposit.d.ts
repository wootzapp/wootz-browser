import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "previewDeposit" function.
 */
export type PreviewDepositParams = {
    assets: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "assets";
    }>;
};
export declare const FN_SELECTOR: "0xef8b30f7";
/**
 * Checks if the `previewDeposit` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `previewDeposit` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isPreviewDepositSupported } from "thirdweb/extensions/erc4626";
 * const supported = isPreviewDepositSupported(["0x..."]);
 * ```
 */
export declare function isPreviewDepositSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "previewDeposit" function.
 * @param options - The options for the previewDeposit function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewDepositParams } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewDepositParams({
 *  assets: ...,
 * });
 * ```
 */
export declare function encodePreviewDepositParams(options: PreviewDepositParams): `0x${string}`;
/**
 * Encodes the "previewDeposit" function into a Hex string with its parameters.
 * @param options - The options for the previewDeposit function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodePreviewDeposit } from "thirdweb/extensions/erc4626";
 * const result = encodePreviewDeposit({
 *  assets: ...,
 * });
 * ```
 */
export declare function encodePreviewDeposit(options: PreviewDepositParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the previewDeposit function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodePreviewDepositResult } from "thirdweb/extensions/erc4626";
 * const result = decodePreviewDepositResultResult("...");
 * ```
 */
export declare function decodePreviewDepositResult(result: Hex): bigint;
/**
 * Calls the "previewDeposit" function on the contract.
 * @param options - The options for the previewDeposit function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { previewDeposit } from "thirdweb/extensions/erc4626";
 *
 * const result = await previewDeposit({
 *  contract,
 *  assets: ...,
 * });
 *
 * ```
 */
export declare function previewDeposit(options: BaseTransactionOptions<PreviewDepositParams>): Promise<bigint>;
//# sourceMappingURL=previewDeposit.d.ts.map