import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getNonce" function.
 */
export type GetNonceParams = {
    sender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "sender";
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "uint192";
        name: "key";
    }>;
};
export declare const FN_SELECTOR: "0x35567e1a";
/**
 * Checks if the `getNonce` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getNonce` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isGetNonceSupported } from "thirdweb/extensions/erc4337";
 * const supported = isGetNonceSupported(["0x..."]);
 * ```
 */
export declare function isGetNonceSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getNonce" function.
 * @param options - The options for the getNonce function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetNonceParams } from "thirdweb/extensions/erc4337";
 * const result = encodeGetNonceParams({
 *  sender: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeGetNonceParams(options: GetNonceParams): `0x${string}`;
/**
 * Encodes the "getNonce" function into a Hex string with its parameters.
 * @param options - The options for the getNonce function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetNonce } from "thirdweb/extensions/erc4337";
 * const result = encodeGetNonce({
 *  sender: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeGetNonce(options: GetNonceParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getNonce function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4337
 * @example
 * ```ts
 * import { decodeGetNonceResult } from "thirdweb/extensions/erc4337";
 * const result = decodeGetNonceResultResult("...");
 * ```
 */
export declare function decodeGetNonceResult(result: Hex): bigint;
/**
 * Calls the "getNonce" function on the contract.
 * @param options - The options for the getNonce function.
 * @returns The parsed result of the function call.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getNonce } from "thirdweb/extensions/erc4337";
 *
 * const result = await getNonce({
 *  contract,
 *  sender: ...,
 *  key: ...,
 * });
 *
 * ```
 */
export declare function getNonce(options: BaseTransactionOptions<GetNonceParams>): Promise<bigint>;
//# sourceMappingURL=getNonce.d.ts.map