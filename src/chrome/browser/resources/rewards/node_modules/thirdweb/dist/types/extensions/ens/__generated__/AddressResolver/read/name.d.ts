import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "name" function.
 */
export type NameParams = {
    name: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "name";
    }>;
};
export declare const FN_SELECTOR: "0x691f3431";
/**
 * Checks if the `name` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `name` method is supported.
 * @extension ENS
 * @example
 * ```ts
 * import { isNameSupported } from "thirdweb/extensions/ens";
 * const supported = isNameSupported(["0x..."]);
 * ```
 */
export declare function isNameSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "name" function.
 * @param options - The options for the name function.
 * @returns The encoded ABI parameters.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeNameParams } from "thirdweb/extensions/ens";
 * const result = encodeNameParams({
 *  name: ...,
 * });
 * ```
 */
export declare function encodeNameParams(options: NameParams): `0x${string}`;
/**
 * Encodes the "name" function into a Hex string with its parameters.
 * @param options - The options for the name function.
 * @returns The encoded hexadecimal string.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeName } from "thirdweb/extensions/ens";
 * const result = encodeName({
 *  name: ...,
 * });
 * ```
 */
export declare function encodeName(options: NameParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the name function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ENS
 * @example
 * ```ts
 * import { decodeNameResult } from "thirdweb/extensions/ens";
 * const result = decodeNameResultResult("...");
 * ```
 */
export declare function decodeNameResult(result: Hex): string;
/**
 * Calls the "name" function on the contract.
 * @param options - The options for the name function.
 * @returns The parsed result of the function call.
 * @extension ENS
 * @example
 * ```ts
 * import { name } from "thirdweb/extensions/ens";
 *
 * const result = await name({
 *  contract,
 *  name: ...,
 * });
 *
 * ```
 */
export declare function name(options: BaseTransactionOptions<NameParams>): Promise<string>;
//# sourceMappingURL=name.d.ts.map