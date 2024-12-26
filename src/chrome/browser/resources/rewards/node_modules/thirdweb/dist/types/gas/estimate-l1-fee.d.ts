import type { PreparedTransaction } from "../transaction/prepare-transaction.js";
type EstimateL1FeeOptions = {
    transaction: PreparedTransaction;
    gasPriceOracleAddress?: string;
};
/**
 * @internal
 */
export declare function estimateL1Fee(options: EstimateL1FeeOptions): Promise<bigint>;
export {};
//# sourceMappingURL=estimate-l1-fee.d.ts.map