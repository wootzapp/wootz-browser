import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
import type { LensProfileSchema } from "./type.js";
/**
 * @extension LENS
 */
export type GetProfileMetadataParams = {
    profileId: bigint;
    client: ThirdwebClient;
    overrides?: {
        /**
         * Contract address for the LensHub contract
         */
        lensHubAddress?: string;
        chain?: Chain;
    };
};
/**
 * Download user lens profile from Arweave
 * This method does NOT give you the user handle & join-time - consider using `getFullProfileData` instead
 * It is useful & cost efficient if you only want to get user's name, bio, picture, coverPicture etc.
 *
 * @important The contract here is the LensHub contract
 * @param options
 * @returns LensProfileSchema | null
 * @extension LENS
 *
 * @example
 * ```ts
 * import { getProfileMetadata } from "thirdweb/extensions/lens";
 *
 * const profileData = await getProfileMetadata({ profileId, client });
 *
 * if (profileData) {
 *   console.log("Display name: ", profileData.lens.name);
 *   console.log("Bio: ", profileData.lens.bio);
 * }
 * ```
 */
export declare function getProfileMetadata(options: GetProfileMetadataParams): Promise<LensProfileSchema | null>;
//# sourceMappingURL=getProfileMetadata.d.ts.map