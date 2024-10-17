import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setClaimConditionByTokenId" function.
 */
export type SetClaimConditionByTokenIdParams = WithOverrides<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    claimCondition: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "_claimCondition";
        components: [
            {
                type: "uint256";
                name: "availableSupply";
            },
            {
                type: "bytes32";
                name: "allowlistMerkleRoot";
            },
            {
                type: "uint256";
                name: "pricePerUnit";
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
                type: "uint48";
                name: "startTimestamp";
            },
            {
                type: "uint48";
                name: "endTimestamp";
            },
            {
                type: "string";
                name: "auxData";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x3bcec708";
/**
 * Checks if the `setClaimConditionByTokenId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setClaimConditionByTokenId` method is supported.
 * @modules ClaimableERC1155
 * @example
 * ```ts
 * import { ClaimableERC1155 } from "thirdweb/modules";
 *
 * const supported = ClaimableERC1155.isSetClaimConditionByTokenIdSupported(["0x..."]);
 * ```
 */
export declare function isSetClaimConditionByTokenIdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setClaimConditionByTokenId" function.
 * @param options - The options for the setClaimConditionByTokenId function.
 * @returns The encoded ABI parameters.
 * @modules ClaimableERC1155
 * @example
 * ```ts
 * import { ClaimableERC1155 } from "thirdweb/modules";
 * const result = ClaimableERC1155.encodeSetClaimConditionByTokenIdParams({
 *  tokenId: ...,
 *  claimCondition: ...,
 * });
 * ```
 */
export declare function encodeSetClaimConditionByTokenIdParams(options: SetClaimConditionByTokenIdParams): `0x${string}`;
/**
 * Encodes the "setClaimConditionByTokenId" function into a Hex string with its parameters.
 * @param options - The options for the setClaimConditionByTokenId function.
 * @returns The encoded hexadecimal string.
 * @modules ClaimableERC1155
 * @example
 * ```ts
 * import { ClaimableERC1155 } from "thirdweb/modules";
 * const result = ClaimableERC1155.encodeSetClaimConditionByTokenId({
 *  tokenId: ...,
 *  claimCondition: ...,
 * });
 * ```
 */
export declare function encodeSetClaimConditionByTokenId(options: SetClaimConditionByTokenIdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setClaimConditionByTokenId" function on the contract.
 * @param options - The options for the "setClaimConditionByTokenId" function.
 * @returns A prepared transaction object.
 * @modules ClaimableERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { ClaimableERC1155 } from "thirdweb/modules";
 *
 * const transaction = ClaimableERC1155.setClaimConditionByTokenId({
 *  contract,
 *  tokenId: ...,
 *  claimCondition: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setClaimConditionByTokenId(options: BaseTransactionOptions<SetClaimConditionByTokenIdParams | {
    asyncParams: () => Promise<SetClaimConditionByTokenIdParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setClaimConditionByTokenId.d.ts.map