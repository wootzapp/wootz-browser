import type { ThirdwebClient } from "../../../client/client.js";
import type { ShardedMerkleTreeInfo, SnapshotEntryERC1155 } from "./types.js";
export declare function processSnapshotERC1155(options: {
    client: ThirdwebClient;
    snapshot: SnapshotEntryERC1155[];
    shardNybbles?: number;
}): Promise<{
    shardedMerkleInfo: ShardedMerkleTreeInfo;
    uri: string;
}>;
//# sourceMappingURL=process-snapshot-erc1155.d.ts.map