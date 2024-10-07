import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "verifyFidSignature" function.
 */
export type VerifyFidSignatureParams = {
    custodyAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "custodyAddress";
    }>;
    fid: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fid";
    }>;
    digest: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "digest";
    }>;
    sig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "sig";
    }>;
};
export declare const FN_SELECTOR: "0x32faac70";
/**
 * Checks if the `verifyFidSignature` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `verifyFidSignature` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isVerifyFidSignatureSupported } from "thirdweb/extensions/farcaster";
 * const supported = isVerifyFidSignatureSupported(["0x..."]);
 * ```
 */
export declare function isVerifyFidSignatureSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "verifyFidSignature" function.
 * @param options - The options for the verifyFidSignature function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeVerifyFidSignatureParams } from "thirdweb/extensions/farcaster";
 * const result = encodeVerifyFidSignatureParams({
 *  custodyAddress: ...,
 *  fid: ...,
 *  digest: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeVerifyFidSignatureParams(options: VerifyFidSignatureParams): `0x${string}`;
/**
 * Encodes the "verifyFidSignature" function into a Hex string with its parameters.
 * @param options - The options for the verifyFidSignature function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeVerifyFidSignature } from "thirdweb/extensions/farcaster";
 * const result = encodeVerifyFidSignature({
 *  custodyAddress: ...,
 *  fid: ...,
 *  digest: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeVerifyFidSignature(options: VerifyFidSignatureParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the verifyFidSignature function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeVerifyFidSignatureResult } from "thirdweb/extensions/farcaster";
 * const result = decodeVerifyFidSignatureResultResult("...");
 * ```
 */
export declare function decodeVerifyFidSignatureResult(result: Hex): boolean;
/**
 * Calls the "verifyFidSignature" function on the contract.
 * @param options - The options for the verifyFidSignature function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { verifyFidSignature } from "thirdweb/extensions/farcaster";
 *
 * const result = await verifyFidSignature({
 *  contract,
 *  custodyAddress: ...,
 *  fid: ...,
 *  digest: ...,
 *  sig: ...,
 * });
 *
 * ```
 */
export declare function verifyFidSignature(options: BaseTransactionOptions<VerifyFidSignatureParams>): Promise<boolean>;
//# sourceMappingURL=verifyFidSignature.d.ts.map