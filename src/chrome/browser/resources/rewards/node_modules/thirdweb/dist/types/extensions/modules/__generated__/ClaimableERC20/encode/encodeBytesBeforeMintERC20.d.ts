import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the parameters for the "encodeBytesBeforeMintERC20" function.
 */
export type EncodeBytesBeforeMintERC20Params = {
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
export declare const FN_SELECTOR: "0x4e6030da";
/**
 * Encodes the parameters for the "encodeBytesBeforeMintERC20" function.
 * @param options - The options for the encodeBytesBeforeMintERC20 function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeEncodeBytesBeforeMintERC20Params } "thirdweb/extensions/modules";
 * const result = encodeEncodeBytesBeforeMintERC20Params({
 *  params: ...,
 * });
 * ```
 */
export declare function encodeBytesBeforeMintERC20Params(options: EncodeBytesBeforeMintERC20Params): `0x${string}`;
//# sourceMappingURL=encodeBytesBeforeMintERC20.d.ts.map