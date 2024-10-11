import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x094ec830";
/**
 * Checks if the `appURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `appURI` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isAppURISupported } from "thirdweb/extensions/thirdweb";
 * const supported = isAppURISupported(["0x..."]);
 * ```
 */
export declare function isAppURISupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the appURI function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeAppURIResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeAppURIResultResult("...");
 * ```
 */
export declare function decodeAppURIResult(result: Hex): string;
/**
 * Calls the "appURI" function on the contract.
 * @param options - The options for the appURI function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { appURI } from "thirdweb/extensions/thirdweb";
 *
 * const result = await appURI({
 *  contract,
 * });
 *
 * ```
 */
export declare function appURI(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=appURI.d.ts.map