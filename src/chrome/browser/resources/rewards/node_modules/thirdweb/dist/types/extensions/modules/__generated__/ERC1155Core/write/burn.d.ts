import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "burn" function.
 */
export type BurnParams = WithOverrides<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
    }>;
    value: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "value";
    }>;
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
}>;
export declare const FN_SELECTOR: "0x8a94b05f";
/**
 * Checks if the `burn` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `burn` method is supported.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { ERC1155Core } from "thirdweb/modules";
 *
 * const supported = ERC1155Core.isBurnSupported(["0x..."]);
 * ```
 */
export declare function isBurnSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "burn" function.
 * @param options - The options for the burn function.
 * @returns The encoded ABI parameters.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { ERC1155Core } from "thirdweb/modules";
 * const result = ERC1155Core.encodeBurnParams({
 *  from: ...,
 *  tokenId: ...,
 *  value: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeBurnParams(options: BurnParams): `0x${string}`;
/**
 * Encodes the "burn" function into a Hex string with its parameters.
 * @param options - The options for the burn function.
 * @returns The encoded hexadecimal string.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { ERC1155Core } from "thirdweb/modules";
 * const result = ERC1155Core.encodeBurn({
 *  from: ...,
 *  tokenId: ...,
 *  value: ...,
 *  data: ...,
 * });
 * ```
 */
export declare function encodeBurn(options: BurnParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "burn" function on the contract.
 * @param options - The options for the "burn" function.
 * @returns A prepared transaction object.
 * @modules ERC1155Core
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { ERC1155Core } from "thirdweb/modules";
 *
 * const transaction = ERC1155Core.burn({
 *  contract,
 *  from: ...,
 *  tokenId: ...,
 *  value: ...,
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
export declare function burn(options: BaseTransactionOptions<BurnParams | {
    asyncParams: () => Promise<BurnParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=burn.d.ts.map