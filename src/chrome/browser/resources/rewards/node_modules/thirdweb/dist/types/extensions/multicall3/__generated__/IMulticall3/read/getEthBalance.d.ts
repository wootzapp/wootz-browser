import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getEthBalance" function.
 */
export type GetEthBalanceParams = {
    addr: AbiParameterToPrimitiveType<{
        type: "address";
        name: "addr";
    }>;
};
export declare const FN_SELECTOR: "0x4d2301cc";
/**
 * Checks if the `getEthBalance` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getEthBalance` method is supported.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { isGetEthBalanceSupported } from "thirdweb/extensions/multicall3";
 * const supported = isGetEthBalanceSupported(["0x..."]);
 * ```
 */
export declare function isGetEthBalanceSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getEthBalance" function.
 * @param options - The options for the getEthBalance function.
 * @returns The encoded ABI parameters.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeGetEthBalanceParams } from "thirdweb/extensions/multicall3";
 * const result = encodeGetEthBalanceParams({
 *  addr: ...,
 * });
 * ```
 */
export declare function encodeGetEthBalanceParams(options: GetEthBalanceParams): `0x${string}`;
/**
 * Encodes the "getEthBalance" function into a Hex string with its parameters.
 * @param options - The options for the getEthBalance function.
 * @returns The encoded hexadecimal string.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { encodeGetEthBalance } from "thirdweb/extensions/multicall3";
 * const result = encodeGetEthBalance({
 *  addr: ...,
 * });
 * ```
 */
export declare function encodeGetEthBalance(options: GetEthBalanceParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getEthBalance function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { decodeGetEthBalanceResult } from "thirdweb/extensions/multicall3";
 * const result = decodeGetEthBalanceResultResult("...");
 * ```
 */
export declare function decodeGetEthBalanceResult(result: Hex): bigint;
/**
 * Calls the "getEthBalance" function on the contract.
 * @param options - The options for the getEthBalance function.
 * @returns The parsed result of the function call.
 * @extension MULTICALL3
 * @example
 * ```ts
 * import { getEthBalance } from "thirdweb/extensions/multicall3";
 *
 * const result = await getEthBalance({
 *  contract,
 *  addr: ...,
 * });
 *
 * ```
 */
export declare function getEthBalance(options: BaseTransactionOptions<GetEthBalanceParams>): Promise<bigint>;
//# sourceMappingURL=getEthBalance.d.ts.map