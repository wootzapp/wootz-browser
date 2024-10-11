import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "shares" function.
 */
export type SharesParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0xce7c2ac2";
/**
 * Checks if the `shares` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `shares` method is supported.
 * @extension SPLIT
 * @example
 * ```ts
 * import { isSharesSupported } from "thirdweb/extensions/split";
 * const supported = isSharesSupported(["0x..."]);
 * ```
 */
export declare function isSharesSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "shares" function.
 * @param options - The options for the shares function.
 * @returns The encoded ABI parameters.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeSharesParams } from "thirdweb/extensions/split";
 * const result = encodeSharesParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeSharesParams(options: SharesParams): `0x${string}`;
/**
 * Encodes the "shares" function into a Hex string with its parameters.
 * @param options - The options for the shares function.
 * @returns The encoded hexadecimal string.
 * @extension SPLIT
 * @example
 * ```ts
 * import { encodeShares } from "thirdweb/extensions/split";
 * const result = encodeShares({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeShares(options: SharesParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the shares function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension SPLIT
 * @example
 * ```ts
 * import { decodeSharesResult } from "thirdweb/extensions/split";
 * const result = decodeSharesResultResult("...");
 * ```
 */
export declare function decodeSharesResult(result: Hex): bigint;
/**
 * Calls the "shares" function on the contract.
 * @param options - The options for the shares function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { shares } from "thirdweb/extensions/split";
 *
 * const result = await shares({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function shares(options: BaseTransactionOptions<SharesParams>): Promise<bigint>;
//# sourceMappingURL=shares.d.ts.map