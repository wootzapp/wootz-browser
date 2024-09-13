import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "maxMint" function.
 */
export type MaxMintParams = {
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
    }>;
};
export declare const FN_SELECTOR: "0xc63d75b6";
/**
 * Checks if the `maxMint` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `maxMint` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isMaxMintSupported } from "thirdweb/extensions/erc4626";
 * const supported = isMaxMintSupported(["0x..."]);
 * ```
 */
export declare function isMaxMintSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "maxMint" function.
 * @param options - The options for the maxMint function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxMintParams } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxMintParams({
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeMaxMintParams(options: MaxMintParams): `0x${string}`;
/**
 * Encodes the "maxMint" function into a Hex string with its parameters.
 * @param options - The options for the maxMint function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMaxMint } from "thirdweb/extensions/erc4626";
 * const result = encodeMaxMint({
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeMaxMint(options: MaxMintParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the maxMint function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4626
 * @example
 * ```ts
 * import { decodeMaxMintResult } from "thirdweb/extensions/erc4626";
 * const result = decodeMaxMintResultResult("...");
 * ```
 */
export declare function decodeMaxMintResult(result: Hex): bigint;
/**
 * Calls the "maxMint" function on the contract.
 * @param options - The options for the maxMint function.
 * @returns The parsed result of the function call.
 * @extension ERC4626
 * @example
 * ```ts
 * import { maxMint } from "thirdweb/extensions/erc4626";
 *
 * const result = await maxMint({
 *  contract,
 *  receiver: ...,
 * });
 *
 * ```
 */
export declare function maxMint(options: BaseTransactionOptions<MaxMintParams>): Promise<bigint>;
//# sourceMappingURL=maxMint.d.ts.map