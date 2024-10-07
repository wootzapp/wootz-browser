"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getDefaultWallets = getDefaultWallets;
const constants_js_1 = require("../../../wallets/constants.js");
const create_wallet_js_1 = require("../../../wallets/native/create-wallet.js");
/**
 * @internal
 */
function getDefaultWallets(options) {
    return [
        (0, create_wallet_js_1.createWallet)("inApp"),
        (0, create_wallet_js_1.createWallet)(constants_js_1.METAMASK),
        (0, create_wallet_js_1.createWallet)(constants_js_1.COINBASE, {
            appMetadata: options?.appMetadata,
            chains: options?.chains,
        }),
        (0, create_wallet_js_1.createWallet)(constants_js_1.RAINBOW),
        (0, create_wallet_js_1.createWallet)(constants_js_1.ZERION),
        (0, create_wallet_js_1.createWallet)("com.trustwallet.app"),
    ];
}
//# sourceMappingURL=defaultWallets.js.map