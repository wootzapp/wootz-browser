import type { ThirdwebClient } from "../../../client/client.js";
import type { ShardedMerkleTreeInfo, SnapshotEntryERC20 } from "./types.js";
export declare function processSnapshotERC20(options: {
    client: ThirdwebClient;
    snapshot: SnapshotEntryERC20[];
    tokenDecimals: number;
    shardNybbles?: number;
}): Promise<{
    shardedMerkleInfo: ShardedMerkleTreeInfo;
    uri: string;
}>;
//# sourceMappingURL=process-snapshot-erc20.d.ts.map