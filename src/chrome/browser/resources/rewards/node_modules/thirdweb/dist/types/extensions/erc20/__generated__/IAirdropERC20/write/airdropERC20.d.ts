import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "airdropERC20" function.
 */
export type AirdropERC20Params = WithOverrides<{
    tokenAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenAddress";
    }>;
    tokenOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenOwner";
    }>;
    contents: AbiParameterToPrimitiveType<{
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
    }>;
}>;
export declare const FN_SELECTOR: "0x0670b2b3";
/**
 * Checks if the `airdropERC20` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `airdropERC20` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isAirdropERC20Supported } from "thirdweb/extensions/erc20";
 *
 * const supported = isAirdropERC20Supported(["0x..."]);
 * ```
 */
export declare function isAirdropERC20Supported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "airdropERC20" function.
 * @param options - The options for the airdropERC20 function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeAirdropERC20Params } from "thirdweb/extensions/erc20";
 * const result = encodeAirdropERC20Params({
 *  tokenAddress: ...,
 *  tokenOwner: ...,
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC20Params(options: AirdropERC20Params): `0x${string}`;
/**
 * Encodes the "airdropERC20" function into a Hex string with its parameters.
 * @param options - The options for the airdropERC20 function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeAirdropERC20 } from "thirdweb/extensions/erc20";
 * const result = encodeAirdropERC20({
 *  tokenAddress: ...,
 *  tokenOwner: ...,
 *  contents: ...,
 * });
 * ```
 */
export declare function encodeAirdropERC20(options: AirdropERC20Params): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "airdropERC20" function on the contract.
 * @param options - The options for the "airdropERC20" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { airdropERC20 } from "thirdweb/extensions/erc20";
 *
 * const transaction = airdropERC20({
 *  contract,
 *  tokenAddress: ...,
 *  tokenOwner: ...,
 *  contents: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function airdropERC20(options: BaseTransactionOptions<AirdropERC20Params | {
    asyncParams: () => Promise<AirdropERC20Params>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=airdropERC20.d.ts.map