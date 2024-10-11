import { type EstimateGasOptions } from "./estimate-gas.js";
export type EstimateGasCostResult = {
    /**
     * The estimated gas cost in ether.
     */
    ether: string;
    /**
     * The estimated gas cost in wei.
     */
    wei: bigint;
};
/**
 * Estimate the gas cost of a transaction in ether and wei.
 * @example
 * ```ts
 * import { estimateGasCost } from "thirdweb";
 *
 * const gasCost = await estimateGasCost({ transaction });
 * ```
 * @transaction
 */
export declare function estimateGasCost(options: EstimateGasOptions): Promise<EstimateGasCostResult>;
//# sourceMappingURL=estimate-gas-cost.d.ts.map