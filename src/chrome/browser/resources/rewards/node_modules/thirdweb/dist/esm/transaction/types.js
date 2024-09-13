import { isObjectWithKeys } from "../utils/type-guards.js";
// type-guard
/**
 * Checks if the given value is of type TxOpts.
 * @param value - The value to check.
 * @returns True if the value is of type TxOpts, false otherwise.
 * @internal
 */
export function isBaseTransactionOptions(value) {
    return (isObjectWithKeys(value, ["__contract"]) &&
        isObjectWithKeys(value.__contract, ["address", "chain"]) &&
        typeof value.__contract.address === "string");
}
//# sourceMappingURL=types.js.map