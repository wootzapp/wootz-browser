import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xf147db8a";
/**
 * Checks if the `getSupportedCallbackFunctions` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getSupportedCallbackFunctions` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isGetSupportedCallbackFunctionsSupported } from "thirdweb/extensions/modules";
 * const supported = isGetSupportedCallbackFunctionsSupported(["0x..."]);
 * ```
 */
export declare function isGetSupportedCallbackFunctionsSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getSupportedCallbackFunctions function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MODULES
 * @example
 * ```ts
 * import { decodeGetSupportedCallbackFunctionsResult } from "thirdweb/extensions/modules";
 * const result = decodeGetSupportedCallbackFunctionsResultResult("...");
 * ```
 */
export declare function decodeGetSupportedCallbackFunctionsResult(result: Hex): readonly {
    selector: `0x${string}`;
    mode: number;
}[];
/**
 * Calls the "getSupportedCallbackFunctions" function on the contract.
 * @param options - The options for the getSupportedCallbackFunctions function.
 * @returns The parsed result of the function call.
 * @extension MODULES
 * @example
 * ```ts
 * import { getSupportedCallbackFunctions } from "thirdweb/extensions/modules";
 *
 * const result = await getSupportedCallbackFunctions({
 *  contract,
 * });
 *
 * ```
 */
export declare function getSupportedCallbackFunctions(options: BaseTransactionOptions): Promise<readonly {
    selector: `0x${string}`;
    mode: number;
}[]>;
//# sourceMappingURL=getSupportedCallbackFunctions.d.ts.map