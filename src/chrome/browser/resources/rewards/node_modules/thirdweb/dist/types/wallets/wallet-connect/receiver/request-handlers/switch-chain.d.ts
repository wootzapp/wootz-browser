import { type Hex } from "../../../../utils/encoding/hex.js";
import type { Wallet } from "../../../interfaces/wallet.js";
import type { WalletConnectSwitchEthereumChainRequestParams } from "../types.js";
/**
 * @internal
 */
export declare function handleSwitchChain(options: {
    wallet: Wallet;
    params: WalletConnectSwitchEthereumChainRequestParams;
}): Promise<Hex>;
//# sourceMappingURL=switch-chain.d.ts.map