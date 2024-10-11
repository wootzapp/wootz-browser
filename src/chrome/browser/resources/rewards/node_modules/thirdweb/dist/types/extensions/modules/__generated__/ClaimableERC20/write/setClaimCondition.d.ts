import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setClaimCondition" function.
 */
export type SetClaimConditionParams = WithOverrides<{
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
export declare const FN_SELECTOR: "0xac0c12f4";
/**
 * Checks if the `setClaimCondition` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setClaimCondition` method is supported.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { ClaimableERC20 } from "thirdweb/modules";
 *
 * const supported = ClaimableERC20.isSetClaimConditionSupported(["0x..."]);
 * ```
 */
export declare function isSetClaimConditionSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setClaimCondition" function.
 * @param options - The options for the setClaimCondition function.
 * @returns The encoded ABI parameters.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { ClaimableERC20 } from "thirdweb/modules";
 * const result = ClaimableERC20.encodeSetClaimConditionParams({
 *  claimCondition: ...,
 * });
 * ```
 */
export declare function encodeSetClaimConditionParams(options: SetClaimConditionParams): `0x${string}`;
/**
 * Encodes the "setClaimCondition" function into a Hex string with its parameters.
 * @param options - The options for the setClaimCondition function.
 * @returns The encoded hexadecimal string.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { ClaimableERC20 } from "thirdweb/modules";
 * const result = ClaimableERC20.encodeSetClaimCondition({
 *  claimCondition: ...,
 * });
 * ```
 */
export declare function encodeSetClaimCondition(options: SetClaimConditionParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setClaimCondition" function on the contract.
 * @param options - The options for the "setClaimCondition" function.
 * @returns A prepared transaction object.
 * @modules ClaimableERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { ClaimableERC20 } from "thirdweb/modules";
 *
 * const transaction = ClaimableERC20.setClaimCondition({
 *  contract,
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
export declare function setClaimCondition(options: BaseTransactionOptions<SetClaimConditionParams | {
    asyncParams: () => Promise<SetClaimConditionParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setClaimCondition.d.ts.map