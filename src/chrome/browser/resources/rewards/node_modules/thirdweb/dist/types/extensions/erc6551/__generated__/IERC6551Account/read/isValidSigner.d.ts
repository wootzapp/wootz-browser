import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isValidSigner" function.
 */
export type IsValidSignerParams = {
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
    }>;
    context: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "context";
    }>;
};
export declare const FN_SELECTOR: "0x523e3260";
/**
 * Checks if the `isValidSigner` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isValidSigner` method is supported.
 * @extension ERC6551
 * @example
 * ```ts
 * import { isIsValidSignerSupported } from "thirdweb/extensions/erc6551";
 * const supported = isIsValidSignerSupported(["0x..."]);
 * ```
 */
export declare function isIsValidSignerSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isValidSigner" function.
 * @param options - The options for the isValidSigner function.
 * @returns The encoded ABI parameters.
 * @extension ERC6551
 * @example
 * ```ts
 * import { encodeIsValidSignerParams } from "thirdweb/extensions/erc6551";
 * const result = encodeIsValidSignerParams({
 *  signer: ...,
 *  context: ...,
 * });
 * ```
 */
export declare function encodeIsValidSignerParams(options: IsValidSignerParams): `0x${string}`;
/**
 * Encodes the "isValidSigner" function into a Hex string with its parameters.
 * @param options - The options for the isValidSigner function.
 * @returns The encoded hexadecimal string.
 * @extension ERC6551
 * @example
 * ```ts
 * import { encodeIsValidSigner } from "thirdweb/extensions/erc6551";
 * const result = encodeIsValidSigner({
 *  signer: ...,
 *  context: ...,
 * });
 * ```
 */
export declare function encodeIsValidSigner(options: IsValidSignerParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isValidSigner function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC6551
 * @example
 * ```ts
 * import { decodeIsValidSignerResult } from "thirdweb/extensions/erc6551";
 * const result = decodeIsValidSignerResultResult("...");
 * ```
 */
export declare function decodeIsValidSignerResult(result: Hex): `0x${string}`;
/**
 * Calls the "isValidSigner" function on the contract.
 * @param options - The options for the isValidSigner function.
 * @returns The parsed result of the function call.
 * @extension ERC6551
 * @example
 * ```ts
 * import { isValidSigner } from "thirdweb/extensions/erc6551";
 *
 * const result = await isValidSigner({
 *  contract,
 *  signer: ...,
 *  context: ...,
 * });
 *
 * ```
 */
export declare function isValidSigner(options: BaseTransactionOptions<IsValidSignerParams>): Promise<`0x${string}`>;
//# sourceMappingURL=isValidSigner.d.ts.map