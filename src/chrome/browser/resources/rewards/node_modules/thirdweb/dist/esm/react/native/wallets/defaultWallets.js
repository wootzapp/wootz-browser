import { COINBASE, METAMASK, RAINBOW, ZERION, } from "../../../wallets/constants.js";
import { createWallet } from "../../../wallets/native/create-wallet.js";
/**
 * @internal
 */
export function getDefaultWallets(options) {
    return [
        createWallet("inApp"),
        createWallet(METAMASK),
        createWallet(COINBASE, {
            appMetadata: options?.appMetadata,
            chains: options?.chains,
        }),
        createWallet(RAINBOW),
        createWallet(ZERION),
        createWallet("com.trustwallet.app"),
    ];
}
//# sourceMappingURL=defaultWallets.js.map