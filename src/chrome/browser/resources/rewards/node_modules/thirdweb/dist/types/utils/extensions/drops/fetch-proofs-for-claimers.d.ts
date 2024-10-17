import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
import type { ThirdwebContract } from "../../../contract/contract.js";
import type { Hex } from "../../encoding/hex.js";
import type { OverrideEntry, OverrideProof } from "./types.js";
export declare function fetchProofsForClaimer(options: {
    contract: ThirdwebContract;
    claimer: string;
    merkleTreeUri: string;
    tokenDecimals: number;
    hashEntry?: (options: {
        entry: OverrideEntry;
        chain: Chain;
        client: ThirdwebClient;
        tokenDecimals: number;
    }) => Promise<Hex>;
}): Promise<OverrideProof | null>;
//# sourceMappingURL=fetch-proofs-for-claimers.d.ts.map