import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * @extension FARCASTER
 */
export type GetUsdRegistrationPriceParams = {
    client: ThirdwebClient;
    chain?: Chain;
    extraStorage?: bigint | number | string;
    disableCache?: boolean;
};
/**
 * Retrieves the current cost to register a Farcaster fid in USD.
 * @param options - An object containing a client to use to fetch the price and the amount of extra storage to include in the returned price.
 * @returns A promise that resolves to the current cost of a Farcaster fid in USD.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getUsdRegistrationPrice } from "thirdweb/extensions/farcaster";
 *
 * const price = await getUsdRegistrationPrice({
 *  client,
 * });
 * ```
 */
export declare function getUsdRegistrationPrice(options: GetUsdRegistrationPriceParams): Promise<number>;
//# sourceMappingURL=getUsdRegistrationPrice.d.ts.map