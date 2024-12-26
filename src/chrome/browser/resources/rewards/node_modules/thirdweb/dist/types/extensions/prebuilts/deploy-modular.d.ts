import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
import type { FileOrBufferOrString } from "../../storage/upload/types.js";
import { type Address } from "../../utils/address.js";
import type { Hex } from "../../utils/encoding/hex.js";
import type { Prettify } from "../../utils/type-utils.js";
import type { ClientAndChainAndAccount } from "../../utils/types.js";
import type { Account } from "../../wallets/interfaces/wallet.js";
export type CoreType = "ERC20" | "ERC721" | "ERC1155";
/**
 * @extension DEPLOY
 */
export type ModularContractParams = {
    name: string;
    description?: string;
    image?: FileOrBufferOrString;
    external_link?: string;
    social_urls?: Record<string, string>;
    symbol?: string;
    contractURI?: string;
    defaultAdmin?: string;
};
export type ModuleInstaller = (args: {
    client: ThirdwebClient;
    chain: Chain;
    account: Account;
}) => Promise<ModuleInstallData>;
export type ModuleInstallData = {
    module: Address;
    data: Hex;
};
/**
 * @extension DEPLOY
 */
export type DeployModularContractOptions = Prettify<ClientAndChainAndAccount & {
    core: CoreType;
    params: ModularContractParams;
    modules?: ModuleInstaller[];
    publisher?: string;
    salt?: string;
}>;
/**
 * Deploys an thirdweb ERC20 contract of the given type.
 * On chains where the thirdweb infrastructure contracts are not deployed, this function will deploy them as well.
 * @param options - The deployment options.
 * @returns The deployed contract address.
 * @modules
 * @example
 * ```ts
 * import { deployModularContract } from "thirdweb/modules";
 * const contractAddress = await deployModularContract({
 *  chain,
 *  client,
 *  account,
 *  core: "ERC20",
 *  params: {
 *    name: "MyToken",
 *    description: "My Token contract",
 *    symbol: "MT",
 * },
 * modules: [
 *   ClaimableERC721.module({
 *     primarySaleRecipient: "0x...",
 *   }),
 *   RoyaltyERC721.module({
 *     royaltyRecipient: "0x...",
 *     royaltyBps: 500n,
 *   }),
 * ],
 * });
 * ```
 */
export declare function deployModularContract(options: DeployModularContractOptions): Promise<string>;
//# sourceMappingURL=deploy-modular.d.ts.map