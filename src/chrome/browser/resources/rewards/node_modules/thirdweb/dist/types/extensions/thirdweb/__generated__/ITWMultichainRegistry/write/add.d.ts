import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "add" function.
 */
export type AddParams = WithOverrides<{
    deployer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_deployer";
    }>;
    deployment: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_deployment";
    }>;
    chainId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_chainId";
    }>;
    metadataUri: AbiParameterToPrimitiveType<{
        type: "string";
        name: "metadataUri";
    }>;
}>;
export declare const FN_SELECTOR: "0x26c5b516";
/**
 * Checks if the `add` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `add` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isAddSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isAddSupported(["0x..."]);
 * ```
 */
export declare function isAddSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "add" function.
 * @param options - The options for the add function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeAddParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeAddParams({
 *  deployer: ...,
 *  deployment: ...,
 *  chainId: ...,
 *  metadataUri: ...,
 * });
 * ```
 */
export declare function encodeAddParams(options: AddParams): `0x${string}`;
/**
 * Encodes the "add" function into a Hex string with its parameters.
 * @param options - The options for the add function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeAdd } from "thirdweb/extensions/thirdweb";
 * const result = encodeAdd({
 *  deployer: ...,
 *  deployment: ...,
 *  chainId: ...,
 *  metadataUri: ...,
 * });
 * ```
 */
export declare function encodeAdd(options: AddParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "add" function on the contract.
 * @param options - The options for the "add" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { add } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = add({
 *  contract,
 *  deployer: ...,
 *  deployment: ...,
 *  chainId: ...,
 *  metadataUri: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function add(options: BaseTransactionOptions<AddParams | {
    asyncParams: () => Promise<AddParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=add.d.ts.map