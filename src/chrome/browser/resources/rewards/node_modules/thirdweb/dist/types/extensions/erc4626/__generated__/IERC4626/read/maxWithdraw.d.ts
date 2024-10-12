import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "maxWithdraw" function.
 */
export type MaxWithdrawParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
};
export declare const FN_SELECTOR: "0xce96cb77";
/**
 * Checks if the `maxWithdraw` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `maxWithdraw` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isMaxWithdrawSupported } from "thirdweb/extensions/erc4626";
 * const supported = isMaxWithdrawSupported(["0x..."]);
 * ```
 */
export declare function isMaxWithdrawSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "maxWithdraw" function.
 * @param options - The options for the maxWithdraw function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxWithdrawParams } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxWithdrawParams({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeMaxWithdrawParams(options: MaxWithdrawParams): `0x${string}`;
/**
 * Encodes the "maxWithdraw" function into a Hex string with its parameters.
 * @param options - The options for the maxWithdraw function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxWithdraw } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxWithdraw({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeMaxWithdraw(options: MaxWithdrawParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the maxWithdraw function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodeMaxWithdrawResult } from "thirdweb/extensions/erc4626";
 * const result = decodeMaxWithdrawResultResult("...");
 * ```
 */
export declare function decodeMaxWithdrawResult(result: Hex): bigint;
/**
 * Calls the "maxWithdraw" function on the contract.
 * @param options - The options for the maxWithdraw function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { maxWithdraw } from "thirdweb/extensions/erc4626";
 *
 * const result = await maxWithdraw({
 *  contract,
 *  owner: ...,
 * });
 *
 * ```
 */
export declare function maxWithdraw(options: BaseTransactionOptions<MaxWithdrawParams>): Promise<bigint>;
//# sourceMappingURL=maxWithdraw.d.ts.map