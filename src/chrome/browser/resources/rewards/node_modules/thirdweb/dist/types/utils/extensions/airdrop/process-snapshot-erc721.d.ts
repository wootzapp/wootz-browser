import type { ThirdwebClient } from "../../../client/client.js";
import type { ShardedMerkleTreeInfo, SnapshotEntryERC721 } from "./types.js";
export declare function processSnapshotERC721(options: {
    client: ThirdwebClient;
    snapshot: SnapshotEntryERC721[];
    shardNybbles?: number;
}): Promise<{
    shardedMerkleInfo: ShardedMerkleTreeInfo;
    uri: string;
}>;
//# sourceMappingURL=process-snapshot-erc721.d.ts.map