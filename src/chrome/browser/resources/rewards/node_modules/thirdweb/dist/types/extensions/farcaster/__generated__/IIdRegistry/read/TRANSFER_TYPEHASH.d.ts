import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x00bf26f4";
/**
 * Checks if the `TRANSFER_TYPEHASH` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `TRANSFER_TYPEHASH` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isTRANSFER_TYPEHASHSupported } from "thirdweb/extensions/farcaster";
 * const supported = isTRANSFER_TYPEHASHSupported(["0x..."]);
 * ```
 */
export declare function isTRANSFER_TYPEHASHSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the TRANSFER_TYPEHASH function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeTRANSFER_TYPEHASHResult } from "thirdweb/extensions/farcaster";
 * const result = decodeTRANSFER_TYPEHASHResultResult("...");
 * ```
 */
export declare function decodeTRANSFER_TYPEHASHResult(result: Hex): `0x${string}`;
/**
 * Calls the "TRANSFER_TYPEHASH" function on the contract.
 * @param options - The options for the TRANSFER_TYPEHASH function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { TRANSFER_TYPEHASH } from "thirdweb/extensions/farcaster";
 *
 * const result = await TRANSFER_TYPEHASH({
 *  contract,
 * });
 *
 * ```
 */
export declare function TRANSFER_TYPEHASH(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=TRANSFER_TYPEHASH.d.ts.map