import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "maxRedeem" function.
 */
export type MaxRedeemParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
};
export declare const FN_SELECTOR: "0xd905777e";
/**
 * Checks if the `maxRedeem` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `maxRedeem` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isMaxRedeemSupported } from "thirdweb/extensions/erc4626";
 * const supported = isMaxRedeemSupported(["0x..."]);
 * ```
 */
export declare function isMaxRedeemSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "maxRedeem" function.
 * @param options - The options for the maxRedeem function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxRedeemParams } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxRedeemParams({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeMaxRedeemParams(options: MaxRedeemParams): `0x${string}`;
/**
 * Encodes the "maxRedeem" function into a Hex string with its parameters.
 * @param options - The options for the maxRedeem function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxRedeem } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxRedeem({
 *  owner: ...,
 * });
 * ```
 */
export declare function encodeMaxRedeem(options: MaxRedeemParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the maxRedeem function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodeMaxRedeemResult } from "thirdweb/extensions/erc4626";
 * const result = decodeMaxRedeemResultResult("...");
 * ```
 */
export declare function decodeMaxRedeemResult(result: Hex): bigint;
/**
 * Calls the "maxRedeem" function on the contract.
 * @param options - The options for the maxRedeem function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { maxRedeem } from "thirdweb/extensions/erc4626";
 *
 * const result = await maxRedeem({
 *  contract,
 *  owner: ...,
 * });
 *
 * ```
 */
export declare function maxRedeem(options: BaseTransactionOptions<MaxRedeemParams>): Promise<bigint>;
//# sourceMappingURL=maxRedeem.d.ts.map