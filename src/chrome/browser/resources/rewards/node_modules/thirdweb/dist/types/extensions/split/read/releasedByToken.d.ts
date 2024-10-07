import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { ReleasedParams } from "../__generated__/Split/read/released.js";
/**
 * Calls the "released" function on the contract.
 * Similar to the `released` extension, however this one requires you to specify a tokenAddress
 *
 * @param options - The options for the released function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { releasedByToken } from "thirdweb/extensions/split";
 *
 * const result = await releasedByToken({
 *  contract,
 *  account: "0x...",
 *  tokenAddress: "0x...",
 * });
 *
 * ```
 */
export declare function releasedByToken(options: BaseTransactionOptions<ReleasedParams & {
    tokenAddress: string;
}>): Promise<bigint>;
//# sourceMappingURL=releasedByToken.d.ts.map