import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * @extension FARCASTER
 */
export type GetRegistrationPriceParams = {
    client: ThirdwebClient;
    chain?: Chain;
    extraStorage?: bigint | number | string;
    disableCache?: boolean;
};
/**
 * Retrieves the current cost to register a Farcaster fid in wei.
 * @param options - An object containing a client to use to fetch the price and the amount of extra storage to include in the returned price.
 * @returns A promise that resolves to the current cost of a Farcaster fid in wei.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getRegistrationPrice } from "thirdweb/extensions/farcaster";
 *
 * const price = await getRegistrationPrice({
 *  client,
 * });
 * ```
 */
export declare function getRegistrationPrice(options: GetRegistrationPriceParams): Promise<bigint>;
//# sourceMappingURL=getRegistrationPrice.d.ts.map