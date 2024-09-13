import { RelayUI } from './RelayUI';
export declare class WLMobileRelayUI implements RelayUI {
    private readonly redirectDialog;
    private attached;
    constructor();
    attach(): void;
    private redirectToCoinbaseWallet;
    openCoinbaseWalletDeeplink(walletLinkUrl?: string): void;
    showConnecting(_options: {
        isUnlinkedErrorState?: boolean | undefined;
        onCancel: (error?: Error) => void;
        onResetConnection: () => void;
    }): () => void;
}
