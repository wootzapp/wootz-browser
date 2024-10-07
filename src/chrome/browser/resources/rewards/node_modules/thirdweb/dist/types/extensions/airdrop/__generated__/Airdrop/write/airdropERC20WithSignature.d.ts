import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "airdropERC20WithSignature" function.
 */
export type AirdropERC20WithSignatureParams = WithOverrides<{
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
export declare const FN_SELECTOR: "0xaaba07f6";
/**
 * Checks if the `airdropERC20WithSignature` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `airdropERC20WithSignature` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isAirdropERC20WithSignatureSupported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isAirdropERC20WithSignatureSupported(["0x..."]);
 * ```
 */
export declare function isAirdropERC20WithSignatureSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "airdropERC20WithSignature" function.
 * @param options - The options for the airdropERC20WithSignature function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC20WithSignatureParams } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC20WithSignatureParams({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC20WithSignatureParams(options: AirdropERC20WithSignatureParams): `0x${string}`;
/**
 * Encodes the "airdropERC20WithSignature" function into a Hex string with its parameters.
 * @param options - The options for the airdropERC20WithSignature function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeAirdropERC20WithSignature } from "thirdweb/extensions/airdrop";
 * const result = encodeAirdropERC20WithSignature({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC20WithSignature(options: AirdropERC20WithSignatureParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "airdropERC20WithSignature" function on the contract.
 * @param options - The options for the "airdropERC20WithSignature" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { airdropERC20WithSignature } from "thirdweb/extensions/airdrop";
 *
 * const transaction = airdropERC20WithSignature({
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
export declare function airdropERC20WithSignature(options: BaseTransactionOptions<AirdropERC20WithSignatureParams | {
    asyncParams: () => Promise<AirdropERC20WithSignatureParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=airdropERC20WithSignature.d.ts.map