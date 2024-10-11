import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xdd4e2ba5";
/**
 * Checks if the `COUNTING_MODE` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `COUNTING_MODE` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isCOUNTING_MODESupported } from "thirdweb/extensions/vote";
 * const supported = isCOUNTING_MODESupported(["0x..."]);
 * ```
 */
export declare function isCOUNTING_MODESupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the COUNTING_MODE function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeCOUNTING_MODEResult } from "thirdweb/extensions/vote";
 * const result = decodeCOUNTING_MODEResultResult("...");
 * ```
 */
export declare function decodeCOUNTING_MODEResult(result: Hex): string;
/**
 * Calls the "COUNTING_MODE" function on the contract.
 * @param options - The options for the COUNTING_MODE function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { COUNTING_MODE } from "thirdweb/extensions/vote";
 *
 * const result = await COUNTING_MODE({
 *  contract,
 * });
 *
 * ```
 */
export declare function COUNTING_MODE(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=COUNTING_MODE.d.ts.map