import type { FileOrBufferOrString } from "../../storage/upload/types.js";
import type { Prettify } from "../../utils/type-utils.js";
import type { ClientAndChainAndAccount } from "../../utils/types.js";
/**
 * @extension DEPLOY
 */
export type ERC1155ContractType = "DropERC1155" | "TokenERC1155";
/**
 * @extension DEPLOY
 */
export type ERC1155ContractParams = {
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
    royaltyRecipient?: string;
    royaltyBps?: bigint;
    trustedForwarders?: string[];
};
/**
 * @extension DEPLOY
 */
export type DeployERC1155ContractOptions = Prettify<ClientAndChainAndAccount & {
    type: ERC1155ContractType;
    params: ERC1155ContractParams;
}>;
/**
 * Deploys an thirdweb ERC1155 contract of the given type.
 * On chains where the thirdweb infrastructure contracts are not deployed, this function will deploy them as well.
 * @param options - The deployment options.
 * @returns The deployed contract address.
 * @extension DEPLOY
 * @example
 * ```ts
 * import { deployERC1155Contract } from "thirdweb/deploys";
 * const contractAddress = await deployERC1155Contract({
 *  chain,
 *  client,
 *  account,
 *  type: "DropERC1155",
 *  params: {
 *    name: "MyEdition",
 *    description: "My edition contract",
 *    symbol: "ME",
 * });
 * ```
 */
export declare function deployERC1155Contract(options: DeployERC1155ContractOptions): Promise<string>;
//# sourceMappingURL=deploy-erc1155.d.ts.map