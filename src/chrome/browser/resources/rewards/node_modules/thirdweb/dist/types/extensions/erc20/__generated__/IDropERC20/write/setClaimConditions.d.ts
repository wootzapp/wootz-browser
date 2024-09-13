import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setClaimConditions" function.
 */
export type SetClaimConditionsParams = WithOverrides<{
    phases: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "phases";
        components: [
            {
                type: "uint256";
                name: "startTimestamp";
            },
            {
                type: "uint256";
                name: "maxClaimableSupply";
            },
            {
                type: "uint256";
                name: "supplyClaimed";
            },
            {
                type: "uint256";
                name: "quantityLimitPerWallet";
            },
            {
                type: "bytes32";
                name: "merkleRoot";
            },
            {
                type: "uint256";
                name: "pricePerToken";
            },
            {
                type: "address";
                name: "currency";
            },
            {
                type: "string";
                name: "metadata";
            }
        ];
    }>;
    resetClaimEligibility: AbiParameterToPrimitiveType<{
        type: "bool";
        name: "resetClaimEligibility";
    }>;
}>;
export declare const FN_SELECTOR: "0x74bc7db7";
/**
 * Checks if the `setClaimConditions` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setClaimConditions` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isSetClaimConditionsSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isSetClaimConditionsSupported(["0x..."]);
 * ```
 */
export declare function isSetClaimConditionsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setClaimConditions" function.
 * @param options - The options for the setClaimConditions function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeSetClaimConditionsParams } from "thirdweb/extensions/erc20";
 * const result = encodeSetClaimConditionsParams({
 *  phases: ...,
 *  resetClaimEligibility: ...,
 * });
 * ```
 */
export declare function encodeSetClaimConditionsParams(options: SetClaimConditionsParams): `0x${string}`;
/**
 * Encodes the "setClaimConditions" function into a Hex string with its parameters.
 * @param options - The options for the setClaimConditions function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeSetClaimConditions } from "thirdweb/extensions/erc20";
 * const result = encodeSetClaimConditions({
 *  phases: ...,
 *  resetClaimEligibility: ...,
 * });
 * ```
 */
export declare function encodeSetClaimConditions(options: SetClaimConditionsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setClaimConditions" function on the contract.
 * @param options - The options for the "setClaimConditions" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setClaimConditions } from "thirdweb/extensions/erc20";
 *
 * const transaction = setClaimConditions({
 *  contract,
 *  phases: ...,
 *  resetClaimEligibility: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setClaimConditions(options: BaseTransactionOptions<SetClaimConditionsParams | {
    asyncParams: () => Promise<SetClaimConditionsParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setClaimConditions.d.ts.map