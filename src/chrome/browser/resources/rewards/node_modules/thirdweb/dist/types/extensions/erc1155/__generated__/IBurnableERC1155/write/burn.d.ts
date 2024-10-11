import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "burn" function.
 */
export type BurnParams = WithOverrides<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
    id: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "id";
    }>;
    value: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "value";
    }>;
}>;
export declare const FN_SELECTOR: "0xf5298aca";
/**
 * Checks if the `burn` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `burn` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isBurnSupported } from "thirdweb/extensions/erc1155";
 *
 * const supported = isBurnSupported(["0x..."]);
 * ```
 */
export declare function isBurnSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "burn" function.
 * @param options - The options for the burn function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBurnParams } from "thirdweb/extensions/erc1155";
 * const result = encodeBurnParams({
 *  account: ...,
 *  id: ...,
 *  value: ...,
 * });
 * ```
 */
export declare function encodeBurnParams(options: BurnParams): `0x${string}`;
/**
 * Encodes the "burn" function into a Hex string with its parameters.
 * @param options - The options for the burn function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBurn } from "thirdweb/extensions/erc1155";
 * const result = encodeBurn({
 *  account: ...,
 *  id: ...,
 *  value: ...,
 * });
 * ```
 */
export declare function encodeBurn(options: BurnParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "burn" function on the contract.
 * @param options - The options for the "burn" function.
 * @returns A prepared transaction object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { burn } from "thirdweb/extensions/erc1155";
 *
 * const transaction = burn({
 *  contract,
 *  account: ...,
 *  id: ...,
 *  value: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function burn(options: BaseTransactionOptions<BurnParams | {
    asyncParams: () => Promise<BurnParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=burn.d.ts.map