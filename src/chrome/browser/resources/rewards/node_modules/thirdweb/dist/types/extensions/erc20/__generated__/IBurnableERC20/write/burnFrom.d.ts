import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "burnFrom" function.
 */
export type BurnFromParams = WithOverrides<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amount";
    }>;
}>;
export declare const FN_SELECTOR: "0x79cc6790";
/**
 * Checks if the `burnFrom` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `burnFrom` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isBurnFromSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isBurnFromSupported(["0x..."]);
 * ```
 */
export declare function isBurnFromSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "burnFrom" function.
 * @param options - The options for the burnFrom function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeBurnFromParams } from "thirdweb/extensions/erc20";
 * const result = encodeBurnFromParams({
 *  account: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeBurnFromParams(options: BurnFromParams): `0x${string}`;
/**
 * Encodes the "burnFrom" function into a Hex string with its parameters.
 * @param options - The options for the burnFrom function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeBurnFrom } from "thirdweb/extensions/erc20";
 * const result = encodeBurnFrom({
 *  account: ...,
 *  amount: ...,
 * });
 * ```
 */
export declare function encodeBurnFrom(options: BurnFromParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "burnFrom" function on the contract.
 * @param options - The options for the "burnFrom" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { burnFrom } from "thirdweb/extensions/erc20";
 *
 * const transaction = burnFrom({
 *  contract,
 *  account: ...,
 *  amount: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function burnFrom(options: BaseTransactionOptions<BurnFromParams | {
    asyncParams: () => Promise<BurnFromParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=burnFrom.d.ts.map