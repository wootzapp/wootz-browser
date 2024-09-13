import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x6a5306a3";
/**
 * Checks if the `REGISTER_TYPEHASH` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `REGISTER_TYPEHASH` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isREGISTER_TYPEHASHSupported } from "thirdweb/extensions/farcaster";
 * const supported = isREGISTER_TYPEHASHSupported(["0x..."]);
 * ```
 */
export declare function isREGISTER_TYPEHASHSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the REGISTER_TYPEHASH function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeREGISTER_TYPEHASHResult } from "thirdweb/extensions/farcaster";
 * const result = decodeREGISTER_TYPEHASHResultResult("...");
 * ```
 */
export declare function decodeREGISTER_TYPEHASHResult(result: Hex): `0x${string}`;
/**
 * Calls the "REGISTER_TYPEHASH" function on the contract.
 * @param options - The options for the REGISTER_TYPEHASH function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { REGISTER_TYPEHASH } from "thirdweb/extensions/farcaster";
 *
 * const result = await REGISTER_TYPEHASH({
 *  contract,
 * });
 *
 * ```
 */
export declare function REGISTER_TYPEHASH(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=REGISTER_TYPEHASH.d.ts.map