import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesOnInstall" function.
 */
export type EncodeBytesOnInstallParams = {
    royaltyRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "royaltyRecipient";
    }>;
    royaltyBps: AbiParameterToPrimitiveType<{
        type: "uint16";
        name: "royaltyBps";
    }>;
    transferValidator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "transferValidator";
    }>;
};
export declare const FN_SELECTOR: "0x2fbb2623";
/**
 * Encodes the parameters for the "encodeBytesOnInstall" function.
 * @param options - The options for the encodeBytesOnInstall function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesOnInstallParams } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesOnInstallParams({
 *  royaltyRecipient: ...,
 *  royaltyBps: ...,
 *  transferValidator: ...,
 * });
 * ```
 */
export declare function encodeBytesOnInstallParams(options: EncodeBytesOnInstallParams): `0x${string}`;
//# sourceMappingURL=encodeBytesOnInstall.d.ts.map