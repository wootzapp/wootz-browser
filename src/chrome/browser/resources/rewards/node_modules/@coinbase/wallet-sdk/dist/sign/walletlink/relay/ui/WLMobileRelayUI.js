"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.WLMobileRelayUI = void 0;
const RedirectDialog_1 = require("./components/RedirectDialog/RedirectDialog");
const util_1 = require("./components/util");
const constants_1 = require("../../../../core/constants");
class WLMobileRelayUI {
    constructor() {
        this.attached = false;
        this.redirectDialog = new RedirectDialog_1.RedirectDialog();
    }
    attach() {
        if (this.attached) {
            throw new Error('Coinbase Wallet SDK UI is already attached');
        }
        this.redirectDialog.attach();
        this.attached = true;
    }
    redirectToCoinbaseWallet(walletLinkUrl) {
        const url = new URL(constants_1.CBW_MOBILE_DEEPLINK_URL);
        url.searchParams.append('redirect_url', (0, util_1.getLocation)().href);
        if (walletLinkUrl) {
            url.searchParams.append('wl_url', walletLinkUrl);
        }
        const anchorTag = document.createElement('a');
        anchorTag.target = 'cbw-opener';
        anchorTag.href = url.href;
        anchorTag.rel = 'noreferrer noopener';
        anchorTag.click();
    }
    openCoinbaseWalletDeeplink(walletLinkUrl) {
        this.redirectDialog.present({
            title: 'Redirecting to Coinbase Wallet...',
            buttonText: 'Open',
            onButtonClick: () => {
                this.redirectToCoinbaseWallet(walletLinkUrl);
            },
        });
        setTimeout(() => {
            this.redirectToCoinbaseWallet(walletLinkUrl);
        }, 99);
    }
    showConnecting(_options) {
        // it uses the return callback to clear the dialog
        return () => {
            this.redirectDialog.clear();
        };
    }
}
exports.WLMobileRelayUI = WLMobileRelayUI;
