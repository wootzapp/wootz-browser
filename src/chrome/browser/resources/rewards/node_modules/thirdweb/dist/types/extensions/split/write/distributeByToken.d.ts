import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * This extension is similar to the `distribute` extension,
 * however it require you to specify the token (address) that you want to distribute
 * @param options.tokenAddress - The contract address of the token you want to distribute
 * @returns A prepared transaction object.
 * @extension SPLIT
 * @example
 * ```ts
 * import { distributeByToken } from "thirdweb/extensions/split";
 *
 * const transaction = distributeByToken();
 *
 * // Send the transaction
 * ...
 *
 * ```
 */
export declare function distributeByToken(options: BaseTransactionOptions<{
    tokenAddress: string;
}>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, {
    readonly name: "distribute";
    readonly type: "function";
    readonly stateMutability: "nonpayable";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "token";
    }];
    readonly outputs: readonly [];
}, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=distributeByToken.d.ts.map