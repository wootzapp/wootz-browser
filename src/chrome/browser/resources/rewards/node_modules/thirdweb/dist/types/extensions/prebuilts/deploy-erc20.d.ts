import type { FileOrBufferOrString } from "../../storage/upload/types.js";
import type { Prettify } from "../../utils/type-utils.js";
import type { ClientAndChainAndAccount } from "../../utils/types.js";
export type ERC20ContractType = "DropERC20" | "TokenERC20";
/**
 * @extension DEPLOY
 */
export type ERC20ContractParams = {
    name: string;
    description?: string;
    image?: FileOrBufferOrString;
    external_link?: string;
    social_urls?: Record<string, string>;
    symbol?: string;
    contractURI?: string;
    defaultAdmin?: string;
    saleRecipient?: string;
    platformFeeBps?: bigint;
    platformFeeRecipient?: string;
    trustedForwarders?: string[];
};
/**
 * @extension DEPLOY
 */
export type DeployERC20ContractOptions = Prettify<ClientAndChainAndAccount & {
    type: ERC20ContractType;
    params: ERC20ContractParams;
    publisher?: string;
}>;
/**
 * Deploys an thirdweb ERC20 contract of the given type.
 * On chains where the thirdweb infrastructure contracts are not deployed, this function will deploy them as well.
 * @param options - The deployment options.
 * @returns The deployed contract address.
 * @extension DEPLOY
 * @example
 * ```ts
 * import { deployERC20Contract } from "thirdweb/deploys";
 * const contractAddress = await deployERC20Contract({
 *  chain,
 *  client,
 *  account,
 *  type: "TokenERC20",
 *  params: {
 *    name: "MyToken",
 *    description: "My Token contract",
 *    symbol: "MT",
 * });
 * ```
 */
export declare function deployERC20Contract(options: DeployERC20ContractOptions): Promise<string>;
//# sourceMappingURL=deploy-erc20.d.ts.map