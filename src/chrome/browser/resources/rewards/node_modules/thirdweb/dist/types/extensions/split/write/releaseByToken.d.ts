import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { ReleaseParams } from "../__generated__/Split/write/release.js";
export type ReleaseByTokenParams = ReleaseParams & {
    tokenAddress: string;
};
/**
 * Similar to the `release` extension, however this one requires you to specify a tokenAddress to release
 * @param options
 * @returns
 * @example
 * ```ts
 * import { releaseByToken } from "thirdweb/extensions/split";
 *
 * const transaction = releaseByToken({
 *  contract,
 *  account: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * ...
 *
 * ```
 * @extension SPLIT
 */
export declare function releaseByToken(options: BaseTransactionOptions<ReleaseByTokenParams>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, {
    readonly name: "release";
    readonly type: "function";
    readonly stateMutability: "nonpayable";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "token";
    }, {
        readonly type: "address";
        readonly name: "account";
    }];
    readonly outputs: readonly [];
}, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=releaseByToken.d.ts.map