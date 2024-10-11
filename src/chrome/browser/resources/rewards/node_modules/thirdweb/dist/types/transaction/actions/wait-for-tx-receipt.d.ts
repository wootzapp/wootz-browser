import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
import type { Prettify } from "../../utils/type-utils.js";
import type { SendTransactionResult, TransactionReceipt } from "../types.js";
export type WaitForReceiptOptions = Prettify<SendTransactionResult & {
    client: ThirdwebClient;
    chain: Chain;
    maxBlocksWaitTime?: number;
}>;
/**
 * Waits for the transaction receipt of a given transaction hash on a specific contract.
 * @param options - The options for waiting for the receipt.
 * By default, it's 100 blocks.
 * @returns A promise that resolves with the transaction receipt.
 * @transaction
 * @example
 * ```ts
 * import { waitForReceipt } from "thirdweb";
 * const receipt = await waitForReceipt({
 *   client,
 *   chain,
 *   transactionHash: "0x123...",
 * });
 * ```
 */
export declare function waitForReceipt(options: WaitForReceiptOptions): Promise<TransactionReceipt>;
//# sourceMappingURL=wait-for-tx-receipt.d.ts.map