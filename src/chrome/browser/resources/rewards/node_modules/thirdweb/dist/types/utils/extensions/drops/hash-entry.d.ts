import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
import type { OverrideEntry } from "./types.js";
export declare function hashEntry(options: {
    entry: OverrideEntry;
    chain: Chain;
    client: ThirdwebClient;
    tokenDecimals: number;
}): Promise<`0x${string}`>;
//# sourceMappingURL=hash-entry.d.ts.map