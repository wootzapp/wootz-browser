import type { WalletData } from '@walletconnect/modal-core';
export declare const TemplateUtil: {
    onConnecting(data: WalletData): void;
    manualWalletsTemplate(): import("lit-html").TemplateResult<1>[];
    recomendedWalletsTemplate(skipRecent?: boolean): import("lit-html").TemplateResult<1>[];
    recentWalletTemplate(): import("lit-html").TemplateResult<1> | undefined;
};
