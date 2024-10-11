import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * @extension LENS
 */
export type ResolveLensAddressParams = {
    /**
     * Either a full lens handle, or a lens localName
     * Examples:
     * 1. Full handle: "lens/vitalik"
     * 2. Just local name: "vitalik"
     */
    name: string;
    /**
     * a [`ThirdwebClient`](https://portal.thirdweb.com/references/typescript/v5/ThirdwebClient)
     */
    client: ThirdwebClient;
    /**
     * Override parameters for Lens contract, defaults to LensHandle contract on Polygon
     */
    overrides?: {
        /**
         * The contract address of the [`LensHandle contract`](https://www.lens.xyz/docs/resources/smart-contracts)
         */
        lensHandleContractAddress?: string;
        /**
         * The chain which `lensHandleContractAddress` is deployed on
         */
        chain?: Chain;
    };
};
/**
 * Take in a Lens handle or local-name and return the wallet address behind that handle/local-name.
 * For example, "lens/vitalik" is a handle, with "lens" being the namespace and "vitalik" being the local name
 * @extension LENS
 * @example
 * ```ts
 * import { resolveAddress } from "thirdweb/extensions/lens";
 *
 * const walletAddress = await resolveAddress({
 *   name: "vitalik",
 *   client,
 * });
 * ```
 */
export declare function resolveAddress(options: ResolveLensAddressParams): Promise<string>;
//# sourceMappingURL=resolveAddress.d.ts.map