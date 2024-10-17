import type { Chain } from "../chains/types.js";
import type { ThirdwebClient } from "../client/client.js";
export type GetGasPriceOptions = {
    client: ThirdwebClient;
    chain: Chain;
    percentMultiplier?: number;
};
/**
 * Retrieves the gas price for a transaction on a specific chain.
 * @param client - The Thirdweb client.
 * @param chain - The ID of the chain.
 * @returns A promise that resolves to the gas price as a bigint.
 * @example
 * ```ts
 * import { getGasPrice } from "thirdweb";
 *
 * const gasPrice = await getGasPrice({ client, chain });
 * ```
 * @utils
 */
export declare function getGasPrice(options: GetGasPriceOptions): Promise<bigint>;
//# sourceMappingURL=get-gas-price.d.ts.map