import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesBeforeMintWithSignatureERC1155" function.
 */
export type EncodeBytesBeforeMintWithSignatureERC1155Params = {
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
export declare const FN_SELECTOR: "0x63dacad2";
/**
 * Encodes the parameters for the "encodeBytesBeforeMintWithSignatureERC1155" function.
 * @param options - The options for the encodeBytesBeforeMintWithSignatureERC1155 function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesBeforeMintWithSignatureERC1155Params } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesBeforeMintWithSignatureERC1155Params({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeBytesBeforeMintWithSignatureERC1155Params(options: EncodeBytesBeforeMintWithSignatureERC1155Params): `0x${string}`;
//# sourceMappingURL=encodeBytesBeforeMintWithSignatureERC1155.d.ts.map