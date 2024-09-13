import type { Chain } from "../../../chains/types.js";
import type { Wallet } from "../../../wallets/interfaces/wallet.js";
import type { AppMetadata } from "../../../wallets/types.js";
/**
 * @internal
 */
export declare function getDefaultWallets(options?: {
    appMetadata?: AppMetadata;
    chains?: Chain[];
}): Wallet[];
//# sourceMappingURL=defaultWallets.d.ts.map