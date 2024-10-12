import { type UseMutationResult } from "@tanstack/react-query";
import { type EstimateGasResult } from "../../../../transaction/actions/estimate-gas.js";
import type { PreparedTransaction } from "../../../../transaction/prepare-transaction.js";
/**
 * A hook to estimate the gas for a given transaction.
 * @returns A mutation object to estimate gas.
 * @example
 * ```jsx
 * import { useEstimateGas } from "thirdweb/react";
 * const { mutate: estimateGas, data: gasEstimate } = useEstimateGas();
 *
 * // later
 * const estimatedGas = await estimateGas(tx);
 * ```
 * @transaction
 */
export declare function useEstimateGas(): UseMutationResult<EstimateGasResult, Error, PreparedTransaction>;
//# sourceMappingURL=useEstimateGas.d.ts.map