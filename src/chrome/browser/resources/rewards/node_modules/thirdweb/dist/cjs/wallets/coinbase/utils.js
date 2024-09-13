"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.showCoinbasePopup = showCoinbasePopup;
async function showCoinbasePopup(provider) {
    // biome-ignore lint/suspicious/noExplicitAny: based on the latest CB SDK - scary but works
    await provider?.communicator?.waitForPopupLoaded?.();
}
//# sourceMappingURL=utils.js.map