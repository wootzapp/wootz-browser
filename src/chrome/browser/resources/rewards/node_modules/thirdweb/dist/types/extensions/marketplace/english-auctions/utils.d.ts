import type { BaseTransactionOptions } from "../../../transaction/types.js";
import type { getAuction } from "../__generated__/IEnglishAuctions/read/getAuction.js";
import type { EnglishAuction } from "./types.js";
/**
 * @internal
 */
export declare function mapEnglishAuction(options: BaseTransactionOptions<{
    latestBlock: {
        timestamp: bigint;
    };
    rawAuction: Awaited<ReturnType<typeof getAuction>>;
}>): Promise<EnglishAuction>;
//# sourceMappingURL=utils.d.ts.map