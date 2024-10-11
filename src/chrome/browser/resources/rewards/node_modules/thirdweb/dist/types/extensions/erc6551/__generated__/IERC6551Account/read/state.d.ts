import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xc19d93fb";
/**
 * Checks if the `state` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `state` method is supported.
 * @extension ERC6551
 * @example
 * ```ts
 * import { isStateSupported } from "thirdweb/extensions/erc6551";
 * const supported = isStateSupported(["0x..."]);
 * ```
 */
export declare function isStateSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the state function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC6551
 * @example
 * ```ts
 * import { decodeStateResult } from "thirdweb/extensions/erc6551";
 * const result = decodeStateResultResult("...");
 * ```
 */
export declare function decodeStateResult(result: Hex): bigint;
/**
 * Calls the "state" function on the contract.
 * @param options - The options for the state function.
 * @returns The parsed result of the function call.
 * @extension ERC6551
 * @example
 * ```ts
 * import { state } from "thirdweb/extensions/erc6551";
 *
 * const result = await state({
 *  contract,
 * });
 *
 * ```
 */
export declare function state(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=state.d.ts.map