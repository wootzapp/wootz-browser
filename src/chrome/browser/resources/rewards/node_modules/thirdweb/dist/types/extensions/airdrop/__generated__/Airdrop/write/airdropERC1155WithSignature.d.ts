import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "airdropERC1155WithSignature" function.
 */
export type AirdropERC1155WithSignatureParams = WithOverrides<{
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
                    },
                    {
                        type: "uint256";
                        name: "amount";
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
export declare const FN_SELECTOR: "0xd0d4afd6";
/**
 * Checks if the `airdropERC1155WithSignature` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `airdropERC1155WithSignature` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isAirdropERC1155WithSignatureSupported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isAirdropERC1155WithSignatureSupported(["0x..."]);
 * ```
 */
export declare function isAirdropERC1155WithSignatureSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "airdropERC1155WithSignature" function.
 * @param options - The options for the airdropERC1155WithSignature function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC1155WithSignatureParams } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC1155WithSignatureParams({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC1155WithSignatureParams(options: AirdropERC1155WithSignatureParams): `0x${string}`;
/**
 * Encodes the "airdropERC1155WithSignature" function into a Hex string with its parameters.
 * @param options - The options for the airdropERC1155WithSignature function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC1155WithSignature } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC1155WithSignature({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC1155WithSignature(options: AirdropERC1155WithSignatureParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "airdropERC1155WithSignature" function on the contract.
 * @param options - The options for the "airdropERC1155WithSignature" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { airdropERC1155WithSignature } from "thirdweb/extensions/airdrop";
 *
 * const transaction = airdropERC1155WithSignature({
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
export declare function airdropERC1155WithSignature(options: BaseTransactionOptions<AirdropERC1155WithSignatureParams | {
    asyncParams: () => Promise<AirdropERC1155WithSignatureParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=airdropERC1155WithSignature.d.ts.map