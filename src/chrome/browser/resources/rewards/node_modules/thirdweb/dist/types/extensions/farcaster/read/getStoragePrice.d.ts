import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * @extension FARCASTER
 */
export type GetStoragePriceParams = {
    client: ThirdwebClient;
    chain?: Chain;
    units?: bigint | number | string;
    disableCache?: boolean;
};
/**
 * Retrieves the current cost to register a Farcaster fid in wei.
 * @param options - An object containing a client to use to fetch the price and the amount of extra storage to include in the returned price.
 * @returns A promise that resolves to the current cost of a Farcaster fid in USD.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getStoragePrice } from "thirdweb/extensions/farcaster";
 *
 * const price = await getStoragePrice({
 *  client,
 * });
 * ```
 */
export declare function getStoragePrice(options: GetStoragePriceParams): Promise<bigint>;
//# sourceMappingURL=getStoragePrice.d.ts.map