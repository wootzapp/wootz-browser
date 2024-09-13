import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesBeforeMintERC721" function.
 */
export type EncodeBytesBeforeMintERC721Params = {
    params: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "params";
        components: [
            {
                type: "address";
                name: "currency";
            },
            {
                type: "uint256";
                name: "pricePerUnit";
            },
            {
                type: "bytes32[]";
                name: "recipientAllowlistProof";
            }
        ];
    }>;
};
export declare const FN_SELECTOR: "0xd9584651";
/**
 * Encodes the parameters for the "encodeBytesBeforeMintERC721" function.
 * @param options - The options for the encodeBytesBeforeMintERC721 function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesBeforeMintERC721Params } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesBeforeMintERC721Params({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeBytesBeforeMintERC721Params(options: EncodeBytesBeforeMintERC721Params): `0x${string}`;
//# sourceMappingURL=encodeBytesBeforeMintERC721.d.ts.map