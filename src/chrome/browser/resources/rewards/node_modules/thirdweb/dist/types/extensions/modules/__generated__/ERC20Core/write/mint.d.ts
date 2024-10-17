import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "mint" function.
 */
export type MintParams = WithOverrides<{
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
    }>;
    amount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "amount";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0x94d008ef";
/**
 * Checks if the `mint` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `mint` method is supported.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { ERC20Core } from "thirdweb/modules";
 *
 * const supported = ERC20Core.isMintSupported(["0x..."]);
 * ```
 */
export declare function isMintSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "mint" function.
 * @param options - The options for the mint function.
 * @returns The encoded ABI parameters.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { ERC20Core } from "thirdweb/modules";
 * const result = ERC20Core.encodeMintParams({
 *  to: ...,
 *  amount: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeMintParams(options: MintParams): `0x${string}`;
/**
 * Encodes the "mint" function into a Hex string with its parameters.
 * @param options - The options for the mint function.
 * @returns The encoded hexadecimal string.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { ERC20Core } from "thirdweb/modules";
 * const result = ERC20Core.encodeMint({
 *  to: ...,
 *  amount: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeMint(options: MintParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "mint" function on the contract.
 * @param options - The options for the "mint" function.
 * @returns A prepared transaction object.
 * @modules ERC20Core
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { ERC20Core } from "thirdweb/modules";
 *
 * const transaction = ERC20Core.mint({
 *  contract,
 *  to: ...,
 *  amount: ...,
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
export declare function mint(options: BaseTransactionOptions<MintParams | {
    asyncParams: () => Promise<MintParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=mint.d.ts.map