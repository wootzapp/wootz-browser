import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xb5775561";
/**
 * Checks if the `REMOVE_TYPEHASH` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `REMOVE_TYPEHASH` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isREMOVE_TYPEHASHSupported } from "thirdweb/extensions/farcaster";
 * const supported = isREMOVE_TYPEHASHSupported(["0x..."]);
 * ```
 */
export declare function isREMOVE_TYPEHASHSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the REMOVE_TYPEHASH function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeREMOVE_TYPEHASHResult } from "thirdweb/extensions/farcaster";
 * const result = decodeREMOVE_TYPEHASHResultResult("...");
 * ```
 */
export declare function decodeREMOVE_TYPEHASHResult(result: Hex): `0x${string}`;
/**
 * Calls the "REMOVE_TYPEHASH" function on the contract.
 * @param options - The options for the REMOVE_TYPEHASH function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { REMOVE_TYPEHASH } from "thirdweb/extensions/farcaster";
 *
 * const result = await REMOVE_TYPEHASH({
 *  contract,
 * });
 *
 * ```
 */
export declare function REMOVE_TYPEHASH(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=REMOVE_TYPEHASH.d.ts.map