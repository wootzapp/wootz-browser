import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "maxDeposit" function.
 */
export type MaxDepositParams = {
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
    }>;
};
export declare const FN_SELECTOR: "0x402d267d";
/**
 * Checks if the `maxDeposit` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `maxDeposit` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isMaxDepositSupported } from "thirdweb/extensions/erc4626";
 * const supported = isMaxDepositSupported(["0x..."]);
 * ```
 */
export declare function isMaxDepositSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "maxDeposit" function.
 * @param options - The options for the maxDeposit function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxDepositParams } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxDepositParams({
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeMaxDepositParams(options: MaxDepositParams): `0x${string}`;
/**
 * Encodes the "maxDeposit" function into a Hex string with its parameters.
 * @param options - The options for the maxDeposit function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxDeposit } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxDeposit({
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeMaxDeposit(options: MaxDepositParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the maxDeposit function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodeMaxDepositResult } from "thirdweb/extensions/erc4626";
 * const result = decodeMaxDepositResultResult("...");
 * ```
 */
export declare function decodeMaxDepositResult(result: Hex): bigint;
/**
 * Calls the "maxDeposit" function on the contract.
 * @param options - The options for the maxDeposit function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { maxDeposit } from "thirdweb/extensions/erc4626";
 *
 * const result = await maxDeposit({
 *  contract,
 *  receiver: ...,
 * });
 *
 * ```
 */
export declare function maxDeposit(options: BaseTransactionOptions<MaxDepositParams>): Promise<bigint>;
//# sourceMappingURL=maxDeposit.d.ts.map