import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
import type { Prettify } from "../type-utils.js";
type AmountOrAmountInWei = {
    amount: string | number;
} | {
    amountInWei: bigint;
};
export declare function convertErc20Amount(options: Prettify<{
    client: ThirdwebClient;
    chain: Chain;
    erc20Address: string;
} & AmountOrAmountInWei>): Promise<bigint>;
export {};
//# sourceMappingURL=convert-erc20-amount.d.ts.map