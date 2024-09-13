/**
 * internal helper functions
 */
import type { ProviderInterface } from "@coinbase/wallet-sdk";
import { COINBASE } from "../constants.js";
import type { Wallet } from "../interfaces/wallet.js";
import type { CreateWalletArgs } from "../wallet-types.js";
/**
 * @internal
 */
export declare function coinbaseWalletSDK(args: {
    createOptions?: CreateWalletArgs<typeof COINBASE>[1];
    providerFactory: () => Promise<ProviderInterface>;
    onConnectRequested?: (provider: ProviderInterface) => Promise<void>;
}): Wallet<typeof COINBASE>;
//# sourceMappingURL=coinbase-wallet.d.ts.map