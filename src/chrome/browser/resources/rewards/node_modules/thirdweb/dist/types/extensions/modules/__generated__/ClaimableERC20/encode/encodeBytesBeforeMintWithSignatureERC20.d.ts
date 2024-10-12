import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesBeforeMintWithSignatureERC20" function.
 */
export type EncodeBytesBeforeMintWithSignatureERC20Params = {
    params: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "params";
        components: [
            {
                type: "uint48";
                name: "startTimestamp";
            },
            {
                type: "uint48";
                name: "endTimestamp";
            },
            {
                type: "address";
                name: "currency";
            },
            {
                type: "uint256";
                name: "maxMintPerWallet";
            },
            {
                type: "uint256";
                name: "pricePerUnit";
            },
            {
                type: "bytes32";
                name: "uid";
            }
        ];
    }>;
};
export declare const FN_SELECTOR: "0x3f4a1bb6";
/**
 * Encodes the parameters for the "encodeBytesBeforeMintWithSignatureERC20" function.
 * @param options - The options for the encodeBytesBeforeMintWithSignatureERC20 function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesBeforeMintWithSignatureERC20Params } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesBeforeMintWithSignatureERC20Params({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeBytesBeforeMintWithSignatureERC20Params(options: EncodeBytesBeforeMintWithSignatureERC20Params): `0x${string}`;
//# sourceMappingURL=encodeBytesBeforeMintWithSignatureERC20.d.ts.map