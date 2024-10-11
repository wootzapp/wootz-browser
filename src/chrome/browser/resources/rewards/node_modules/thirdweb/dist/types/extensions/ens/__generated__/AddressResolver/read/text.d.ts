import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "text" function.
 */
export type TextParams = {
    name: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "name";
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "string";
        name: "key";
    }>;
};
export declare const FN_SELECTOR: "0x59d1d43c";
/**
 * Checks if the `text` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `text` method is supported.
 * @extension ENS
 * @example
 * ```ts
 * import { isTextSupported } from "thirdweb/extensions/ens";
 * const supported = isTextSupported(["0x..."]);
 * ```
 */
export declare function isTextSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "text" function.
 * @param options - The options for the text function.
 * @returns The encoded ABI parameters.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeTextParams } from "thirdweb/extensions/ens";
 * const result = encodeTextParams({
 *  name: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeTextParams(options: TextParams): `0x${string}`;
/**
 * Encodes the "text" function into a Hex string with its parameters.
 * @param options - The options for the text function.
 * @returns The encoded hexadecimal string.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeText } from "thirdweb/extensions/ens";
 * const result = encodeText({
 *  name: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeText(options: TextParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the text function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ENS
 * @example
 * ```ts
 * import { decodeTextResult } from "thirdweb/extensions/ens";
 * const result = decodeTextResultResult("...");
 * ```
 */
export declare function decodeTextResult(result: Hex): string;
/**
 * Calls the "text" function on the contract.
 * @param options - The options for the text function.
 * @returns The parsed result of the function call.
 * @extension ENS
 * @example
 * ```ts
 * import { text } from "thirdweb/extensions/ens";
 *
 * const result = await text({
 *  contract,
 *  name: ...,
 *  key: ...,
 * });
 *
 * ```
 */
export declare function text(options: BaseTransactionOptions<TextParams>): Promise<string>;
//# sourceMappingURL=text.d.ts.map