import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getStakeInfo" function.
 */
export type GetStakeInfoParams = {
    staker: AbiParameterToPrimitiveType<{
        type: "address";
        name: "staker";
    }>;
};
export declare const FN_SELECTOR: "0xc3453153";
/**
 * Checks if the `getStakeInfo` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getStakeInfo` method is supported.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isGetStakeInfoSupported } from "thirdweb/extensions/erc1155";
 * const supported = isGetStakeInfoSupported(["0x..."]);
 * ```
 */
export declare function isGetStakeInfoSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getStakeInfo" function.
 * @param options - The options for the getStakeInfo function.
 * @returns The encoded ABI parameters.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeGetStakeInfoParams } from "thirdweb/extensions/erc1155";
 * const result = encodeGetStakeInfoParams({
 *  staker: ...,
 * });
 * ```
 */
export declare function encodeGetStakeInfoParams(options: GetStakeInfoParams): `0x${string}`;
/**
 * Encodes the "getStakeInfo" function into a Hex string with its parameters.
 * @param options - The options for the getStakeInfo function.
 * @returns The encoded hexadecimal string.
 * @extension ERC1155
 * @example
 * ```ts
 * import { encodeGetStakeInfo } from "thirdweb/extensions/erc1155";
 * const result = encodeGetStakeInfo({
 *  staker: ...,
 * });
 * ```
 */
export declare function encodeGetStakeInfo(options: GetStakeInfoParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getStakeInfo function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1155
 * @example
 * ```ts
 * import { decodeGetStakeInfoResult } from "thirdweb/extensions/erc1155";
 * const result = decodeGetStakeInfoResultResult("...");
 * ```
 */
export declare function decodeGetStakeInfoResult(result: Hex): readonly [readonly bigint[], readonly bigint[], bigint];
/**
 * Calls the "getStakeInfo" function on the contract.
 * @param options - The options for the getStakeInfo function.
 * @returns The parsed result of the function call.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getStakeInfo } from "thirdweb/extensions/erc1155";
 *
 * const result = await getStakeInfo({
 *  contract,
 *  staker: ...,
 * });
 *
 * ```
 */
export declare function getStakeInfo(options: BaseTransactionOptions<GetStakeInfoParams>): Promise<readonly [readonly bigint[], readonly bigint[], bigint]>;
//# sourceMappingURL=getStakeInfo.d.ts.map