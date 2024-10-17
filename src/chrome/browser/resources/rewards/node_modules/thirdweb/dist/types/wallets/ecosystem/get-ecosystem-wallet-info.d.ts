import type { Prettify } from "../../utils/type-utils.js";
import type { WalletInfo } from "../wallet-info.js";
import type { EcosystemWalletId } from "../wallet-types.js";
/**
 * Fetches metadata for a given ecosystem wallet.
 *
 * @param {EcosystemWalletId} walletId - The ecosystem wallet ID.
 * @returns {Promise<{ imageUrl: string | undefined, name: string | undefined }>} A promise that resolves to an object containing the wallet's image URL and name.
 * @throws {Error} Throws an error if no partner ID is provided in the wallet configuration.
 * @internal
 */
export declare function getEcosystemWalletInfo(walletId: EcosystemWalletId): Promise<Prettify<WalletInfo>>;
//# sourceMappingURL=get-ecosystem-wallet-info.d.ts.map