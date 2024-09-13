import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "claim" function.
 */
export type ClaimParams = WithOverrides<{
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
    }>;
    quantity: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "quantity";
    }>;
    currency: AbiParameterToPrimitiveType<{
        type: "address";
        name: "currency";
    }>;
    pricePerToken: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "pricePerToken";
    }>;
    allowlistProof: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "allowlistProof";
        components: [
            {
                type: "bytes32[]";
                name: "proof";
            },
            {
                type: "uint256";
                name: "quantityLimitPerWallet";
            },
            {
                type: "uint256";
                name: "pricePerToken";
            },
            {
                type: "address";
                name: "currency";
            }
        ];
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0x84bb1e42";
/**
 * Checks if the `claim` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claim` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isClaimSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isClaimSupported(["0x..."]);
 * ```
 */
export declare function isClaimSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "claim" function.
 * @param options - The options for the claim function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeClaimParams } from "thirdweb/extensions/erc20";
 * const result = encodeClaimParams({
 *  receiver: ...,
 *  quantity: ...,
 *  currency: ...,
 *  pricePerToken: ...,
 *  allowlistProof: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeClaimParams(options: ClaimParams): `0x${string}`;
/**
 * Encodes the "claim" function into a Hex string with its parameters.
 * @param options - The options for the claim function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeClaim } from "thirdweb/extensions/erc20";
 * const result = encodeClaim({
 *  receiver: ...,
 *  quantity: ...,
 *  currency: ...,
 *  pricePerToken: ...,
 *  allowlistProof: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeClaim(options: ClaimParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "claim" function on the contract.
 * @param options - The options for the "claim" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { claim } from "thirdweb/extensions/erc20";
 *
 * const transaction = claim({
 *  contract,
 *  receiver: ...,
 *  quantity: ...,
 *  currency: ...,
 *  pricePerToken: ...,
 *  allowlistProof: ...,
 *  data: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function claim(options: BaseTransactionOptions<ClaimParams | {
    asyncParams: () => Promise<ClaimParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=claim.d.ts.map