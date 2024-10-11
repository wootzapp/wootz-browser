import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "balanceOfBatch" function.
 */
export type BalanceOfBatchParams = {
    owners: AbiParameterToPrimitiveType<{
        type: "address[]";
        name: "_owners";
    }>;
    tokenIds: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "tokenIds";
    }>;
};
export declare const FN_SELECTOR: "0x4e1273f4";
/**
 * Checks if the `balanceOfBatch` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `balanceOfBatch` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isBalanceOfBatchSupported } from "thirdweb/extensions/erc1155";
 * const supported = isBalanceOfBatchSupported(["0x..."]);
 * ```
 */
export declare function isBalanceOfBatchSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "balanceOfBatch" function.
 * @param options - The options for the balanceOfBatch function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBalanceOfBatchParams } from "thirdweb/extensions/erc1155";
 * const result = encodeBalanceOfBatchParams({
 *  owners: ...,
 *  tokenIds: ...,
 * });
 * ```
 */
export declare function encodeBalanceOfBatchParams(options: BalanceOfBatchParams): `0x${string}`;
/**
 * Encodes the "balanceOfBatch" function into a Hex string with its parameters.
 * @param options - The options for the balanceOfBatch function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeBalanceOfBatch } from "thirdweb/extensions/erc1155";
 * const result = encodeBalanceOfBatch({
 *  owners: ...,
 *  tokenIds: ...,
 * });
 * ```
 */
export declare function encodeBalanceOfBatch(options: BalanceOfBatchParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the balanceOfBatch function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeBalanceOfBatchResult } from "thirdweb/extensions/erc1155";
 * const result = decodeBalanceOfBatchResultResult("...");
 * ```
 */
export declare function decodeBalanceOfBatchResult(result: Hex): readonly bigint[];
/**
 * Calls the "balanceOfBatch" function on the contract.
 * @param options - The options for the balanceOfBatch function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { balanceOfBatch } from "thirdweb/extensions/erc1155";
 *
 * const result = await balanceOfBatch({
 *  contract,
 *  owners: ...,
 *  tokenIds: ...,
 * });
 *
 * ```
 */
export declare function balanceOfBatch(options: BaseTransactionOptions<BalanceOfBatchParams>): Promise<readonly bigint[]>;
//# sourceMappingURL=balanceOfBatch.d.ts.map