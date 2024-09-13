import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "totalSupply" function.
 */
export type TotalSupplyParams = {
    id: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "id";
    }>;
};
export declare const FN_SELECTOR: "0xbd85b039";
/**
 * Checks if the `totalSupply` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `totalSupply` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isTotalSupplySupported } from "thirdweb/extensions/erc1155";
 * const supported = isTotalSupplySupported(["0x..."]);
 * ```
 */
export declare function isTotalSupplySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "totalSupply" function.
 * @param options - The options for the totalSupply function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeTotalSupplyParams } from "thirdweb/extensions/erc1155";
 * const result = encodeTotalSupplyParams({
 *  id: ...,
 * });
 * ```
 */
export declare function encodeTotalSupplyParams(options: TotalSupplyParams): `0x${string}`;
/**
 * Encodes the "totalSupply" function into a Hex string with its parameters.
 * @param options - The options for the totalSupply function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeTotalSupply } from "thirdweb/extensions/erc1155";
 * const result = encodeTotalSupply({
 *  id: ...,
 * });
 * ```
 */
export declare function encodeTotalSupply(options: TotalSupplyParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the totalSupply function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeTotalSupplyResult } from "thirdweb/extensions/erc1155";
 * const result = decodeTotalSupplyResultResult("...");
 * ```
 */
export declare function decodeTotalSupplyResult(result: Hex): bigint;
/**
 * Calls the "totalSupply" function on the contract.
 * @param options - The options for the totalSupply function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { totalSupply } from "thirdweb/extensions/erc1155";
 *
 * const result = await totalSupply({
 *  contract,
 *  id: ...,
 * });
 *
 * ```
 */
export declare function totalSupply(options: BaseTransactionOptions<TotalSupplyParams>): Promise<bigint>;
//# sourceMappingURL=totalSupply.d.ts.map