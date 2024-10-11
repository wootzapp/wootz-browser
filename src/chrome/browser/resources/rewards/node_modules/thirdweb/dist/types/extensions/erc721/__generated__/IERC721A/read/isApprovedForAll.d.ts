import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isApprovedForAll" function.
 */
export type IsApprovedForAllParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
    operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "operator";
    }>;
};
export declare const FN_SELECTOR: "0xe985e9c5";
/**
 * Checks if the `isApprovedForAll` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isApprovedForAll` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isIsApprovedForAllSupported } from "thirdweb/extensions/erc721";
 * const supported = isIsApprovedForAllSupported(["0x..."]);
 * ```
 */
export declare function isIsApprovedForAllSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isApprovedForAll" function.
 * @param options - The options for the isApprovedForAll function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeIsApprovedForAllParams } from "thirdweb/extensions/erc721";
 * const result = encodeIsApprovedForAllParams({
 *  owner: ...,
 *  operator: ...,
 * });
 * ```
 */
export declare function encodeIsApprovedForAllParams(options: IsApprovedForAllParams): `0x${string}`;
/**
 * Encodes the "isApprovedForAll" function into a Hex string with its parameters.
 * @param options - The options for the isApprovedForAll function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeIsApprovedForAll } from "thirdweb/extensions/erc721";
 * const result = encodeIsApprovedForAll({
 *  owner: ...,
 *  operator: ...,
 * });
 * ```
 */
export declare function encodeIsApprovedForAll(options: IsApprovedForAllParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isApprovedForAll function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeIsApprovedForAllResult } from "thirdweb/extensions/erc721";
 * const result = decodeIsApprovedForAllResultResult("...");
 * ```
 */
export declare function decodeIsApprovedForAllResult(result: Hex): boolean;
/**
 * Calls the "isApprovedForAll" function on the contract.
 * @param options - The options for the isApprovedForAll function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { isApprovedForAll } from "thirdweb/extensions/erc721";
 *
 * const result = await isApprovedForAll({
 *  contract,
 *  owner: ...,
 *  operator: ...,
 * });
 *
 * ```
 */
export declare function isApprovedForAll(options: BaseTransactionOptions<IsApprovedForAllParams>): Promise<boolean>;
//# sourceMappingURL=isApprovedForAll.d.ts.map