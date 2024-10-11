import type { ThirdwebClient } from "../../client/client.js";
import type { NFTMetadata } from "./parseNft.js";
/**
 * @internal
 */
export type FetchTokenMetadataOptions = {
    client: ThirdwebClient;
    tokenId: bigint;
    tokenUri: string;
};
/**
 * Fetches the metadata for a token.
 *
 * @param options - The options for fetching the token metadata.
 * @returns The token metadata.
 * @internal
 */
export declare function fetchTokenMetadata(options: FetchTokenMetadataOptions): Promise<NFTMetadata>;
//# sourceMappingURL=fetchTokenMetadata.d.ts.map