"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
window.loadWallet = function loadWallet(token, encryptionKey) {
    return __awaiter(this, void 0, void 0, function* () {
        const wallet = window.thirdwebWallets.inAppWallet();
        return wallet.connect({
            client: window.client,
            chain: window.chain,
            strategy: 'jwt',
            jwt: token,
            encryptionKey: encryptionKey,
        });
    });
};
