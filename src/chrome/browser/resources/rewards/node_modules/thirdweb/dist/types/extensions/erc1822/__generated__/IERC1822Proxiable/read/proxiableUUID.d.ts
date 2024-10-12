import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x52d1902d";
/**
 * Checks if the `proxiableUUID` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `proxiableUUID` method is supported.
 * @extension ERC1822
 * @example
 * ```ts
 * import { isProxiableUUIDSupported } from "thirdweb/extensions/erc1822";
 * const supported = isProxiableUUIDSupported(["0x..."]);
 * ```
 */
export declare function isProxiableUUIDSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the proxiableUUID function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC1822
 * @example
 * ```ts
 * import { decodeProxiableUUIDResult } from "thirdweb/extensions/erc1822";
 * const result = decodeProxiableUUIDResultResult("...");
 * ```
 */
export declare function decodeProxiableUUIDResult(result: Hex): `0x${string}`;
/**
 * Calls the "proxiableUUID" function on the contract.
 * @param options - The options for the proxiableUUID function.
 * @returns The parsed result of the function call.
 * @extension ERC1822
 * @example
 * ```ts
 * import { proxiableUUID } from "thirdweb/extensions/erc1822";
 *
 * const result = await proxiableUUID({
 *  contract,
 * });
 *
 * ```
 */
export declare function proxiableUUID(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=proxiableUUID.d.ts.map