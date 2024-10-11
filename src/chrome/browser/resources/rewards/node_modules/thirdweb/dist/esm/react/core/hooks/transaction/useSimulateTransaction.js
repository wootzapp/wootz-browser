import { useMutation } from "@tanstack/react-query";
import { simulateTransaction, } from "../../../../transaction/actions/simulate.js";
/**
 * A hook to simulate a transaction.
 * @returns A mutation object to simulate a transaction.
 * @example
 * ```jsx
 * import { useSimulateTransaction } from "thirdweb/react";
 * const { mutate: simulateTx } = useSimulateTransaction();
 *
 * // later
 * const result = await simulateTx(tx);
 * ```
 * @transaction
 */
export function useSimulateTransaction() {
    return useMutation({
        mutationFn: (options) => simulateTransaction(options),
    });
}
//# sourceMappingURL=useSimulateTransaction.js.map