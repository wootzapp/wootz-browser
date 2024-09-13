"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useSimulateTransaction = useSimulateTransaction;
const react_query_1 = require("@tanstack/react-query");
const simulate_js_1 = require("../../../../transaction/actions/simulate.js");
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
function useSimulateTransaction() {
    return (0, react_query_1.useMutation)({
        mutationFn: (options) => (0, simulate_js_1.simulateTransaction)(options),
    });
}
//# sourceMappingURL=useSimulateTransaction.js.map