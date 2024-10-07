import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesBeforeMintERC1155" function.
 */
export type EncodeBytesBeforeMintERC1155Params = {
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
export declare const FN_SELECTOR: "0x819ed5a3";
/**
 * Encodes the parameters for the "encodeBytesBeforeMintERC1155" function.
 * @param options - The options for the encodeBytesBeforeMintERC1155 function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesBeforeMintERC1155Params } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesBeforeMintERC1155Params({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeBytesBeforeMintERC1155Params(options: EncodeBytesBeforeMintERC1155Params): `0x${string}`;
//# sourceMappingURL=encodeBytesBeforeMintERC1155.d.ts.map