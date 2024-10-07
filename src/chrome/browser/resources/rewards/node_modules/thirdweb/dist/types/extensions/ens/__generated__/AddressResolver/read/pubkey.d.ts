import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "pubkey" function.
 */
export type PubkeyParams = {
    name: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "name";
    }>;
};
export declare const FN_SELECTOR: "0xc8690233";
/**
 * Checks if the `pubkey` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `pubkey` method is supported.
 * @extension ENS
 * @example
 * ```ts
 * import { isPubkeySupported } from "thirdweb/extensions/ens";
 * const supported = isPubkeySupported(["0x..."]);
 * ```
 */
export declare function isPubkeySupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "pubkey" function.
 * @param options - The options for the pubkey function.
 * @returns The encoded ABI parameters.
 * @extension ENS
 * @example
 * ```ts
 * import { encodePubkeyParams } from "thirdweb/extensions/ens";
 * const result = encodePubkeyParams({
 *  name: ...,
 * });
 * ```
 */
export declare function encodePubkeyParams(options: PubkeyParams): `0x${string}`;
/**
 * Encodes the "pubkey" function into a Hex string with its parameters.
 * @param options - The options for the pubkey function.
 * @returns The encoded hexadecimal string.
 * @extension ENS
 * @example
 * ```ts
 * import { encodePubkey } from "thirdweb/extensions/ens";
 * const result = encodePubkey({
 *  name: ...,
 * });
 * ```
 */
export declare function encodePubkey(options: PubkeyParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the pubkey function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ENS
 * @example
 * ```ts
 * import { decodePubkeyResult } from "thirdweb/extensions/ens";
 * const result = decodePubkeyResultResult("...");
 * ```
 */
export declare function decodePubkeyResult(result: Hex): readonly [`0x${string}`, `0x${string}`];
/**
 * Calls the "pubkey" function on the contract.
 * @param options - The options for the pubkey function.
 * @returns The parsed result of the function call.
 * @extension ENS
 * @example
 * ```ts
 * import { pubkey } from "thirdweb/extensions/ens";
 *
 * const result = await pubkey({
 *  contract,
 *  name: ...,
 * });
 *
 * ```
 */
export declare function pubkey(options: BaseTransactionOptions<PubkeyParams>): Promise<readonly [`0x${string}`, `0x${string}`]>;
//# sourceMappingURL=pubkey.d.ts.map