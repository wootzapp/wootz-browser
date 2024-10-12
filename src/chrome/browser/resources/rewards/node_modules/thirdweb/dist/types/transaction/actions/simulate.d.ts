import type { Abi, AbiFunction } from "abitype";
import type { Prettify } from "../../utils/type-utils.js";
import type { Account } from "../../wallets/interfaces/wallet.js";
import type { PreparedTransaction } from "../prepare-transaction.js";
export type SimulateOptions<abi extends Abi, abiFn extends AbiFunction> = Prettify<{
    transaction: PreparedTransaction<abi, abiFn>;
} & ({
    account: Account;
    from?: never;
} | {
    account?: never;
    from?: string;
})>;
/**
 * Simulates the execution of a transaction.
 * @param options - The options for simulating the transaction.
 * @returns A promise that resolves to the result of the simulation.
 * @transaction
 * @example
 * ```ts
 * import { simulateTransaction } from "thirdweb";
 * const result = await simulateTransaction({
 *  transaction,
 * });
 * ```
 */
export declare function simulateTransaction<const abi extends Abi, const abiFn extends AbiFunction>(options: SimulateOptions<abi, abiFn>): Promise<any>;
//# sourceMappingURL=simulate.d.ts.map