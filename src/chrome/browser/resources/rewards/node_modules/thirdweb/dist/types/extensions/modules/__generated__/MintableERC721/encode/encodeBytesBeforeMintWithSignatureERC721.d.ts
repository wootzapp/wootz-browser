import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesBeforeMintWithSignatureERC721" function.
 */
export type EncodeBytesBeforeMintWithSignatureERC721Params = {
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
                name: "pricePerUnit";
            },
            {
                type: "bytes32";
                name: "uid";
            }
        ];
    }>;
};
export declare const FN_SELECTOR: "0xcab11f42";
/**
 * Encodes the parameters for the "encodeBytesBeforeMintWithSignatureERC721" function.
 * @param options - The options for the encodeBytesBeforeMintWithSignatureERC721 function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesBeforeMintWithSignatureERC721Params } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesBeforeMintWithSignatureERC721Params({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeBytesBeforeMintWithSignatureERC721Params(options: EncodeBytesBeforeMintWithSignatureERC721Params): `0x${string}`;
//# sourceMappingURL=encodeBytesBeforeMintWithSignatureERC721.d.ts.map