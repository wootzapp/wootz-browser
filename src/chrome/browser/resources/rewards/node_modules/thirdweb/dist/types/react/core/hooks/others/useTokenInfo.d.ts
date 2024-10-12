import type { Chain } from "../../../../chains/types.js";
import type { ThirdwebClient } from "../../../../client/client.js";
type GetTokenInfoOptions = {
    client: ThirdwebClient;
    chain: Chain;
    tokenAddress?: string;
};
type GetTokenInfoResult = {
    name: string;
    symbol: string;
    decimals: number;
};
/**
 * @internal
 */
export declare function useTokenInfo(options: GetTokenInfoOptions): import("@tanstack/react-query").UseQueryResult<GetTokenInfoResult, Error>;
export {};
//# sourceMappingURL=useTokenInfo.d.ts.map