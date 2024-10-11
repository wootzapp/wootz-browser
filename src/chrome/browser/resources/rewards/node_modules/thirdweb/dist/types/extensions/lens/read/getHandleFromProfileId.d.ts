import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * @extension LENS
 */
export type GetHandleFromProfileIdParams = {
    profileId: bigint;
    client: ThirdwebClient;
    /**
     * Override variables for LensHandle contract and TokenHandleRegistry contract
     * Make sure both of them have to be on the same network
     */
    overrides?: {
        /**
         * Contract address for the LensHandle contract
         */
        lensHandleAddress?: string;
        /**
         * Contract address for the TokenHandleRegistry contract
         */
        tokenHandleRegistryAddress?: string;
        chain?: Chain;
    };
};
/**
 * Return the Lens handle of a profile in the format: lens/@<name-of-user>
 * @param options
 * @returns
 * @extension LENS
 *
 * @example
 * ```ts
 * import { getHandleFromProfileId } from "thirdweb/extensions/lens";
 *
 * const profileId = 461662n;
 * const handle = await getHandleFromProfileId({ profileId, client }); // "lens/@captain_jack"
 * ```
 */
export declare function getHandleFromProfileId(options: GetHandleFromProfileIdParams): Promise<string | null>;
//# sourceMappingURL=getHandleFromProfileId.d.ts.map