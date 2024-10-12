import type { Chain } from "../chains/types.js";
import type { ThirdwebClient } from "../client/client.js";
export type WatchBlockNumberOptions = {
    client: ThirdwebClient;
    chain: Chain;
    onNewBlockNumber: (blockNumber: bigint) => void;
    onError?: (error: Error) => void;
    overPollRatio?: number;
    latestBlockNumber?: bigint;
};
/**
 * Watches the block number for a specific chain.
 * @param opts - The options for watching the block number.
 * @returns The unwatch function.
 * @example
 * ```ts
 * import { watchBlockNumber } from "thirdweb";
 * const unwatch = watchBlockNumber({
 *  client,
 *  chainId,
 *  onNewBlockNumber: (blockNumber) => {
 *    // do something with the block number
 *  },
 *  onError: (err) => {
 *    // do something if getting the block number fails
 *  },
 * });
 *
 * // later stop watching
 * unwatch();
 * ```
 * @rpc
 */
export declare function watchBlockNumber(opts: WatchBlockNumberOptions): () => void;
//# sourceMappingURL=watchBlockNumber.d.ts.map