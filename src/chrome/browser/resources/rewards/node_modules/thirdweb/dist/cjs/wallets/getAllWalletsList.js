"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getAllWalletsList = getAllWalletsList;
/**
 * Hide it for documentation - but expose it because we will use this to render the list of wallets in docs website
 * Using dynamic import just to be extra safe and avoid any tree shaking issues
 * @internal
 */
async function getAllWalletsList() {
    return (await Promise.resolve().then(() => require("./__generated__/wallet-infos.js"))).default;
}
//# sourceMappingURL=getAllWalletsList.js.map