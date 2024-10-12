import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xdeaaa7cc";
/**
 * Checks if the `BALLOT_TYPEHASH` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `BALLOT_TYPEHASH` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isBALLOT_TYPEHASHSupported } from "thirdweb/extensions/vote";
 * const supported = isBALLOT_TYPEHASHSupported(["0x..."]);
 * ```
 */
export declare function isBALLOT_TYPEHASHSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the BALLOT_TYPEHASH function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeBALLOT_TYPEHASHResult } from "thirdweb/extensions/vote";
 * const result = decodeBALLOT_TYPEHASHResultResult("...");
 * ```
 */
export declare function decodeBALLOT_TYPEHASHResult(result: Hex): `0x${string}`;
/**
 * Calls the "BALLOT_TYPEHASH" function on the contract.
 * @param options - The options for the BALLOT_TYPEHASH function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { BALLOT_TYPEHASH } from "thirdweb/extensions/vote";
 *
 * const result = await BALLOT_TYPEHASH({
 *  contract,
 * });
 *
 * ```
 */
export declare function BALLOT_TYPEHASH(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=BALLOT_TYPEHASH.d.ts.map