import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
import type { Hex } from "../../encoding/hex.js";
import type { OverrideEntry, ShardedMerkleTreeInfo } from "./types.js";
export declare function processOverrideList(options: {
    client: ThirdwebClient;
    chain: Chain;
    overrides: OverrideEntry[];
    tokenDecimals: number;
    shardNybbles?: number;
    hashEntry?: (options: {
        entry: OverrideEntry;
        chain: Chain;
        client: ThirdwebClient;
        tokenDecimals: number;
    }) => Promise<Hex>;
}): Promise<{
    shardedMerkleInfo: ShardedMerkleTreeInfo;
    uri: string;
}>;
//# sourceMappingURL=process-override-list.d.ts.map