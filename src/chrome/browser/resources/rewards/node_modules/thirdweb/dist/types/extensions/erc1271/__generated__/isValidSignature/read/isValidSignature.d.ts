import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isValidSignature" function.
 */
export type IsValidSignatureParams = {
    hash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "hash";
    }>;
    signature: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "signature";
    }>;
};
export declare const FN_SELECTOR: "0x1626ba7e";
/**
 * Checks if the `isValidSignature` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isValidSignature` method is supported.
 * @extension ERC1271
 * @example
 * ```ts
 * import { isIsValidSignatureSupported } from "thirdweb/extensions/erc1271";
 * const supported = isIsValidSignatureSupported(["0x..."]);
 * ```
 */
export declare function isIsValidSignatureSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isValidSignature" function.
 * @param options - The options for the isValidSignature function.
 * @returns The encoded ABI parameters.
 * @extension ERC1271
 * @example
 * ```ts
 * import { encodeIsValidSignatureParams } from "thirdweb/extensions/erc1271";
 * const result = encodeIsValidSignatureParams({
 *  hash: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeIsValidSignatureParams(options: IsValidSignatureParams): `0x${string}`;
/**
 * Encodes the "isValidSignature" function into a Hex string with its parameters.
 * @param options - The options for the isValidSignature function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1271
 * @example
 * ```ts
 * import { encodeIsValidSignature } from "thirdweb/extensions/erc1271";
 * const result = encodeIsValidSignature({
 *  hash: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeIsValidSignature(options: IsValidSignatureParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isValidSignature function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1271
 * @example
 * ```ts
 * import { decodeIsValidSignatureResult } from "thirdweb/extensions/erc1271";
 * const result = decodeIsValidSignatureResultResult("...");
 * ```
 */
export declare function decodeIsValidSignatureResult(result: Hex): `0x${string}`;
/**
 * Calls the "isValidSignature" function on the contract.
 * @param options - The options for the isValidSignature function.
 * @returns The parsed result of the function call.
 * @extension ERC1271
 * @example
 * ```ts
 * import { isValidSignature } from "thirdweb/extensions/erc1271";
 *
 * const result = await isValidSignature({
 *  contract,
 *  hash: ...,
 *  signature: ...,
 * });
 *
 * ```
 */
export declare function isValidSignature(options: BaseTransactionOptions<IsValidSignatureParams>): Promise<`0x${string}`>;
//# sourceMappingURL=isValidSignature.d.ts.map