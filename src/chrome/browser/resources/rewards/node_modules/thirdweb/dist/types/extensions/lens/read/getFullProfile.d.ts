import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
import type { LensProfileSchema } from "./type.js";
/**
 * @extension LENS
 */
export type GetFullProfileParams = {
    profileId: bigint;
    client: ThirdwebClient;
    includeJoinDate?: boolean;
    /**
     * Override variables for Lens smart contracts
     * Make sure all of them have to be on the same network
     */
    overrides?: {
        /**
         * Contract address for the LensHub contract
         */
        lensHubAddress?: string;
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
 * @extension LENS
 */
export type FullProfileResponse = {
    handle: string;
    joinDate: bigint | null;
    profileData: LensProfileSchema | null;
} | null;
/**
 * Return the profile data _with Lens handle_ and optional join date
 *
 * In Lens Protocol, each profile is associated with an ERC721 token,
 * thus, the tokenId represent profileId and the 2 terms can be used interchangeably
 * @extension LENS
 *
 * @example
 * ```ts
 * import { getFullProfile } from "thirdweb/extension/lens";
 *
 * const profileId = 10000n; // profileId is the tokenId of the NFT
 * const lensProfile = await getFullProfile({ profileId, client });
 * ```
 */
export declare function getFullProfile(options: GetFullProfileParams): Promise<FullProfileResponse>;
//# sourceMappingURL=getFullProfile.d.ts.map