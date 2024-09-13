import type { FileOrBufferOrString } from "../../storage/upload/types.js";
import type { Prettify } from "../../utils/type-utils.js";
import type { ClientAndChainAndAccount } from "../../utils/types.js";
/**
 * @extension DEPLOY
 */
export type SplitContractParams = {
    name: string;
    /**
     * An array of strings containing wallet addresses of the recipients
     * For example:
     * ```ts
     * ["0x...123", "0x...456"]
     * ```
     */
    payees: string[];
    /**
     * An array of bigints containing the shared percentages of each respective payees.
     * Must have the same length as `payees`
     * @example
     * ```ts
     * [
     *   5100n, // 51%
     *   4900n, // 49%
     * ]
     * ```
     */
    shares: bigint[];
    description?: string;
    image?: FileOrBufferOrString;
    external_link?: string;
    social_urls?: Record<string, string>;
    symbol?: string;
    contractURI?: string;
    defaultAdmin?: string;
    trustedForwarders?: string[];
};
/**
 * @extension DEPLOY
 */
export type DeploySplitContractOptions = Prettify<ClientAndChainAndAccount & {
    params: SplitContractParams;
}>;
/**
 * Deploys a thirdweb [`Split contract`](https://thirdweb.com/thirdweb.eth/Split)
 * On chains where the thirdweb infrastructure contracts are not deployed, this function will deploy them as well.
 * @param options - The deployment options.
 * @returns The deployed contract address.
 * @extension DEPLOY
 *
 * @example
 * ```ts
 * import { deploySplitContract } from "thirdweb/deploys";
 * const contractAddress = await deploySplitContract({
 *  chain,
 *  client,
 *  account,
 *  params: {
 *    name: "Split contract",
 *    payees: ["0x...123", "0x...456"],
 *    shares: [5100, 4900], // See type `SplitContractParams` for more context
 *  },
 * });
 * ```
 */
export declare function deploySplitContract(options: DeploySplitContractOptions): Promise<string>;
//# sourceMappingURL=deploy-split.d.ts.map