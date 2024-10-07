import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "createPool" function.
 */
export type CreatePoolParams = WithOverrides<{
    tokenA: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenA";
    }>;
    tokenB: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenB";
    }>;
    fee: AbiParameterToPrimitiveType<{
        type: "uint24";
        name: "fee";
    }>;
}>;
export declare const FN_SELECTOR: "0xa1671295";
/**
 * Checks if the `createPool` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createPool` method is supported.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { isCreatePoolSupported } from "thirdweb/extensions/uniswap";
 *
 * const supported = isCreatePoolSupported(["0x..."]);
 * ```
 */
export declare function isCreatePoolSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "createPool" function.
 * @param options - The options for the createPool function.
 * @returns The encoded ABI parameters.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeCreatePoolParams } from "thirdweb/extensions/uniswap";
 * const result = encodeCreatePoolParams({
 *  tokenA: ...,
 *  tokenB: ...,
 *  fee: ...,
 * });
 * ```
 */
export declare function encodeCreatePoolParams(options: CreatePoolParams): `0x${string}`;
/**
 * Encodes the "createPool" function into a Hex string with its parameters.
 * @param options - The options for the createPool function.
 * @returns The encoded hexadecimal string.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { encodeCreatePool } from "thirdweb/extensions/uniswap";
 * const result = encodeCreatePool({
 *  tokenA: ...,
 *  tokenB: ...,
 *  fee: ...,
 * });
 * ```
 */
export declare function encodeCreatePool(options: CreatePoolParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "createPool" function on the contract.
 * @param options - The options for the "createPool" function.
 * @returns A prepared transaction object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { createPool } from "thirdweb/extensions/uniswap";
 *
 * const transaction = createPool({
 *  contract,
 *  tokenA: ...,
 *  tokenB: ...,
 *  fee: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function createPool(options: BaseTransactionOptions<CreatePoolParams | {
    asyncParams: () => Promise<CreatePoolParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=createPool.d.ts.map