import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "deposit" function.
 */
export type DepositParams = WithOverrides<{
    assets: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "assets";
    }>;
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
    }>;
}>;
export declare const FN_SELECTOR: "0x6e553f65";
/**
 * Checks if the `deposit` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `deposit` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isDepositSupported } from "thirdweb/extensions/erc4626";
 *
 * const supported = isDepositSupported(["0x..."]);
 * ```
 */
export declare function isDepositSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "deposit" function.
 * @param options - The options for the deposit function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeDepositParams } from "thirdweb/extensions/erc4626";
 * const result = encodeDepositParams({
 *  assets: ...,
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeDepositParams(options: DepositParams): `0x${string}`;
/**
 * Encodes the "deposit" function into a Hex string with its parameters.
 * @param options - The options for the deposit function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeDeposit } from "thirdweb/extensions/erc4626";
 * const result = encodeDeposit({
 *  assets: ...,
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeDeposit(options: DepositParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "deposit" function on the contract.
 * @param options - The options for the "deposit" function.
 * @returns A prepared transaction object.
 * @extension ERC4626
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { deposit } from "thirdweb/extensions/erc4626";
 *
 * const transaction = deposit({
 *  contract,
 *  assets: ...,
 *  receiver: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function deposit(options: BaseTransactionOptions<DepositParams | {
    asyncParams: () => Promise<DepositParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=deposit.d.ts.map