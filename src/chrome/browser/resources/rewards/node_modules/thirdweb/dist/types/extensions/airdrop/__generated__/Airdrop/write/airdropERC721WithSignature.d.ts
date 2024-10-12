import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "airdropERC721WithSignature" function.
 */
export type AirdropERC721WithSignatureParams = WithOverrides<{
    req: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "req";
        components: [
            {
                type: "bytes32";
                name: "uid";
            },
            {
                type: "address";
                name: "tokenAddress";
            },
            {
                type: "uint256";
                name: "expirationTimestamp";
            },
            {
                type: "tuple[]";
                name: "contents";
                components: [
                    {
                        type: "address";
                        name: "recipient";
                    },
                    {
                        type: "uint256";
                        name: "tokenId";
                    }
                ];
            }
        ];
    }>;
    signature: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "signature";
    }>;
}>;
export declare const FN_SELECTOR: "0xb654a6f3";
/**
 * Checks if the `airdropERC721WithSignature` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `airdropERC721WithSignature` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isAirdropERC721WithSignatureSupported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isAirdropERC721WithSignatureSupported(["0x..."]);
 * ```
 */
export declare function isAirdropERC721WithSignatureSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "airdropERC721WithSignature" function.
 * @param options - The options for the airdropERC721WithSignature function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC721WithSignatureParams } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC721WithSignatureParams({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC721WithSignatureParams(options: AirdropERC721WithSignatureParams): `0x${string}`;
/**
 * Encodes the "airdropERC721WithSignature" function into a Hex string with its parameters.
 * @param options - The options for the airdropERC721WithSignature function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC721WithSignature } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC721WithSignature({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC721WithSignature(options: AirdropERC721WithSignatureParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "airdropERC721WithSignature" function on the contract.
 * @param options - The options for the "airdropERC721WithSignature" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { airdropERC721WithSignature } from "thirdweb/extensions/airdrop";
 *
 * const transaction = airdropERC721WithSignature({
 *  contract,
 *  req: ...,
 *  signature: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function airdropERC721WithSignature(options: BaseTransactionOptions<AirdropERC721WithSignatureParams | {
    asyncParams: () => Promise<AirdropERC721WithSignatureParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=airdropERC721WithSignature.d.ts.map