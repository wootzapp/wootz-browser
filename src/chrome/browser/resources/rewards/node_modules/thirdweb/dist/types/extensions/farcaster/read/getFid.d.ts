import type { Address } from "abitype";
import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * @extension FARCASTER
 */
export type GetFidParams = {
    client: ThirdwebClient;
    address: Address;
    chain?: Chain;
    disableCache?: boolean;
};
/**
 * Retrieves the current fid for an account.
 * @param options - Parameters to pass to the function.
 * @returns A promise that resolves to the account's fid, if one exists, otherwise 0.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getFid } from "thirdweb/extensions/farcaster";
 *
 * const price = await getFid({
 *  client,
 * 	address,
 * });
 * ```
 */
export declare function getFid(options: GetFidParams): Promise<bigint>;
//# sourceMappingURL=getFid.d.ts.map