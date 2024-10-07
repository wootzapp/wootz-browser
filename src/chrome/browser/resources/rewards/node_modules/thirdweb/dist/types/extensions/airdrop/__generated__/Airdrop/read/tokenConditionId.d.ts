import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "tokenConditionId" function.
 */
export type TokenConditionIdParams = {
    tokenAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenAddress";
    }>;
};
export declare const FN_SELECTOR: "0x3dc28d49";
/**
 * Checks if the `tokenConditionId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tokenConditionId` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isTokenConditionIdSupported } from "thirdweb/extensions/airdrop";
 * const supported = isTokenConditionIdSupported(["0x..."]);
 * ```
 */
export declare function isTokenConditionIdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tokenConditionId" function.
 * @param options - The options for the tokenConditionId function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeTokenConditionIdParams } from "thirdweb/extensions/airdrop";
 * const result = encodeTokenConditionIdParams({
 *  tokenAddress: ...,
 * });
 * ```
 */
export declare function encodeTokenConditionIdParams(options: TokenConditionIdParams): `0x${string}`;
/**
 * Encodes the "tokenConditionId" function into a Hex string with its parameters.
 * @param options - The options for the tokenConditionId function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeTokenConditionId } from "thirdweb/extensions/airdrop";
 * const result = encodeTokenConditionId({
 *  tokenAddress: ...,
 * });
 * ```
 */
export declare function encodeTokenConditionId(options: TokenConditionIdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the tokenConditionId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { decodeTokenConditionIdResult } from "thirdweb/extensions/airdrop";
 * const result = decodeTokenConditionIdResultResult("...");
 * ```
 */
export declare function decodeTokenConditionIdResult(result: Hex): bigint;
/**
 * Calls the "tokenConditionId" function on the contract.
 * @param options - The options for the tokenConditionId function.
 * @returns The parsed result of the function call.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { tokenConditionId } from "thirdweb/extensions/airdrop";
 *
 * const result = await tokenConditionId({
 *  contract,
 *  tokenAddress: ...,
 * });
 *
 * ```
 */
export declare function tokenConditionId(options: BaseTransactionOptions<TokenConditionIdParams>): Promise<bigint>;
//# sourceMappingURL=tokenConditionId.d.ts.map