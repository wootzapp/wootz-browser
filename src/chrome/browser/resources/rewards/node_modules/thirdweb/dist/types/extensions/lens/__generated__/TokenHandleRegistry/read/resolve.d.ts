import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "resolve" function.
 */
export type ResolveParams = {
    handleId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "handleId";
    }>;
};
export declare const FN_SELECTOR: "0x4f896d4f";
/**
 * Checks if the `resolve` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `resolve` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isResolveSupported } from "thirdweb/extensions/lens";
 * const supported = isResolveSupported(["0x..."]);
 * ```
 */
export declare function isResolveSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "resolve" function.
 * @param options - The options for the resolve function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeResolveParams } from "thirdweb/extensions/lens";
 * const result = encodeResolveParams({
 *  handleId: ...,
 * });
 * ```
 */
export declare function encodeResolveParams(options: ResolveParams): `0x${string}`;
/**
 * Encodes the "resolve" function into a Hex string with its parameters.
 * @param options - The options for the resolve function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeResolve } from "thirdweb/extensions/lens";
 * const result = encodeResolve({
 *  handleId: ...,
 * });
 * ```
 */
export declare function encodeResolve(options: ResolveParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the resolve function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeResolveResult } from "thirdweb/extensions/lens";
 * const result = decodeResolveResultResult("...");
 * ```
 */
export declare function decodeResolveResult(result: Hex): bigint;
/**
 * Calls the "resolve" function on the contract.
 * @param options - The options for the resolve function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { resolve } from "thirdweb/extensions/lens";
 *
 * const result = await resolve({
 *  contract,
 *  handleId: ...,
 * });
 *
 * ```
 */
export declare function resolve(options: BaseTransactionOptions<ResolveParams>): Promise<bigint>;
//# sourceMappingURL=resolve.d.ts.map