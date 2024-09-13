import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getPastTotalSupply" function.
 */
export type GetPastTotalSupplyParams = {
    blockNumber: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "blockNumber";
    }>;
};
export declare const FN_SELECTOR: "0x8e539e8c";
/**
 * Checks if the `getPastTotalSupply` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getPastTotalSupply` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isGetPastTotalSupplySupported } from "thirdweb/extensions/erc20";
 * const supported = isGetPastTotalSupplySupported(["0x..."]);
 * ```
 */
export declare function isGetPastTotalSupplySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getPastTotalSupply" function.
 * @param options - The options for the getPastTotalSupply function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeGetPastTotalSupplyParams } from "thirdweb/extensions/erc20";
 * const result = encodeGetPastTotalSupplyParams({
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeGetPastTotalSupplyParams(options: GetPastTotalSupplyParams): `0x${string}`;
/**
 * Encodes the "getPastTotalSupply" function into a Hex string with its parameters.
 * @param options - The options for the getPastTotalSupply function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeGetPastTotalSupply } from "thirdweb/extensions/erc20";
 * const result = encodeGetPastTotalSupply({
 *  blockNumber: ...,
 * });
 * ```
 */
export declare function encodeGetPastTotalSupply(options: GetPastTotalSupplyParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getPastTotalSupply function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeGetPastTotalSupplyResult } from "thirdweb/extensions/erc20";
 * const result = decodeGetPastTotalSupplyResultResult("...");
 * ```
 */
export declare function decodeGetPastTotalSupplyResult(result: Hex): bigint;
/**
 * Calls the "getPastTotalSupply" function on the contract.
 * @param options - The options for the getPastTotalSupply function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { getPastTotalSupply } from "thirdweb/extensions/erc20";
 *
 * const result = await getPastTotalSupply({
 *  contract,
 *  blockNumber: ...,
 * });
 *
 * ```
 */
export declare function getPastTotalSupply(options: BaseTransactionOptions<GetPastTotalSupplyParams>): Promise<bigint>;
//# sourceMappingURL=getPastTotalSupply.d.ts.map