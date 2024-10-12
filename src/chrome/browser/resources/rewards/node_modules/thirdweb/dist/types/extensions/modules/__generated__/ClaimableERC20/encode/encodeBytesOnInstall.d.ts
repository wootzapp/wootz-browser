import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesOnInstall" function.
 */
export type EncodeBytesOnInstallParams = {
    primarySaleRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "primarySaleRecipient";
    }>;
};
export declare const FN_SELECTOR: "0x5d4c0b89";
/**
 * Encodes the parameters for the "encodeBytesOnInstall" function.
 * @param options - The options for the encodeBytesOnInstall function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesOnInstallParams } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesOnInstallParams({
 *  primarySaleRecipient: ...,
 * });
 * ```
 */
export declare function encodeBytesOnInstallParams(options: EncodeBytesOnInstallParams): `0x${string}`;
//# sourceMappingURL=encodeBytesOnInstall.d.ts.map