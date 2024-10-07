import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { ReleasableParams } from "../__generated__/Split/read/releasable.js";
export type ReleasableByTokenParams = ReleasableParams & {
    tokenAddress: string;
};
/**
 * Calls the "releasable" function on the contract.
 * @param options - The options for the releasable function.
 * @returns The parsed result of the function call.
 * @extension SPLIT
 * @example
 * ```ts
 * import { releasableByToken } from "thirdweb/extensions/split";
 *
 * const result = await releasableByToken({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function releasableByToken(options: BaseTransactionOptions<ReleasableByTokenParams>): Promise<bigint>;
//# sourceMappingURL=releasableByToken.d.ts.map