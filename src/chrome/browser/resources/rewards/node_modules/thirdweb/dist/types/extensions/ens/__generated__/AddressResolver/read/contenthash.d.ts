import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "contenthash" function.
 */
export type ContenthashParams = {
    name: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "name";
    }>;
};
export declare const FN_SELECTOR: "0xbc1c58d1";
/**
 * Checks if the `contenthash` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `contenthash` method is supported.
 * @extension ENS
 * @example
 * ```ts
 * import { isContenthashSupported } from "thirdweb/extensions/ens";
 * const supported = isContenthashSupported(["0x..."]);
 * ```
 */
export declare function isContenthashSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "contenthash" function.
 * @param options - The options for the contenthash function.
 * @returns The encoded ABI parameters.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeContenthashParams } from "thirdweb/extensions/ens";
 * const result = encodeContenthashParams({
 *  name: ...,
 * });
 * ```
 */
export declare function encodeContenthashParams(options: ContenthashParams): `0x${string}`;
/**
 * Encodes the "contenthash" function into a Hex string with its parameters.
 * @param options - The options for the contenthash function.
 * @returns The encoded hexadecimal string.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeContenthash } from "thirdweb/extensions/ens";
 * const result = encodeContenthash({
 *  name: ...,
 * });
 * ```
 */
export declare function encodeContenthash(options: ContenthashParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the contenthash function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ENS
 * @example
 * ```ts
 * import { decodeContenthashResult } from "thirdweb/extensions/ens";
 * const result = decodeContenthashResultResult("...");
 * ```
 */
export declare function decodeContenthashResult(result: Hex): `0x${string}`;
/**
 * Calls the "contenthash" function on the contract.
 * @param options - The options for the contenthash function.
 * @returns The parsed result of the function call.
 * @extension ENS
 * @example
 * ```ts
 * import { contenthash } from "thirdweb/extensions/ens";
 *
 * const result = await contenthash({
 *  contract,
 *  name: ...,
 * });
 *
 * ```
 */
export declare function contenthash(options: BaseTransactionOptions<ContenthashParams>): Promise<`0x${string}`>;
//# sourceMappingURL=contenthash.d.ts.map